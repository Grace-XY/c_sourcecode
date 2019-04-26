/**
* date : 2018/9/28 18:09
*/
#ifndef __LOCALCONTROL_H__
#define __LOCALCONTROL_H__


typedef enum
{
	SEND_IRCODE_CMD,
	MATCH_SWITCH_CMD,
	CHECK_UPGRADE_CMD,
	HEART_BEAT_CMD,
	SET_PROPERTY_CMD,//��Դ�������� by peter
	UNKOWN_CMD,
}lc_controy_e;

/**
* @����ص�����
* @cmd_type [��������]
* @cmd_id   [�������]
* @cmd      [��������]
* @param    [�ص�����]
*/
typedef int (*swiot_lccmd_callback)(lc_controy_e cmd_type,void* cmd_handle,int cmd_id,char* cmd,void* param);

/**
* @���ؿ��Ƴ�ʼ��
*/
int SWIOT_LC_Construct(char* sn);

/**
* @���ٱ��ؿ���ģ��
*/
int SWIOT_LC_Destroy();


/**
* @ע������ص�����
* @param cmd_handle [����ص�����]
* @param param      [�ص�����]
*/
int SWIOT_LC_Register(swiot_lccmd_callback cmd_handle,void* param);

/**
* @����ظ�
* @param cmd_id [����ID]
* @param code   [�ظ���]
*/
int SWIOT_LC_Response(void* cmd_handle,int cmd_id,int code,char* data,int data_size);


#endif //__LOCALCONTROL_H__