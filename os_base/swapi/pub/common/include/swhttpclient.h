/** 
 * @file swhttpclient.h
 * @brief HTTP�����ӿ�
 * @version %I% %G%
 * @author NiuJiuRu
 * @date 2007-10-29
 * @history
 * 			2010-07-29	qushihui modified
 */

#ifndef __SWHTTPCLIENT_H__
#define __SWHTTPCLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif



#define HTTP_OK 0
#define HTTP_ERROR_PORT_NORESPONSE 1
#define HTTP_ERROR_PORT_DENY  2 
#define HTTP_ERROR_HTTP_NORESPONSE 3
#define HTTP_ERROR_TIMEOUT 4
#define HTTP_ERROR_EXCEPTION 5
#define HTTP_ERROR_HTTP_NORESPONSE_404 404
#define HTTP_ERROR_HTTP_NORESPONSE_302 302
#define HTTP_ERROR_HTTP_NORESPONSE_403 403
#define HTTP_ERROR_HTTP_NORESPONSE_500 500
#define HTTP_ERROR_HTTP_NORESPONSE_206 206
#define HTTP_ERROR_HTTP_NORESPONSE_301 301
#define HTTP_ERROR_HTTP_NORESPONSE_400 400
#define HTTP_ERROR_HTTP_NORESPONSE_406 406
#define HTTP_ERROR_HTTP_NORESPONSE_503 503


//HTTPժҪ��֤
typedef struct http_authentication
{
	char arithmetic[10];		// �㷨����: md5, md5-sess
	char user_name[64];		// �û��� 
	char user_pwd[64];		// ����
	char realm[128];			// realm name
	char nonce[48];			// ���������������nonce���ش�
	char uri[256];			// ����URL
	char qop[10];			// qop-value: "", "auth", "auth-int"
	char opaque[48];		// opaque value
}http_authentcation_t;


HANDLE sw_httpclient_init(unsigned long  ip, unsigned short port);
bool sw_httpclient_connect2( HANDLE h_http_client, int timeout);

/** 
 * @brief ��Http��������������
 * 
 * @param ip http��������ַ
 * @param port http�������˿�
 * @param timeout 
 * 
 * @return �ɹ������ؾ����ʧ�ܣ�����NULL
 */
HANDLE sw_httpclient_connect( unsigned long ip, unsigned short port, int timeout );
HANDLE sw_httpclient_connect_v6( struct in6_addr ip, unsigned short port, int timeout );
HANDLE sw_httpclient_connect_ext( unsigned long  ip, unsigned short port, int timeout, bool is_https);

/** 
 * @brief �Ͽ���Http����������������
 * 
 * @param h_http_client ���ӳɹ���ľ��
 */
void sw_httpclient_disconnect( HANDLE h_http_client );

void sw_httpclient_shutdown ( HANDLE h_http_client);
void sw_httpclient_close( HANDLE h_http_client);

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
 * @return �����ͳɹ�,true;ʧ��,false
 */
/*
bool sw_httpclient_request( HANDLE h_http_client, char* method, char* url, char *host,
							char *accept_type, char* content_type, char* content, int length, 
							int timeout, char* soap_action, SHttpAuthentcation* http_auth );  */

bool sw_httpclient_request( HANDLE h_http_client, char* method, char* url, char* host,
							char *accept_type, char* content_type, char* content, int length,
							int timeout, char* soap_action, http_authentcation_t* http_auth );

/** 
 * @brief ����HTTP Request TTNET request
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
 * @return �����ͳɹ�,true;ʧ��,false
 */
bool sw_httpclient_request_ex( HANDLE h_http_client, char* method, char* url, char* host,
							char *accept_type, char* content_type, char* content, int length,
							int timeout, char* soap_action, http_authentcation_t* http_auth );	

//[in]filename:Ҫ�ϴ��ļ����ļ���
bool sw_httpclient_request_ex2( HANDLE h_http_client, char* method, char* url, char* host,
		                    char *accept_type,char *filename, char* content_type, char* content, int length,
		                    int timeout, char* soap_action, http_authentcation_t* http_auth );


/** 
 * @brief ����HTTP����
 * 
 * @param h_http_client �������Ӻ�ľ��
 * @param buf ���ջ�����
 * @param size ��������С
 * @param timeout ���ճ�ʱ
 * 
 * @return �ɹ������ؽ��յ������ݳ��ȣ�ʧ�ܣ�����-1
 */
int sw_httpclient_recv_data( HANDLE h_http_client, char* buf, int size, int timeout );

/**
 * @brief ����HTTP����Ӧͷ����
 * @param h_http_client �������Ӻ���׽���
 * @param buf ������Ӧͷ�Ļ�����
 * @param size ������Ӧͷ�Ļ�����������󳤶�
 * @param timeout ���ճ�ʱ
 * 
 * @return ������Ӧͷ�����ݳ���(������������Ȳ����п���û������Ӧͷ��)
*/
int sw_httpclient_recv_header( HANDLE h_http_client, char* buf, int size, int timeout );

/** 
 * @brief ȡ��Http��Ӧͷ��������ֶ�
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param bufsize HTTP����ͷ��������С
 * @param name ͷ����������Ҫ����':'��
 * @param namelen ͷ������������
 * @param value ͷ������ֵ������-----���Ϊ�յػ�ʹ��m_tmp_field(��http����ʱ�᲻׼ȷ)
 * @param valuesize ͷ������ֵ����������
 * 
 * @return 
 */
char* sw_httpclient_get_field_value(char* head_buf, int bufsize, char *name, int namelen, char *value, int valuesize);

/** 
 * @brief ȡ�ø��س���"Content-Length:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return �ɹ����ظ��س��ȣ�ʧ�ܷ���-1
 */
int64_t sw_httpclient_get_content_size( char* head_buf, int size );

/** 
 * @brief ȡ�÷���ֵ
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return �ɹ������ط���ֵ��С��ʧ�ܣ�����-1
 */
int sw_httpclient_get_ret_code( char* head_buf, int size );

/** 
 * @brief �õ�http header�ĳ���
 * 
 * @param head_buf HTTP����ͷ����
 * @param size ��������С
 * 
 * @return �ɹ�������http header�ĳ��ȣ�ʧ�ܣ�����-1
 */
int sw_httpclient_get_header_size( char* head_buf, int size );

/** 
 * @brief ע��cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param cookies Ԥ���õ�cookies��Ż�����
 * 
 */
//ԭ����  void sw_httpclient_register_cookies( HANDLE h_http_client, char* cookies );
int sw_httpclient_register_cookies( HANDLE h_http_client, char* cookies );

/** 
 * @brief ���cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 */
void sw_httpclient_clear_cookies( HANDLE h_http_client );

/** 
 * @brief ȡ��cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return h_http_client cookie �洢��
 */
char* sw_httpclient_get_cookies( HANDLE h_http_client );

/** 
 * @brief ע��User Agent
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param useragent Ԥ���õ�User Agent��Ż�����,��ʽUser-Agent: 
 * 
 * @return 
 */
int sw_httpclient_register_useragent( HANDLE h_http_client, const char* useragent );

/** 
 * @brief ���User Agent
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return
 */
void sw_httpclient_clear_useragent( HANDLE h_http_client );

/** 
 * @brief ȡ��User Agent
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return h_http_client cookie �洢��
 */
char* sw_httpclient_get_useragent( HANDLE h_http_client );

/** 
 * @brief ����HTTP�汾��
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param version Ԥ���õ�HTTP�汾��
 * 
 */
void sw_httpclient_set_version( HANDLE h_http_client, char* version );

/** 
 * @brief  ȡ��HTTP client ��socket
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return  �ɹ�,����http client���ӵ�socket;ʧ��,����-1
 */
int sw_httpclient_get_skt( HANDLE h_http_client );

int sw_httpclient_get_err_code(HANDLE h_http_client);

void sw_httpclient_set_err_code(HANDLE h_http_client, int code);

unsigned short sw_httpclient_get_listen_port( );

/** 
 * @brief �ж�,ȡ�ø��س���"Content-Length:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return true����content-length��false������
 */
bool sw_httpclient_check_content_size( char* head_buf, int size, int64_t *contentsize );

/** 
 * @brief ȡ�ø����ļ��ĳ��ȷ�Χ"Content-Range:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	startpos http����ͷָʾ���ļ�Ƭ����ʼλֵ
 *	endpos http����ͷָʾ���ļ�Ƭ�ν���λ��(�����Content-Length,�䣺endpos-startpos+1 = Content-Length)
 *	filesize http����ͷָʾ���ļ��ܴ�С
 * @return �Ƿ���range�ֶ�
 */
bool sw_httpclient_get_content_range( char* head_buf, int size,  int64_t *startpos, int64_t *endpos, int64_t *filesize);

/** 
 * @brief ȡ�ø����ļ���"Content-MD5:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value md5������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ���md5У��,NULL������
 */
char* sw_httpclient_get_content_md5( char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief ȡ�ø����ļ��ı��뷽ʽ"Content-Encoding:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value encoding������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ��ڱ��룬NULL�����ڱ����ֶ�,ʹ�÷�ѹ���ļ�����
 */
char* sw_httpclient_get_content_encoding(char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief ����HTTP���ӿɽ��ܵı��뷽ʽ
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param encoding Ԥ���õ�HTTP���뷽ʽ
 * 
 * @return 
 */
void sw_httclient_set_encoding(HANDLE h_http_client, char* encoding );

/** 
 * @brief ����HTTP���ӿɽ��ܵ�����
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param language Ԥ���õ�HTTP����
 * 
 * @return 
 */
void sw_httclient_set_language(HANDLE h_http_client, const char* language );

/** 
 * @brief ����HTTP���ӵ�If-None-Match
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param etag	ETag��
 * 
 * @return 
 */
void sw_httpclient_set_etag(HANDLE h_http_client, char* etag );

/** 
 * @brief ȡ�ø����ļ���HTTP ETag
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value type������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize ����������
 * @return �ǿմ��ڱ��룬NULL�����ڱ����ֶ�,ʹ�÷�ѹ���ļ�����
 */
char* sw_httpclient_get_etag(char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief ȡ�ø����ļ��Ĵ�������"Transfer-Encoding:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value Transfer Encoding������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ��ڴ������룬NULL������
 */
 char* sw_httpclient_get_transfer_encoding(char* head_buf, int size,  char *value, int valuesize);
 
/** 
 * @brief ȡ�ø����ļ���"Location:"�ض���
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value location������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ����ض���NULL�������ض���
 */
 char* sw_httpclient_get_location(char* head_buf, int size,  char *value, int valuesize);
 
/** 
 * @brief ȡ�ø����ļ��Ĺ�����"Set-Cookie:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value ������Cookie������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ��ڹ���Cookie��NULL������
 */
char* sw_httpclient_get_set_cookie(char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief �жϷ������Ƿ�֧��byte-range requests"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return true֧��byte-range requests��false��֧��
 */
bool sw_httpclient_check_accept_ranges(char* head_buf, int size);

/** 
 * @brief ����HTTP���ӵ�˽���ֶ�(����ͨ)
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param extra ˽���ֶ�
 * 
 * @return 
 */
void sw_httpclient_set_extra(HANDLE h_http_client, char* extra);

/** 
 * @brief ����HTTP���ӵ�˽���ֶ�(����ͨ)
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param extra ˽���ֶ�
 * 
 * @return 
 */
void sw_httpclient_set_getrange(HANDLE h_http_client, const char* getrange);

/** 
 * @brief ȡ�ø����ļ��Ĳ��ŷ�Χ"PlayTime-Range:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	start http����ͷָʾ���ļ�Ƭ����ʼʱ��
 *	end http����ͷָʾ���ļ�Ƭ�ν���ʱ��
 *	duration����ͷָʾ���ļ��ɹ�����ʱ��
 * @return �Ƿ���PlayTime-Range�ֶ�
 */
bool sw_httpclient_get_playtime_range( char* head_buf, int size,  int64_t *start, int64_t *end, int64_t *duration);

/** 
 * @brief ����HTTP����ͷ���ֶ���Щ����Ҫ����
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param ex_field ͷ���ֶ���
 *								Accept-language
 *								Accept-Encoding
 *								User-Agent
 *								Pragma
 *								Cache-Control
 *								Cookie
 * 
 * @return 
 */
void sw_httpclient_set_exclude_field(HANDLE h_http_client, const char* ex_field);

#ifdef __cplusplus
}
#endif

#endif /* __SWHTTPCLIENT_H__ */

