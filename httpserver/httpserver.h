/**
* @date   : 2018/9/27 14:21
* @author : zhoutengbo
*/
#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

/**
 * [swiot_packet_deal_func  http���ص�����]
 * @param  code 	[������]
 * @param  uri  	[·��]
 * @param  palyload [����]
 * @param  len      [���س���]
 */
typedef void (*swiot_packet_deal_func)( void* client,char* method,char* uri,char* palyload,int head_len,int size,int session_id,void* param );

/**
 * [sw_http_server_init http�����ʼ��]
 * @param  ip   [ip��ַ]
 * @param  port [�˿�]
 * @return      [�ɹ����ؾ��]
 */
int swiot_http_server_init( char* ip,short port,swiot_packet_deal_func func,void* param );


/**
 * [swiot_http_server_respond ���ͻظ�]
 * @param  handle    [���]
 * @param  code      [�ظ���]
 * @param  data      [����]
 * @param  data_size [���ݴ�С]
 * @return           [�ɹ�����0]
 */
int swiot_http_server_response( void* client,int session_id,int code,char* data,int data_size );


/**
* [swiot_http_server_ontimer ��ʱ��]
* @param now [��ǰʱ��]
*/
int swiot_http_server_ontimer(int now);

/**
 * [swiot_http_server_destroy ����http����]
 * @param  handle [���]
 * @return        [�ɹ�����0]
 */
int swiot_http_server_destroy();


#endif  //__HTTPSERVER_H__