#include "swiot_common.h"
#include "swiot_platform.h"
#include "localcontrol.h"
#include "udpcast.h"
#include "cJSON.h"

#include "sw_common.h"


#define TCP_LISTEN_IP   "0.0.0.0"
#define TCP_LISTEN_PORT 4004

#define UDP_BROADCAST_IP   "255.255.255.255"
#define UDP_BROADCAST_PORT 4005

#define UDP_BROADCAST_DATA "{\"port\":%d,\"devicename\":\"%s\"}"

#define WORK_THREAD_STACK_SIZE 1024

#define SEND_IRCODE_URI   "/send/ircode"
#define MATCH_SWITCH_URI  "/math/switch"
#define CHECK_UPGRADE_URI "/check/upgrade"
#define HEART_BEAT        "/heartBeat"
#define SET_PROPERTY      "/setProperty"

#define LCWORKING 0
#define LCEXITED 1

typedef struct
{
    swiot_lccmd_callback cmd_callback;
    void* callback_param;
    void*                  thread_handle;
    int status;
} swiot_lchandle_t;

swiot_lchandle_t g_lc_handle;

void http_packet_handle( void* client,char* method,char* uri,char* palyload,int head_len,int size,int session_id,void* param );
lc_controy_e get_cmd_type(char*method,char*uri);
char* get_cmd_msg(char* data,int data_size);

void work_threadlc(void *params);

xTaskHandle lc_xHandle;

/**
* @���ؿ��Ƴ�ʼ��
*/
int SWIOT_LC_Construct(char* sn)
{
    int rc = -1;
    char tmp[128] = {0};

    memset(&g_lc_handle,0,sizeof(swiot_lchandle_t));

    if (0 != swiot_http_server_init(TCP_LISTEN_IP,TCP_LISTEN_PORT,http_packet_handle,NULL))
    {
        log_error("Init http server error!\n");
        goto ERROR;
    }

    rc = snprintf(tmp,sizeof(tmp),UDP_BROADCAST_DATA,TCP_LISTEN_PORT,sn);
    if (0 != swiot_udpcast_start(UDP_BROADCAST_IP,UDP_BROADCAST_PORT,tmp,rc))
    {
        log_error("Init udpcast error!\n");
        goto ERROR;
    }

    g_lc_handle.status = LCWORKING;

    #if 0//by peter
    rc = xTaskCreate(work_threadlc, "work_threadlc", 1024, NULL, tskIDLE_PRIORITY + 2, &lc_xHandle);
    if (rc != pdPASS)
    {
        log_error("Create thread error\n");
        goto ERROR;
    }
    #else //by swiot
    rc = SWIOT_Thread_Create(g_lc_handle.thread_handle,work_threadlc,"lc_thread",WORK_THREAD_STACK_SIZE, NULL,125);
    if (0 == rc)
    {
        log_error("Create thread error\n");
        goto ERROR;
    }
    #endif

    return 0;
ERROR:
    swiot_udpcast_stop();
    swiot_http_server_destroy();
    return -1;
}

/**
* @���ٱ��ؿ���ģ��
*/
int SWIOT_LC_Destroy()
{
    swiot_udpcast_stop();
    swiot_http_server_destroy();

    g_lc_handle.status = LCEXITED;
    return 0;
}

#if 1

int my_swiot_lccmd_callback(lc_controy_e cmd_type,void* cmd_handle,int cmd_id,char* cmd,void* param)
{
    printf("\n++++++++++++++++++++++++++++++++\n");
    printf(" cmd_type = %d\n",cmd_type);
    printf(" cmd_id = %d\n",cmd_id);
    printf(" cmd = %s\n",cmd);
    printf(" param = %s\n",param);
    printf("\n\n");
}

#endif


/**
* @ע������ص�����
* @param cmd_handle [����ص�����]
* @param param      [�ص�����]
*/
int SWIOT_LC_Register(swiot_lccmd_callback cmd_handle,void* param)
{
    g_lc_handle.cmd_callback = cmd_handle;
    g_lc_handle.callback_param = param;
    return 0;
}

/**
* @����ظ�
* @param cmd_id [����ID]
* @param code   [�ظ���]
*/
int SWIOT_LC_Response(void* cmd_handle,int cmd_id,int code,char* data,int data_size)
{
    return swiot_http_server_response(cmd_handle,cmd_id,code,data,data_size);
}

extern xSemaphoreHandle key_press_sem;
extern dev_st G_dev_info;

void http_packet_handle( void* client,char* method,char* uri,char* palyload,int head_len,int size,int session_id,void* param )
{
    lc_controy_e cmd_type;
    char* cmd =NULL ;

    log_info("method:%s uri:%s palyload:%s head_len:%d size:%d session_id:%d\n",method,uri,palyload,head_len,size,session_id);

    cmd_type = get_cmd_type(method,uri);

    if (UNKOWN_CMD == cmd_type)
    {
        log_error("unkown cmd\n");
        SWIOT_LC_Response(client,session_id,404,NULL,0);
        return;
    }
    else if(HEART_BEAT_CMD == cmd_type)
    {
        SWIOT_LC_Response(client,session_id,200,NULL,0);
        return;
    }

    if (NULL == (cmd = get_cmd_msg(palyload+head_len,size-head_len)))
    {
        log_error("get cmd error,palyload=%s\n",palyload+head_len);
        SWIOT_LC_Response(client,session_id,404,NULL,0);
        return;
    }
    #if 1//by peter
    cJSON* json = NULL;

    json = cJSON_Parse(cmd);
    if(json)
    {
        cJSON* json_switch = NULL;

        json_switch = cJSON_GetObjectItem(json,"switch");
        if(json_switch && json_switch->type == cJSON_Number)
        {
            printf("switch = %d\n",json_switch->valueint);

            if(json_switch->valueint != G_dev_info.dev_state.powerSwitch)
            {
                xSemaphoreGive(key_press_sem);//
            }

            SWIOT_LC_Response(client,session_id,200,NULL,0);//�ظ�ok
        }
    }

    if(json)
        cJSON_Delete(json);

    #endif

    if(g_lc_handle.cmd_callback)
    {
        g_lc_handle.cmd_callback(cmd_type,client,session_id,cmd,g_lc_handle.callback_param);
    }

    if (cmd)
    {
        free(cmd);
    }
}

lc_controy_e get_cmd_type(char*method,char*uri)
{
    lc_controy_e cmd_type=UNKOWN_CMD;

    if(memcmp(method,"POST",4) != 0)
    {
        log_error("the method is not right,method=%s\n",method);
        return UNKOWN_CMD;
    }

    if (strlen(uri) == strlen(SEND_IRCODE_URI)
            &&memcmp(uri,SEND_IRCODE_URI,strlen(SEND_IRCODE_URI)) == 0)
    {
        cmd_type = SEND_IRCODE_CMD;
    }
    else if (strlen(uri) == strlen(MATCH_SWITCH_URI)
             &&memcmp(uri,MATCH_SWITCH_URI,strlen(MATCH_SWITCH_URI)) == 0)
    {
        cmd_type = MATCH_SWITCH_CMD;
    }
    else if (strlen(uri) == strlen(CHECK_UPGRADE_URI)
             &&memcmp(uri,CHECK_UPGRADE_URI,strlen(CHECK_UPGRADE_URI)) == 0)
    {
        cmd_type = CHECK_UPGRADE_CMD;
    }
    else if (strlen(uri) == strlen(HEART_BEAT)
             &&memcmp(uri,HEART_BEAT,strlen(HEART_BEAT)) == 0)
    {
        cmd_type = HEART_BEAT_CMD;
    }
    else if (strlen(uri) == strlen(SET_PROPERTY)&& memcmp(uri,SET_PROPERTY,strlen(SET_PROPERTY)) == 0)
    {
        cmd_type = SET_PROPERTY_CMD;
    }
    return cmd_type;
}

char* get_cmd_msg(char* data,int data_size)
{
    cJSON* json = NULL;
    cJSON* sub_json= NULL;
    char* str = NULL;

    json = cJSON_Parse(data);
    if (NULL == json)
    {
        log_error("Parser json error,json=%s\n",data);
        goto END;
    }

    sub_json = cJSON_GetObjectItem(json,"params");
    if (NULL == json)
    {
        #if 1//by peter

        sub_json = cJSON_GetObjectItem(json,"switch");//{"switch":1}
        if (NULL == json)
        {
            log_error("Can't get \"switch\",json=%s\n",data);
            goto END;
        }

        #endif
    }

    str = cJSON_Print(sub_json);
    if (NULL == str)
    {
        log_error("Can't conver to string,json=%s\n",data);

        #if 1//by peter

        str = cJSON_Print(json);
        if (NULL == str)
        {
            log_error("Can't conver to string,json=%s\n",data);
            goto END;
        }
        #endif
        //goto END;
    }
    else
    {

    }

END:
    if (json)
        cJSON_Delete(json);

    return str;
}

void  work_threadlc(void *params)
{
    int now = 0;
    static char buf[128] = {0};

    SWIOT_LC_Register(my_swiot_lccmd_callback,buf);

    while(LCWORKING == g_lc_handle.status)
    {
        now = SWIOT_Common_Get_Ms();

        swiot_http_server_ontimer(now);

        swiot_udpcast_ontimer(now);

        //SWIOT_msleep(1);
        vTaskDelay(100 / portTICK_RATE_MS);
    }

    //vTaskDelete(NULL);
}

