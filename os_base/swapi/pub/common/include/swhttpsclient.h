/** 
 * @file swhttpsclient.h
 * @brief HTTP�����ӿ�
 * @author NiuJiuRu
 * @date 2007-10-29
 * @history
 * 				2007-10-29 NiuJiuRu created
 * 				2010-07-29 qushihui modified
 */

#ifndef __SWHTTPSCLIENT_H__
#define __SWHTTPSCLIENT_H__

#include "swhttpclient.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief ��Http��������������,���֤��·�����ڻ򻺴�ǿ���ȷ��֤����ȷ
 * 
 * @param ip http��������ַ
 * @param port http�������˿�
 * @param timeout 
 * @param certpath֤��·��,sw_httpsclient_connect���ʹ��Ĭ�ϵ�֤��·��
 * @param certbuf֤�黺��----�����Ϊ������ʹ�����������certpath
 * @param certlen֤�黺�����ݳ���,���certbuf��Ϊ��len<=0ʱʹ��strlen�ж�
 * 
 * @return �������ӳɹ���ľ��
 */
HANDLE sw_httpsclient_connect_verify( unsigned long  ip, unsigned short port, int timeout, void *key );

HANDLE sw_httpsclient_connect( unsigned long ip, unsigned short port, int timeout);
//��secure��֧��ʵ��
HANDLE sw_httpsclient_connect_ex( unsigned long ip, unsigned short port, int timeout, const char *certpath, const char *certbuf, int certlen);
HANDLE sw_httpsclient_connect_v6( struct in6_addr ipv6, unsigned short port, int timeout, const char *certpath, const char *certbuf, int certlen);

/************************secure��֧����ĺ���������ʹ��httpclient����Ķ�Ӧ�ӿ�,ֻ���ڽ�������ʱ��Ҫָ��**********************************************/
/** 
 * @brief �Ͽ���Http����������������
 * 
 * @param h_http_client ���ӳɹ���ľ��
 */
void sw_httpsclient_disconnect( HANDLE h_http_client );

/** 
 * @brief ����HTTP Request
 * 
 * @param h_http_client �������Ӻ�ľ��
 * @param method ����ʽ
 * @param url �����URL
 * @param host ��������
 * @param accept_type ���յ��ļ�����
 * @param content_type ��������
 * @param content ����
 * @param length ���ݳ���
 * @param timeout ��ʱ
 * @param soap_action: soap action URI
 * @param http_auth ժҪ��֤(NULL, ��ʹ��ժҪ��֤; NOT NULL, Ҫʹ�õ�ժҪ��֤��Ϣ)
 * 
 * @return �����Ƿ��ͳɹ�
 */
bool sw_httpsclient_request( HANDLE h_http_client, char* method, char* url, char *host, char *accept_type, char* content_type,
	   							char* content, int length, int timeout, char* soap_action, http_authentcation_t* http_auth );
/*
[in] pFileName:Ҫ�ϴ����ļ���
*/
bool sw_httpsclient_request_ex( HANDLE hHttpclient, char* pMethod,char* pURL,char *pHost, char *pAcceptType,char *pFileName,
		                        char* pContentType,char* pContent,int nLength, int timeout,char* pSOAPAction,http_authentcation_t* pHttpAuth);
/** 
 * @brief ����HTTP����
 * 
 * @param h_http_client �������Ӻ�ľ��
 * @param buf ���ջ�����
 * @param size ��������С
 * @param timeout ���ճ�ʱ
 * 
 * @return ���յ������ݳ���
 */
int sw_httpsclient_recv_data( HANDLE h_http_client, char* buf, int size, int timeout );

/** 
 * @brief ȡ�ø��س���
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return 
 */
int64_t sw_httpsclient_get_content_size( char* head_buf, int size );

/** 
 * @brief ȡ�÷���ֵ
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return 
 */
int sw_httpsclient_get_ret_code( char* head_buf, int size );

/** 
 * @brief �õ�http header�ĳ���
 * 
 * @param head_buf HTTP����ͷ����
 * @param size ��������С
 * 
 * @return 
 */
int sw_httpsclient_get_header_size( char* head_buf, int size );

/** 
 * @brief ע��cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param cookies Ԥ���õ�cookies��Ż�����
 * 
 * @return 
 */
int sw_httpsclient_register_cookies( HANDLE h_http_client, char* cookies );

/** 
 * @brief ���cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return
 */
void sw_httpsclient_clear_cookies( HANDLE h_http_client );

/** 
 * @brief ȡ��cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return h_http_client cookie �洢��
 */
char* sw_httpsclient_get_cookies( HANDLE h_http_client );

/** 
 * @brief ����HTTP�汾��
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param version Ԥ���õ�HTTP�汾��
 * 
 * @return 
 */
void sw_httpsclient_set_version( HANDLE h_http_client, char* version );


#ifdef __cplusplus
}
#endif

#endif /* __SWHTTPCLIENT_H__ */

