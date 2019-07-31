/**
 * @file   swhttpserver.h
 * @version %I% %G%
 * @author Hongchen Dou
 * @brief  http server
 * @history
 * 				2007-07-15  Houchen Dou created
 * 				2010-07-29 	Shihui Qu modified
 */

#ifndef __SWHTTPSERVER_H__
#define __SWHTTPSERVER_H__

typedef enum
{
  HTTP_OK = 200,
  HTTP_MOVED_TEMPORARILY = 302,
  HTTP_BAD_REQUEST = 400,       /* malformed syntax */
  HTTP_UNAUTHORIZED = 401, /* authentication needed, respond with auth hdr */
  HTTP_NOT_FOUND = 404,
  HTTP_FORBIDDEN = 403,
  HTTP_REQUEST_TIMEOUT = 408,
  HTTP_NOT_IMPLEMENTED = 501,   /* used for unrecognized requests */
  HTTP_INTERNAL_SERVER_ERROR = 500,
  HTTP_CONTINUE = 100,
  HTTP_SWITCHING_PROTOCOLS = 101,
  HTTP_CREATED = 201,
  HTTP_ACCEPTED = 202,
  HTTP_NON_AUTHORITATIVE_INFO = 203,
  HTTP_NO_CONTENT = 204,
  HTTP_MULTIPLE_CHOICES = 300,
  HTTP_MOVED_PERMANENTLY = 301,
  HTTP_NOT_MODIFIED = 304,
  HTTP_PAYMENT_REQUIRED = 402,
  HTTP_BAD_GATEWAY = 502,
  HTTP_SERVICE_UNAVAILABLE = 503, /* overload, maintenance */
  HTTP_RESPONSE_SETSIZE=0x7fffffff
} http_response_num_t;

#define HTTPSERVER_MAX_LINE 1024

#ifndef HTTP_AUTHENTICATION_REQUIRED
#define HTTP_AUTHENTICATION_REQUIRED
#endif

typedef struct
{
	char method[8];
	char request_url[1024];
	char host[128];
	char accept_type[16];
	char accept_encoding[16];
	char connection[16];
#ifdef HTTP_AUTHENTICATION_REQUIRED
	char authorization[256];
#endif	
	char content_type[16];
	int content_length;
	int header_length;
}http_request_header_t;

	
typedef struct
{
	/* �����׽��� */
	int skt;
	/* �ͻ���ip�������ֽ��� */
	unsigned long from_ip;
	/* �ͻ���ipv6�������ֽ��� */
	//struct in6_addr from_ipv6;
	/* �ͻ��˶˿� */
	unsigned short from_port;
	/* ��������� */
	HANDLE h_http_server;
	/* ���������� */
	http_request_header_t request_header;

	char buf[HTTPSERVER_MAX_LINE];
	/*SSL ���*/
	HANDLE ssl;
}http_connect_obj_t;


#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief ����HttpServer�ص�����,����ʱ��:����һ���µ�����ʱ, ���������ӣ�
 *        ͬʱ����һ������ʵ��(�ڲ������ڴ棬��Ҫ�ֹ�����sw_httpserver_close_connectobj�ͷţ�
 *
 * @param obj ������ 
 * @param wparam  ���Ӳ���
 *
 * @return 
 */
typedef int (*http_server_callback)( http_connect_obj_t* obj, uint32_t wparam ); 

/** 
 * @brief ����һ��httpserver 
 * 
 * @param port �������˿ںţ� ������
 * @param callback �ص�����
 * @param wparam  �ص������Ĳ���
 * 
 * @return �ɹ�������Httpserver�����ʧ�ܣ�����NULL
 */
HANDLE sw_httpserver_open(unsigned short port, http_server_callback callback, uint32_t wparam );
HANDLE sw_httpserver_open_v6(unsigned short port, http_server_callback callback, uint32_t wparam );


/** 
 * @brief �ر�httpserver
 * 
 * @param server httpserver���
 */
void sw_httpserver_close( HANDLE server );

/** 
 * @brief ����Request��ͷ���ɹ������request_header�ṹ
 * 
 * @param obj ������
 * @param timeout ��ʱ
 * 
 * @return �ɹ����ر���ͷ�Ĵ�С�� ʧ�ܷ��ظ���
 */
int sw_httpserver_recv_request_header(http_connect_obj_t* obj, int timeout );
	
/** 
 * @brief ��������ʵ��
 * 
 * @param obj ������
 * @param buf ���ջ�����
 * @param buf_size ��������С
 * @param timeout ��ʱ
 * 
 * @return ���ؽ������ݵ��ֽ�����0: ������ɣ�-1: ���ճ���
 */
int sw_httpserver_recv_request_content(http_connect_obj_t* obj, char*buf, int buf_size, int timeout );

/** 
 * @brief ������Ӧ����ͷ
 *
 * @param obj ������
 * @param response_num ��Ӧֵ  
 * @param content_type ��������
 * @param content_encoding 
 * @param connection 
 * @param content_lenth ���ݳ���
 * @param timeout ��ʱ
 * 
 * @return �ɹ������ط��͵������ֽ�����ʧ�ܣ�����-1
 */
int sw_httpserver_send_response_header(http_connect_obj_t* obj, http_response_num_t response_num, 
									   char* content_type, char* content_encoding, 
									   char* connection,
									   int content_length, 
									   int timeout);
	
/** 
 * @brief ������Ӧʵ��
 * 
 * @param obj ������
 * @param buf ���ͻ�����
 * @param buf_size ��������С
 * @param timeout ��ʱ
 * 
 * @return �ɹ�,���ط��������ֽ���;ʧ��,����-1
 */
int sw_httpserver_send_response_content(http_connect_obj_t* obj,
											char*buf, int buf_size,
											int timeout);

/** 
 * @brief �ر�����,ͬʱ�ͷ���Դ
 * 
 * @param obj ������ 
 */
void sw_httpserver_close_connectobj(http_connect_obj_t* obj);
	
#ifdef __cplusplus
}
#endif

#endif
