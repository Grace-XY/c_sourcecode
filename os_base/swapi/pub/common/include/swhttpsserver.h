/**
 * @file   swhttpsserver.h
 * @brief  http server
 * @version %I% %G%
 * @author Hongchen Dou
 * @history 
 * 				2007-07-15 Hongchen Dou created
 * 				2010-07-29 qushihui modified 
 */

#ifndef __SWHTTPSSERVER_H__
#define __SWHTTPSSERVER_H__


#include "swhttpserver.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * ����һ��httpserver 
 * 
 * @param port �������˿ںţ� ������
 * @param callback �ص�����
 * @param wparam  �ص������Ĳ���
 * 
 * @return ����Httpserver���
 */
HANDLE sw_httpsserver_open(unsigned short port, http_server_callback callback, uint32_t wparam, const char *certpath, const char *keypath);
HANDLE sw_httpsserver_open_v6(unsigned short port, http_server_callback callback, uint32_t wparam );


/** 
 * �ر�httpserver
 * 
 * @param server 
 */
void sw_httpsserver_close( HANDLE server );

/** 
 * ����Request��ͷ�� �ɹ������request_header�ṹ
 * 
 * @param obj 
 * @param timeout 
 * 
 * @return int: �ɹ����ر���ͷ�Ĵ�С�� ʧ�ܷ��ظ���
 */
int sw_httpsserver_recv_request_header(HANDLE sobj, int timeout );

/** 
 * ��������ʵ��
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return ���ؽ������ݵ��ֽ����� 0: ������ɣ� -1: ���ճ���
 */
int sw_httpsserver_recv_request_content(HANDLE sobj, char*buf, int buf_size, int timeout );


/** 
 * 
 * ������Ӧ����ͷ
 * @param obj 
 * @param responseNum 
 * @param pContentType 
 * @param pContentEncoding 
 * @param pConnection 
 * @param nContentLength 
 * @param timeout 
 * 
 * @return ���͵������ֽ���
 */
int sw_httpsserver_send_response_header(HANDLE sobj, http_response_num_t responseNum, 
									   char* pContentType, char* pContentEncoding, 
									   char* pConnection,
									   int nContentLength, 
									   int timeout);
	
/** 
 * ������Ӧʵ��
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return ���ط��������ֽ��� -1������ʧ��
 */
int sw_httpsserver_send_response_content(HANDLE sobj,
											char*buf, int buf_size,
											int timeout);

/** 
 * �ر�����,ͬʱ�ͷ���Դ
 * 
 * @param obj 
 */
void sw_httpsserver_close_connectobj(HANDLE sobj);
	
#ifdef __cplusplus
}
#endif

#endif
