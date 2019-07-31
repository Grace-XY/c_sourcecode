/** 
 * @file swhttpclient.h
 * @brief HTTP�����ӿ�
 * @author NiuJiuRu
 * @date 2007-10-29
 */
#include "swapi.h"
#include "swtcp.h"
#include "swurl.h"
#include "swlog.h"
#include "swmutex.h"
#include "swthrd.h"
#include "swhttpauth.h"
#include "base64.h"
#include "swbase.h"
#include "swcommon_priv.h"
#include "swhttp_priv.h"





static http_client_t m_all[MAX_HTTPCLIENT_NUM];
static int m_ref = -1;
static HANDLE m_mutex = NULL;

/* �ݴ�ͷ�������ֵ(������÷�û�������Ի���) */
static char m_tmp_field[1024];
/* ȫ��Cookies */
static char m_sz_cookies[1024] = {0,};
//static int m_error_code=HTTP_OK; 
/* http�����˿ں� */
static unsigned short m_http_listen_port = 0;
/* ȫ��User-Agent */
static char m_user_agent[128];

/* ����http������Ҫ�Ŀռ� */
static void *httpclient_malloc(int size)
{
	int i = 0;
	http_client_t *client = NULL;
	if( m_mutex == NULL)	
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if( m_ref < 0 )
	{
		m_ref = 0;

		memset( m_all, 0, sizeof(m_all) );
		for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
			m_all[i].skt = -1;
	}
	for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
	{
		if( m_all[i].used == 0 )
		{
			m_all[i].used = 1;
			m_all[i].include_Cookie = true;
			m_all[i].include_Accept_Encoding = true;
			m_all[i].include_Accept_language = true;
			m_all[i].include_User_Agent = true;
			m_all[i].include_Pragma = true;
			m_all[i].include_Cache_Control = true;
			m_all[i].sz_cookies[0] = '\0';
			m_all[i].user_agent[0] = '\0';
			m_all[i].encoding[0] = '\0';
			m_all[i].language[0] = '\0';
			m_all[i].etag[0] = '\0';
            m_all[i].shutdown = false;
      m_all[i].extra[0] = '\0';
			m_all[i].skt = -1;
			client = m_all + i;
	    m_ref++;
			break;
		}
	}
	
	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	return client;
}
/* �ͷŷ����httpclient  */
static void httpclient_free(void* client)
{
	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if ( client )
	{
		int i = 0;
		for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
		{
			if (client == (void*)&m_all[i])
			{
				int skt = ((http_client_t*)client)->skt;
				((http_client_t*)client)->skt = -1;
				if( 0 <= skt )
					sw_tcp_close( skt );
				((http_client_t*)client)->used = 0;
			   m_ref--;
			   break;
			}
		}
	}

	if(m_mutex)
		sw_mutex_unlock(m_mutex);
}

HANDLE sw_httpclient_init(unsigned long  ip, unsigned short port)
{
	unsigned long unblock = 1;
	http_client_t* p_http_client = NULL; 
	p_http_client = httpclient_malloc( sizeof(http_client_t));	
    if( p_http_client == NULL )
    {
        SWCOMMON_LOG_ERROR("httpclient_malloc FAILED!\n");
		goto ERROR_EXIT;
    }

	strlcpy(p_http_client->version, "1.1", sizeof(p_http_client->version));
	p_http_client->ip = ip;
	p_http_client->port = port;
	
	/* ����socket */
	p_http_client->skt = sw_tcp_socket();
	if( p_http_client->skt < 0 )
    {
        SWCOMMON_LOG_ERROR("sw_tcp_socket FAILED! ret:%d\n", p_http_client->skt);
		goto ERROR_EXIT;
    }
	/* ����Ϊ������ */
	if( sw_tcp_ioctl( p_http_client->skt, FIONBIO, &unblock ) < 0 )
    {
        SWCOMMON_LOG_ERROR("sw_tcp_ioctl FAILED!\n");
		goto ERROR_EXIT;
    }
	{
		int reuse = 1;
		setsockopt(p_http_client->skt , SOL_SOCKET, SO_REUSEADDR, 
				   (char *)&reuse, sizeof(reuse));	
		//setsockopt( p_http_client->skt, SOL_SOCKET, SO_LINGER,(void*) &lingerOpt, sizeof(lingerOpt) );
	}
    return p_http_client;

ERROR_EXIT:
	SWCOMMON_LOG_ERROR("init failed, errno:%d, error:%s\n", errno, strerror(errno));
    if( p_http_client )
        httpclient_free(p_http_client);
	return NULL;
}

/***********************************************************************************************
* CONTENT: �ж���Http����������������
* PARAM: Ҫ�Ͽ����׽���  
* RETURN:
* NOTE:
************************************************************************************************/
void sw_httpclient_shutdown ( HANDLE h_http_client)
{
	if (h_http_client == NULL)
		return;
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
#ifdef SUPPORT_HTTPS
	if (p_http_client->isHttps == true)
		httpsclient_shutdown(h_http_client);
	else
#endif
	{
		if( 0 <= p_http_client->skt && p_http_client->shutdown == false)
		{
            
			p_http_client->shutdown = true;
			sw_tcp_shutdown( p_http_client->skt);
		}
	}

	return ;
}
void sw_httpclient_close ( HANDLE h_http_client)
{
	if (h_http_client == NULL)
		return;
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
#ifdef SUPPORT_HTTPS
	if (p_http_client->isHttps)
		httpsclient_shutdown(h_http_client);
	else
#endif
	{
		int skt = p_http_client->skt;
		p_http_client->skt = -1;
		if( 0 <= skt )
			sw_tcp_close( skt );
	}

	return ;
}
#include "swhttpclient_v6.c"

HANDLE sw_httpclient_connect_ext( unsigned long  ip, unsigned short port, int timeout, bool is_https)
{
#ifdef SUPPORT_HTTPS
	if(is_https)
		return sw_httpsclient_connect(ip, port, timeout);
	else
#endif
		return sw_httpclient_connect(ip, port, timeout);
}

/***********************************************************************************************
* CONTENT: ��Http��������������
* PARAM:
	[in] ip: http��������ַ
	[in] port: http�������˿�
* RETURN:
	��������״̬
* NOTE:
************************************************************************************************/
HANDLE sw_httpclient_connect( unsigned long  ip, unsigned short port, int timeout )
{
	http_client_t* p_http_client = NULL; 
	unsigned long unblock = 1;
	fd_set wset, rset;
	int n;
	int retcode;
	unsigned int now;
	static bool first = true;	
	struct linger lingerOpt;	
	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;
	
	char ipaddr[32] = {"0.0.0.0"};
	inet_ntop(AF_INET, ((struct in_addr*)&ip), ipaddr, sizeof(ipaddr));//inet_ntoa
	// m_error_code=HTTP_OK; 
	p_http_client = httpclient_malloc( sizeof(http_client_t));	
	SWCOMMON_LOG_DEBUG("%p, %s:%d, timeout=%d, ref=%d\n", p_http_client, ipaddr, ntohs(port), timeout, m_ref);

	if( p_http_client == NULL )
    {
        SWCOMMON_LOG_ERROR("httpclient_malloc FAILED!\n");
		goto ERROR_EXIT;
    }

	strlcpy(p_http_client->version, "1.1", sizeof(p_http_client->version));
	p_http_client->ip = ip;
	p_http_client->port = port;
//	p_http_client->error_code = HTTP_OK;
	sw_httpclient_set_err_code(p_http_client,HTTP_OK);
	if( m_sz_cookies[0] != '\0' )//m_sz_cookies��Ҫȷ���н��������һ����Сһ��
	  	strlcpy(p_http_client->sz_cookies, m_sz_cookies, sizeof(p_http_client->sz_cookies));
	else
	  	memset( p_http_client->sz_cookies, 0, sizeof(p_http_client->sz_cookies) );

	/* ����socket */
	p_http_client->skt = sw_tcp_socket();
	if( p_http_client->skt < 0 )
    {
        SWCOMMON_LOG_ERROR("sw_tcp_socket FAILED! ret:%d\n", p_http_client->skt);
		goto ERROR_EXIT;
    }
	/* ����Ϊ������ */
	if( sw_tcp_ioctl( p_http_client->skt, FIONBIO, &unblock ) < 0 )
    {
        SWCOMMON_LOG_ERROR("sw_tcp_ioctl FAILED!\n");
		goto ERROR_EXIT;
    }
	{
		int reuse = 1;
		setsockopt(p_http_client->skt , SOL_SOCKET, SO_REUSEADDR, 
				   (char *)&reuse, sizeof(reuse));	
		//setsockopt( p_http_client->skt, SOL_SOCKET, SO_LINGER,(void*) &lingerOpt, sizeof(lingerOpt) );
	}
    
	now = sw_thrd_get_tick();

	if( first )
	{
		srand( now );
		m_http_listen_port = htons( (unsigned short) (30000 + rand()%26000 ));
		sw_tcp_bind(p_http_client->skt, INADDR_ANY, m_http_listen_port);
		first = false;
	}
	/* ����... */
	errno = 0;
	retcode = sw_tcp_connect( p_http_client->skt, ip, port );
	if(retcode == 0)
		return p_http_client;
	if(errno != 0 && errno != EINPROGRESS)
	{
		sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_DENY);
		SWCOMMON_LOG_ERROR("[HTTPCLIENT] connect error:%d ret=%d\n", retcode, errno);
		goto ERROR_EXIT;
	}
	
	n=0;
WAIT:
	/* �ȴ����ӳɹ� */
	if( (retcode = sw_tcp_select( p_http_client->skt, &rset, &wset, NULL, timeout )) < 0 )
    {
        sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
        SWCOMMON_LOG_ERROR("sw_tcp_select FAILED! ret:%d\n", retcode);
		goto ERROR_EXIT;
    }
	if(retcode == 0)
	{
        sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_TIMEOUT);
		char ipaddr[32] = {"0.0.0.0"};
		inet_ntop(AF_INET, ((struct in_addr*)&ip), ipaddr, sizeof(ipaddr));//inet_ntoa
		SWCOMMON_LOG_ERROR("[HTTPCLIENT]select socket timeout! %s:%d\n", ipaddr, ntohs(port));
		goto ERROR_EXIT;
	}

	if( FD_ISSET( p_http_client->skt, &rset ) )
	{
		char sz_buf[16];
		int readsize = sw_tcp_recv( p_http_client->skt, sz_buf, sizeof(sz_buf) );
		if( readsize==-1 && (errno==EWOULDBLOCK || errno==EINPROGRESS) )
		{
			if( ++n<3 )
				goto WAIT;
		}
	}
	else if( FD_ISSET( p_http_client->skt, &wset ) )
	{
		int err;
		int len = sizeof(err);
#ifndef WIN32
		if (getsockopt(p_http_client->skt, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0) 
#else
		if (getsockopt(p_http_client->skt, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0) 
#endif
		{
			SWCOMMON_LOG_ERROR("[HTTPCLIENT]getsocketopt failed!\n");
			goto ERROR_EXIT;
		}

		if (err) {
			SWCOMMON_LOG_ERROR("[HTTPCLIENT] connect failed erron:%d!\n", err);
			goto ERROR_EXIT;
		}
		return p_http_client ;
	}
	
	if(n==2)
		sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_NORESPONSE);
	else
	{
		if( errno==ECONNREFUSED )
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_DENY);
		else
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_NORESPONSE);
	}
	
ERROR_EXIT:
	SWCOMMON_LOG_ERROR("connect failed, errno:%d, error:%s\n", errno, strerror(errno));
    if(p_http_client)
        httpclient_free(p_http_client);
	return NULL;
}

bool sw_httpclient_connect2( HANDLE h_http_client,  int timeout )
{
    if(h_http_client == NULL )
    {
        SWCOMMON_LOG_ERROR("connect failed, http client is NULL");
        return false;
    }
    http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if (p_http_client->used == 0 || p_http_client->isHttps == true || p_http_client->skt < 0)
	{
		SWCOMMON_LOG_ERROR("%p used %d %s %s reconnect failed", p_http_client, p_http_client->used, p_http_client->isHttps ? "https" : "http", p_http_client->skt < 0 ? "unopen" : "open");
		return false;
	}
    fd_set wset, rset;
    int n;
    int retcode;
    unsigned int now;
    static bool first = true;	
    struct linger lingerOpt;	
    lingerOpt.l_onoff = 1;
    lingerOpt.l_linger = 0;

    // m_error_code=HTTP_OK; 
	char ipaddr[32] = {"0.0.0.0"};
	inet_ntop(AF_INET, ((struct in_addr*)&p_http_client->ip), ipaddr, sizeof(ipaddr));//inet_ntoa
    SWCOMMON_LOG_DEBUG("%p, %s:%d, timeout=%d, ref=%d\n",p_http_client, ipaddr, 
            ntohs(p_http_client->port), timeout, m_ref);
	//p_http_client->error_code = HTTP_OK;
	sw_httpclient_set_err_code(p_http_client,HTTP_OK);

    if( m_sz_cookies[0] != '\0' )//m_sz_cookies��Ҫȷ���н��������һ����Сһ��
        strlcpy(p_http_client->sz_cookies, m_sz_cookies, sizeof(p_http_client->sz_cookies));
    else
        memset( p_http_client->sz_cookies, 0, sizeof(p_http_client->sz_cookies) );

    now = sw_thrd_get_tick();
    if( first )
    {
        srand( now );
        m_http_listen_port = htons( (unsigned short) (30000 + rand()%26000 ));
        sw_tcp_bind(p_http_client->skt, INADDR_ANY, m_http_listen_port);
        first = false;
    }
    /* ����... */
    errno = 0;
    retcode = sw_tcp_connect( p_http_client->skt, p_http_client->ip, p_http_client->port );
    if(retcode == 0)
        return true;
    if(errno != 0 && errno != EINPROGRESS)
    {
        sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_DENY);
        SWCOMMON_LOG_ERROR("[HTTPCLIENT] connect error:%d ret=%d\n", retcode, errno);
        goto ERROR_EXIT;
    }

    n=0;
WAIT:
    /* �ȴ����ӳɹ� */
    if( (retcode = sw_tcp_select( p_http_client->skt, &rset, &wset, NULL, timeout )) < 0 )
    {
        sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
        SWCOMMON_LOG_ERROR("sw_tcp_select FAILED! ret:%d\n", retcode);
        goto ERROR_EXIT;
    }
    if(retcode == 0)
    {
        sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_TIMEOUT);
		char ipaddr[32] = {"0.0.0.0"};
		inet_ntop(AF_INET, ((struct in_addr*)&p_http_client->ip), ipaddr, sizeof(ipaddr));//inet_ntoa
        SWCOMMON_LOG_ERROR("[HTTPCLIENT]select socket timeout! %s:%d\n", ipaddr, ntohs(p_http_client->port));
        goto ERROR_EXIT;
    }

    if( FD_ISSET( p_http_client->skt, &rset ) )
    {
        char sz_buf[16];
        int readsize = sw_tcp_recv( p_http_client->skt, sz_buf, sizeof(sz_buf) );
        if( readsize==-1 && (errno==EWOULDBLOCK || errno==EINPROGRESS) )
        {
            if( ++n<3 )
                goto WAIT;
        }
    }
    else if( FD_ISSET( p_http_client->skt, &wset ) )
    {
        int err;
        int len = sizeof(err);
#ifndef WIN32
        if (getsockopt(p_http_client->skt, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0) 
#else
            if (getsockopt(p_http_client->skt, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0) 
#endif
            {
                SWCOMMON_LOG_ERROR("[HTTPCLIENT]getsocketopt failed!\n");
                goto ERROR_EXIT;
            }

        if (err) {
            SWCOMMON_LOG_ERROR("[HTTPCLIENT] connect failed erron:%d!\n", err);
            goto ERROR_EXIT;
        }
        return true ;
    }

    if(n==2)
        sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_NORESPONSE);
    else
    {
        if( errno==ECONNREFUSED )
            sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_DENY);
        else
            sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_PORT_NORESPONSE);
    }

ERROR_EXIT:
    SWCOMMON_LOG_ERROR("connect failed, errno:%d, error:%s\n", errno, strerror(errno));
    return false;
}

/***********************************************************************************************
* CONTENT: �Ͽ���Http����������������
* PARAM: Ҫ�Ͽ����׽���  
* RETURN:
	���ӷ������Ƿ�ɹ�
* NOTE:
************************************************************************************************/
void sw_httpclient_disconnect( HANDLE h_http_client )
{
	if(h_http_client == NULL)
		return;
	http_client_t* p_http_client = ( http_client_t* )h_http_client;
#ifdef SUPPORT_HTTPS
	if (p_http_client->isHttps == true)
		sw_httpsclient_disconnect(h_http_client);
	else
#endif
		httpclient_free(h_http_client);
}

/***********************************************************************************************
* CONTENT: ����HTTP Request
* PARAM:
	[in] skt:�������Ӻ���׽��� 
	[in] method: ����ʽ
	[in] url:�����URL
	[in] host:��������
	[in] accept_type:���յ��ļ�����
	[in] content_type:post ����������
	[in] content:post ������
	[in] length: post ���ݳ���
	[in] soap_action: soap action URI
	[in] http_auth: ժҪ��֤��Ϣ
* RETURN:
	�����Ƿ��ͳɹ�
* NOTE:
************************************************************************************************/
bool sw_httpclient_request( HANDLE h_http_client, char* method, char* url, char* host,
							char *accept_type, char* content_type, char* content, int length,
							int timeout, char* soap_action, http_authentcation_t* http_auth )
{
	if (h_http_client == NULL)
		return false;
	length = (length>0) ? length : 0;
	fd_set wset;	
	char* p_buf = NULL;
	char* p_req_buf = NULL;
	int   reqlen = length+8*1024;
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if (p_http_client->used == 0)
		return false;
#ifdef SUPPORT_HTTPS
	if (p_http_client->isHttps == true)
		return sw_httpsclient_request(h_http_client, method, url, host, accept_type, content_type, content, length, timeout, soap_action, http_auth);
#endif
	ahash_hex ha1;
	ahash_hex ha2 = "";
	ahash_hex m_response;
	char c_nonce[48];
	char sz_nonce_count[9] = "00000001";
	int i =0;
	int ret = 0;
	int pos = 0;
	if( p_http_client == NULL || method == NULL || url == NULL)
		return false;
	if(strcmp( method, "GET" ) == 0)
		reqlen = 8*1024;
	
	p_req_buf = (char*)malloc(reqlen);
	if( p_req_buf==NULL )
		return false;
	p_buf =  p_req_buf;

	memset( c_nonce, 0, sizeof(c_nonce) );

	/* method */
	pos += strlcpy( &p_buf[pos], method, pos < reqlen ? reqlen-pos : 0);
	pos += strlcpy( &p_buf[pos], " ", pos < reqlen ? reqlen-pos : 0);
	
	/* url */
	pos += strlcpy( &p_buf[pos], url, pos < reqlen ? reqlen-pos : 0);//sw_url_encode(url,p_buf);
	/* �汾 */
	pos += strlcpy( &p_buf[pos], " HTTP/1.1", pos < reqlen ? reqlen-pos : 0);
	
	/* host information */
	if(host && host[0] != '\0')
	{
		pos += snprintf(&p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nHost: %s", host);
	}
	
	/* �������� */
	pos += strlcpy( &p_buf[pos], "\r\nAccept: ", pos < reqlen ? reqlen-pos : 0);
	if( accept_type && accept_type[0] != '\0')
	{
		pos += strlcpy( &p_buf[pos], accept_type, pos < reqlen ? reqlen-pos : 0 );
	}
	else
	{
		pos += strlcpy( &p_buf[pos], "*/*", pos < reqlen ? reqlen-pos : 0 );
	}
	
	/* soap action */
	if ( soap_action != NULL ) 
	{
		if( soap_action[0] != '\0' )
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nSOAPAction: %s",soap_action);
		else
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nSOAPAction:");		
	}
	/* ���ժҪ��֤��Ϣ */
	if ( http_auth != NULL ) 
	{
		if( strcmp(http_auth->arithmetic,"Basic") ==0 )
		{
			char user_pass[128];
			char enc_user_pass[256];
			int  enc_length = 0;
			pos += strlcpy( &p_buf[pos], "\r\nauthorization: Basic ", pos < reqlen ? reqlen-pos : 0);
			if (snprintf(user_pass, sizeof(user_pass), "%s:%s", http_auth->user_name ,http_auth->user_pwd) >= (int)sizeof(user_pass))
				SWCOMMON_LOG_ERROR("%s authorization user and pwd too long", url);
			enc_length = base64encode((const unsigned char*)user_pass,strlen(user_pass),(unsigned char*)enc_user_pass,sizeof(enc_user_pass)-1);
			enc_user_pass[enc_length] = '\0';
			pos += strlcpy( &p_buf[pos], enc_user_pass, pos < reqlen ? reqlen-pos : 0);
		}
		else
		{
			// USERNAME
			pos += strlcpy( &p_buf[pos],  "\r\nAuthorization: Digest username=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->user_name, pos < reqlen ? reqlen-pos : 0);

			// REALM
			pos += strlcpy( &p_buf[pos], "\", realm=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->realm, pos < reqlen ? reqlen-pos : 0);
			// NONCE
			pos += strlcpy( &p_buf[pos], "\", nonce=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->nonce, pos < reqlen ? reqlen-pos : 0);

			// URI
			pos += strlcpy( &p_buf[pos], "\", uri=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->uri, pos < reqlen ? reqlen-pos : 0);

			// QOP
			pos += strlcpy( &p_buf[pos], "\", qop=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->qop, pos < reqlen ? reqlen-pos : 0);

			// NC
			pos += strlcpy( &p_buf[pos], "\", nc=", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], sz_nonce_count, pos < reqlen ? reqlen-pos : 0);

			// CNONCE
			srand( (unsigned)time( NULL ) + rand()*2 );
			snprintf( c_nonce, sizeof(c_nonce), "%d", rand() ) ;
			//snprintf( c_nonce, sizeof(c_nonce), "%s", http_auth->nonce );

			pos += strlcpy( &p_buf[pos], ", cnonce=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], c_nonce, pos < reqlen ? reqlen-pos : 0);

			// RESPONSE
			digest_calc_ha1(http_auth->arithmetic, http_auth->user_name, http_auth->realm, http_auth->user_pwd, http_auth->nonce, c_nonce, ha1);
			digest_calc_respose(ha1, http_auth->nonce, sz_nonce_count, c_nonce, http_auth->qop, method, http_auth->uri, ha2, m_response);

			pos += strlcpy( &p_buf[pos], "\", response=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], m_response, pos < reqlen ? reqlen-pos : 0);
		
			// OPAQUE
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\", opaque=\"%s\"", http_auth->opaque);
		}
	}
	
	if( strcmp( method, "POST" ) == 0 )
	{
		if( length >0 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nContent-Length: %d", length );
		}
		else
		{
			pos += strlcpy( &p_buf[pos], "\r\nContent-Length: 0", pos < reqlen ? reqlen-pos : 0);
		}
		if( length >0 )
		{ 
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nContent-Type: %s", content_type );
		}
	}
	else if( strcmp( method, "GET" ) == 0 )
	{
		if( length >0 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nRange: bytes=%d-%s", length, content && strchr(content, '-')==NULL ? content : "" );
		}
		else if( content && strchr(content, '-') )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nRange: bytes=%s", content );
		}
	}
	/* ���������Լ�����:identityȱ��Ĳ�ʹ���κ�ת��,���ʹ��gzip, deflate�Ļ��еķ������ᴫ��ѹ�����ļ� */
	if (p_http_client->include_Accept_language)
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nAccept-language: %s", p_http_client->language[0] == '\0' ? "zh-cn" : p_http_client->language);
	if (p_http_client->include_Accept_Encoding)
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nAccept-Encoding: %s", p_http_client->encoding[0] == '\0' ? "identity" : p_http_client->encoding);
	
	if (p_http_client->etag[0] != '\0')
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nIf-None-Match: %s", p_http_client->etag);

	/* user-agent */
	if (p_http_client->include_User_Agent)
	{
		if (p_http_client->user_agent[0] == '\0' && m_user_agent[0] == '\0')
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nUser-Agent: Mozilla/4.0 (compatible; MS IE 6.0; (ziva))");
		else if (p_http_client->user_agent[0] == '\0' && m_user_agent[0] != '\0')
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\n%s", m_user_agent);
		else
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\n%s", p_http_client->user_agent);
	}

	if ( p_http_client->include_Pragma )
		pos += strlcpy( &p_buf[pos], "\r\nPragma: no-cache", pos < reqlen ? reqlen-pos : 0);
	
	if ( p_http_client->include_Cache_Control )
		pos += strlcpy( &p_buf[pos], "\r\nCache-Control: no-cache", pos < reqlen ? reqlen-pos : 0);

	if( p_http_client->include_Cookie && p_http_client->sz_cookies[0] != '\0' )
	{
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0,"\r\n%s", p_http_client->sz_cookies );
	}
	
	if( p_http_client->extra[0] != '\0' )
	{
		if( ((int)strlen( p_http_client->extra ) + pos ) < reqlen - 64 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0,"\r\n%s", p_http_client->extra );
		}
		else
			SWCOMMON_LOG_ERROR("%s extra too long %s", url, p_http_client->extra);
	}
	
	if( strcasecmp( p_http_client->version, "1.0" ) == 0 )
		pos += strlcpy( &p_buf[pos], "\r\nConnection: close\r\n\r\n", pos < reqlen ? reqlen-pos : 0 );
	else
		pos += strlcpy( &p_buf[pos], "\r\nConnection: Keep-Alive\r\n\r\n", pos < reqlen ? reqlen-pos : 0 );
	SWCOMMON_LOG_DEBUG("send request conntent:\n%s\n", !SENSITIVE_PRINT ? "..." : p_req_buf);
	/* ����content */
	if( strcmp( method, "POST" ) == 0 )
	{
		if (content != NULL)
		{
			memcpy( &p_buf[pos], content, (pos+length) <= reqlen ? length : 0);
			pos += length;
		}
	}
	if (reqlen < pos)
	{
		SWCOMMON_LOG_ERROR("%s http header too long %d", url, pos);
		free( p_req_buf );
		sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
		return false;
	}
	i=0;
	reqlen = pos;
	while( i < reqlen )
	{
		/* ���״̬ */
		ret = sw_tcp_select( p_http_client->skt, NULL, &wset, NULL, timeout );
		if( ret<=0 )
		{
			if(ret == 0)
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_TIMEOUT);
			else
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "send errno=%d(%s), ret=%d\n", errno, strerror(errno), ret );
			break;
		}
		if( !FD_ISSET( p_http_client->skt, &wset ) )
		{
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "excute errno=%d(%s)\n", errno, strerror(errno) );
			break;
		}
		/* ����Httpͷ */
		ret =  sw_tcp_send( p_http_client->skt, p_req_buf+i, reqlen-i );
		if( ret <= 0 )
		{
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "send errno=%d(%s), ret=%d\n", errno, strerror(errno), ret );
			break;
		}
		i += ret;
	}
	free( p_req_buf );
	return i==reqlen;
}
/***********************************************************************************************
* CONTENT: ����HTTP Request
* PARAM:
	[in] skt:�������Ӻ���׽��� 
	[in] method: ����ʽ
	[in] url:�����URL
	[in] host:��������
	[in] accept_type:���յ��ļ�����
	[in] filename:�ϴ��ļ�ʱ�ļ�������ļ��� 
	[in] content_type:post ����������
	[in] content:post ������
	[in] length: post ���ݳ���
	[in] soap_action: soap action URI
	[in] http_auth: ժҪ��֤��Ϣ
* RETURN:
	�����Ƿ��ͳɹ�
* NOTE:
************************************************************************************************/
bool sw_httpclient_request_ex2( HANDLE h_http_client, char* method, char* url, char* host,
							char *accept_type,char *filename, char* content_type, char* content, int length,
							int timeout, char* soap_action, http_authentcation_t* http_auth )
{
	length = (length>0) ? length : 0;
	fd_set wset;	
	char* p_buf = NULL;
	char* p_req_buf = NULL;
	int   reqlen = length+8*1024;
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	ahash_hex ha1;
	ahash_hex ha2 = "";
	ahash_hex m_response;
	char c_nonce[48];
	char sz_nonce_count[9] = "00000001";
	int i =0;
	int ret = 0;
	int pos = 0;
	if( p_http_client == NULL || method == NULL || url == NULL)
		return false;
	if(strcmp( method, "GET" ) == 0)
		reqlen = 8*1024;
	
	p_req_buf = (char*)malloc(reqlen);
	if( p_req_buf==NULL )
		return false;
	p_buf =  p_req_buf;

	memset( c_nonce, 0, sizeof(c_nonce) );

	/* method */
	pos += strlcpy( &p_buf[pos], method, pos < reqlen ? reqlen-pos : 0);
	pos += strlcpy( &p_buf[pos], " ", pos < reqlen ? reqlen-pos : 0);
	
	/* url */
	pos += strlcpy( &p_buf[pos], url, pos < reqlen ? reqlen-pos : 0);//sw_url_encode(url,p_buf);
	/* �汾 */
	pos += strlcpy( &p_buf[pos], " HTTP/1.1", pos < reqlen ? reqlen-pos : 0);
	
	/* host information */
	if(host && host[0] != '\0')
	{
		pos += snprintf(&p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nHost: %s", host);
	}
	
	/* �������� */
	pos += strlcpy( &p_buf[pos], "\r\nAccept: ", pos < reqlen ? reqlen-pos : 0);
	if( accept_type && accept_type[0] != '\0')
	{
		pos += strlcpy( &p_buf[pos], accept_type, pos < reqlen ? reqlen-pos : 0 );
	}
	else
	{
		pos += strlcpy( &p_buf[pos], "*/*", pos < reqlen ? reqlen-pos : 0 );
	}
	
	/* soap action */
	if ( soap_action != NULL ) 
	{
		if( soap_action[0] != '\0' )
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nSOAPAction: %s",soap_action);
		else
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nSOAPAction:");		
	}
	/* ���ժҪ��֤��Ϣ */
	if ( http_auth != NULL ) 
	{
		if( strcmp(http_auth->arithmetic,"Basic") ==0 )
		{
			char user_pass[128];
			char enc_user_pass[256];
			int  enc_length = 0;
			pos += strlcpy( &p_buf[pos], "\r\nauthorization: Basic ", pos < reqlen ? reqlen-pos : 0);
			if (snprintf(user_pass, sizeof(user_pass), "%s:%s", http_auth->user_name ,http_auth->user_pwd) >= (int)sizeof(user_pass))
				SWCOMMON_LOG_ERROR("%s authorization user and pwd too long", url);
			enc_length = base64encode((const unsigned char*)user_pass,strlen(user_pass),(unsigned char*)enc_user_pass,sizeof(enc_user_pass)-1);
			enc_user_pass[enc_length] = '\0';
			pos += strlcpy( &p_buf[pos], enc_user_pass, pos < reqlen ? reqlen-pos : 0);
		}
		else
		{
			// USERNAME
			pos += strlcpy( &p_buf[pos],  "\r\nAuthorization: Digest username=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->user_name, pos < reqlen ? reqlen-pos : 0);

			// REALM
			pos += strlcpy( &p_buf[pos], "\", realm=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->realm, pos < reqlen ? reqlen-pos : 0);
			// NONCE
			pos += strlcpy( &p_buf[pos], "\", nonce=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->nonce, pos < reqlen ? reqlen-pos : 0);

			// URI
			pos += strlcpy( &p_buf[pos], "\", uri=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->uri, pos < reqlen ? reqlen-pos : 0);

			// QOP
			pos += strlcpy( &p_buf[pos], "\", qop=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->qop, pos < reqlen ? reqlen-pos : 0);

			// NC
			pos += strlcpy( &p_buf[pos], "\", nc=", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], sz_nonce_count, pos < reqlen ? reqlen-pos : 0);

			// CNONCE
			srand( (unsigned)time( NULL ) + rand()*2 );
			snprintf( c_nonce, sizeof(c_nonce), "%d", rand() ) ;
			//snprintf( c_nonce, sizeof(c_nonce), "%s", http_auth->nonce );

			pos += strlcpy( &p_buf[pos], ", cnonce=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], c_nonce, pos < reqlen ? reqlen-pos : 0);

			// RESPONSE
			digest_calc_ha1(http_auth->arithmetic, http_auth->user_name, http_auth->realm, http_auth->user_pwd, http_auth->nonce, c_nonce, ha1);
			digest_calc_respose(ha1, http_auth->nonce, sz_nonce_count, c_nonce, http_auth->qop, method, http_auth->uri, ha2, m_response);

			pos += strlcpy( &p_buf[pos], "\", response=\"", pos < reqlen ? reqlen-pos : 0);
			pos += strlcpy( &p_buf[pos], m_response, pos < reqlen ? reqlen-pos : 0);
		
			// OPAQUE
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\", opaque=\"%s\"", http_auth->opaque);
		}
	}
	
	if( strcmp( method, "POST" ) == 0 )
	{
		if( length >0 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nContent-Length: %d", length );
		}
		else
		{
			pos += strlcpy( &p_buf[pos], "\r\nContent-Length: 0", pos < reqlen ? reqlen-pos : 0);
		}
		if( length >0 )
		{ 
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nContent-Type: %s", content_type );
		}
		if(filename)
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? (reqlen-pos) : 0, "\r\nfilename: %s", filename );
		}
	}
	else if( strcmp( method, "GET" ) == 0 )
	{
		if( length >0 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nRange: bytes=%d-%s", length, content && strchr(content, '-')==NULL ? content : "" );
		}
		else if( content && strchr(content, '-') )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nRange: bytes=%s", content );
		}
	}
	/* ���������Լ�����:identityȱ��Ĳ�ʹ���κ�ת��,���ʹ��gzip, deflate�Ļ��еķ������ᴫ��ѹ�����ļ� */
	if (p_http_client->include_Accept_language)
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nAccept-language: %s", p_http_client->language[0] == '\0' ? "zh-cn" : p_http_client->language);
	if (p_http_client->include_Accept_Encoding)
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nAccept-Encoding: %s", p_http_client->encoding[0] == '\0' ? "identity" : p_http_client->encoding);
	
	if (p_http_client->etag[0] != '\0')
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nIf-None-Match: %s", p_http_client->etag);

	/* user-agent */
	if (p_http_client->include_User_Agent)
	{
		if (p_http_client->user_agent[0] == '\0' && m_user_agent[0] == '\0')
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\nUser-Agent: Mozilla/4.0 (compatible; MS IE 6.0; (ziva))");
		else if (p_http_client->user_agent[0] == '\0' && m_user_agent[0] != '\0')
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\n%s", m_user_agent);
		else
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0, "\r\n%s", p_http_client->user_agent);
	}

	if ( p_http_client->include_Pragma )
		pos += strlcpy( &p_buf[pos], "\r\nPragma: no-cache", pos < reqlen ? reqlen-pos : 0);
	
	if ( p_http_client->include_Cache_Control )
		pos += strlcpy( &p_buf[pos], "\r\nCache-Control: no-cache", pos < reqlen ? reqlen-pos : 0);

	if( p_http_client->include_Cookie && p_http_client->sz_cookies[0] != '\0' )
	{
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0,"\r\n%s", p_http_client->sz_cookies );
	}
	
	if( p_http_client->extra[0] != '\0' )
	{
		if( ((int)strlen( p_http_client->extra ) + pos ) < reqlen - 64 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen-pos : 0,"\r\n%s", p_http_client->extra );
		}
		else
			SWCOMMON_LOG_ERROR("%s extra too long %s", url, p_http_client->extra);
	}
	
	if( strcasecmp( p_http_client->version, "1.0" ) == 0 )
		pos += strlcpy( &p_buf[pos], "\r\nConnection: close\r\n\r\n", pos < reqlen ? reqlen-pos : 0 );
	else
		pos += strlcpy( &p_buf[pos], "\r\nConnection: Keep-Alive\r\n\r\n", pos < reqlen ? reqlen-pos : 0 );
	SWCOMMON_LOG_DEBUG("send request conntent:\n%s\n", p_req_buf);
	/* ����content */
	if( strcmp( method, "POST" ) == 0 )
	{
		if (content != NULL)
		{
			memcpy( &p_buf[pos], content, (pos+length) <= reqlen ? length : 0);
			pos += length;
		}
	}
	if (reqlen < pos)
	{
		SWCOMMON_LOG_ERROR("%s http header too long %d", url, pos);
		free( p_req_buf );
		sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
		return false;
	}
	i=0;
	reqlen = pos;
	while( i < reqlen )
	{
		/* ���״̬ */
		ret = sw_tcp_select( p_http_client->skt, NULL, &wset, NULL, timeout );
		if( ret<=0 )
		{
			if(ret == 0)
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_TIMEOUT);
			else
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "send errno=%d(%s), ret=%d\n", errno, strerror(errno), ret );
			break;
		}
		if( !FD_ISSET( p_http_client->skt, &wset ) )
		{
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "excute errno=%d(%s)\n", errno, strerror(errno) );
			break;
		}
		/* ����Httpͷ */
		ret =  sw_tcp_send( p_http_client->skt, p_req_buf+i, reqlen-i );
		if( ret <= 0 )
		{
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "send errno=%d(%s), ret=%d\n", errno, strerror(errno), ret );
			break;
		}
		i += ret;
	}
	free( p_req_buf );
	return i==reqlen;
}

/***********************************************************************************************
* CONTENT: ����HTTP����
* PARAM:
	[in] h_http_client:�������Ӻ���׽���
	[in] buf:���ջ�����
	[in] timeout:���ճ�ʱ
* RETURN:
	���յ������ݳ���
* NOTE:
************************************************************************************************/
int sw_httpclient_recv_data( HANDLE h_http_client, char* buf, int size, int timeout )
{
	if (h_http_client == NULL || buf == NULL || size <= 0)
		return -1;
	http_client_t* p_http_client = ( http_client_t* )h_http_client;
	if (p_http_client->used == 0)
		return -1;
#ifdef SUPPORT_HTTPS
	if (p_http_client->isHttps == true)
		return sw_httpsclient_recv_data(h_http_client, buf, size, timeout);
#endif

	int ret;
	fd_set rset;
	if( p_http_client && ( p_http_client->skt != -1 ) )
	{
		/* ���״̬ */
		if((ret = sw_tcp_select( p_http_client->skt, &rset, NULL, NULL, timeout )) <= 0 )
		{
			if(ret == 0)
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_TIMEOUT);
			else
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
	        SWCOMMON_LOG_ERROR("select error  , errno:%d, error:%s\n", errno, strerror(errno));
			return 0;
		}
		/* �������� */
		if(0 <= p_http_client->skt && FD_ISSET( p_http_client->skt, &rset ) )
		{
			int recvlen = sw_tcp_recv( p_http_client->skt, buf, size );
			if (recvlen == 0)
			{//������Ѿ��ر�
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
				recvlen = -1;
			}
			else if (recvlen == -1 && (errno==EWOULDBLOCK || errno==EINPROGRESS))
			{//
				sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_TIMEOUT);
				recvlen = 0;
			}
			return recvlen;
		}
		else
		{	
			sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_INFO("not in set+++++++++++++++++++++++\n");
			return 0;
		}
	}
	sw_httpclient_set_err_code(p_http_client, HTTP_ERROR_EXCEPTION);
	return -1;
}
/***********************************************************************************************
* CONTENT: ����HTTP����Ӧͷ����
* PARAM:
	[in] h_http_client:�������Ӻ���׽���
	[in] buf:������Ӧͷ�Ļ�����
	[int] size:������Ӧͷ�Ļ�����������󳤶�
	[in] timeout:���ճ�ʱ
* RETURN:
	������Ӧͷ�����ݳ���(������������Ȳ����п���û������Ӧͷ��)
* NOTE:
************************************************************************************************/
int sw_httpclient_recv_header( HANDLE h_http_client, char* buf, int size, int timeout )
{
	int i = 0;
	char *p = buf;
	char *end = buf+size;
	int rlen = 16;
	char *t = &buf[4];
	if ( p == NULL || size <= 20 )
		return 0;
	while (p < end)
	{
		i = sw_httpclient_recv_data(h_http_client, p, rlen, timeout);
		if ( i <= 0)
			return (int)(p - buf);
		p += i;
		if ( p >= t )
		{
			if ( *(p-4) == '\r' && *(p-3) == '\n' && *(p-2) == '\r' && *(p-1) == '\n' )
				break;
			else if ( *(p-3) == '\r' && *(p-2) == '\n' && *(p-1) == '\r' )
				rlen = 1;
			else if ( *(p-2) == '\r' && *(p-1) == '\n' )
				rlen = 2;
			else if ( *(p-1) == '\r')
				rlen = 3;
			else
				rlen = 4;
			rlen = (p-end) >= rlen ? rlen : 1;
		}
	}
    SWCOMMON_LOG_ERROR("recv response conntent:\n%s\n", buf);
	return 	(int)(p - buf);
}
/** 
 * @brief ȡ�÷���ֵ
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return 
 */
int sw_httpclient_get_ret_code( char* head_buf, int size )
{
	int i;
	char sz_buffer[20];

	char* buf = head_buf;
	if( buf == NULL )
		return -1;

	if( !strncasecmp( buf, "http/", 5 ) )
	{
		buf += 5;
		while( ( *buf >= '0' && *buf <= '9' ) || (*buf == '.') )
			buf++;
	}

	while( *buf == ' ' )
		buf++;

	for(i = 0; i < 10 && *buf >= '0' && *buf <= '9'; i++, buf++ )
	{
		sz_buffer[i] = *buf;
	}
	sz_buffer[i] = 0;
	return atoi( sz_buffer );
}

static int64_t str_int64( char* str )
{
	int64_t id = 0;
	while(*str == ' ' || *str == '\t')
		str++;
	while( '0' <= *str && *str <= '9' )
	{
		id = id * 10 + ( *str - '0' );
		str++;
	}
	return id;
}

/* ��ȡhttp��Ӧͷ���ָ���ֶ�����ֵ,���뱣֤value�ǿ� */
static char* httpclient_get_field_value(char* head_buf, int bufsize, char *name, int namelen, char *value, int valuesize)
{
	int i=0;
	if ( name == NULL || namelen <= 0 )
		return NULL;
	if ( value == NULL || valuesize <= 0)
	{
		value = m_tmp_field;
		valuesize = sizeof(m_tmp_field);
	}
	if ( valuesize > 0 )
		value[valuesize-1]  = value[0] = '\0';
	while( i < bufsize )
	{
		if( !strncasecmp( head_buf + i, name, namelen ) )
		{
			int j = 0;
			i += namelen;	
			while (i < bufsize && ( head_buf[i] == ' ' ||  head_buf[i] == '\t' ) )
				i++;
			if ( valuesize > 0 )
			{
				valuesize--;
				while ( i < bufsize && j < valuesize  && head_buf[i] != '\r' && head_buf[i] != '\n' )
				{//��׼ȷ�����ж�\r\n??
					value[j++] = head_buf[i++];
				}
				value[j] = '\0';	
			}
			return value;	
		}
		while( i < bufsize && head_buf[i] != '\n' )
			i++;
		i++;
	}
	return NULL;
}
/** 
 * @brief ȡ��Http��Ӧͷ��������ֶ�
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param bufsize HTTP����ͷ��������С
 * @param name ͷ��������
 * @param namelen ͷ������������
 * @param value ͷ������ֵ������-----���Ϊ�յػ�ʹ��m_tmp_field(��http����ʱ�᲻׼ȷ)
 * @param valuesize ͷ������ֵ����������
 * 
 * @return 
 */
char* sw_httpclient_get_field_value(char* head_buf, int bufsize, char *name, int namelen, char *value, int valuesize)
{
	return httpclient_get_field_value(head_buf, bufsize, name, namelen, value, valuesize);
}
/** 
 * @brief ȡ�ø��س���"Content-Length:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return 
 */
int64_t sw_httpclient_get_content_size( char* head_buf, int size )
{
	int64_t contentsize = -1;
	char value[32];
	if ( httpclient_get_field_value(head_buf, size, "Content-Length:", 15, value, sizeof(value)) != NULL)
		contentsize = str_int64(value);
	return contentsize;
}

/** 
 * @brief �ж�,ȡ�ø��س���"Content-Length:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return true����content-length��false������
 */
bool sw_httpclient_check_content_size( char* head_buf, int size, int64_t *contentsize )
{
	char value[32];
	if ( httpclient_get_field_value(head_buf, size, "Content-Length:", 15, value, sizeof(value)) != NULL)
	{
		if ( contentsize )
		 	*contentsize = str_int64(value);
		 return true;
	}
	return false;
}
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
bool sw_httpclient_get_content_range( char* head_buf, int size,  int64_t *startpos, int64_t *endpos, int64_t *filesize)
{
	char value[64];
	if ( httpclient_get_field_value(head_buf, size, "Content-Range:", 14, value, sizeof(value)) != NULL )
	{
		int i = 0;
		while ( value[i] != '\0' && (value[i] < '0' || '9' < value[i]) )
			i++;
		if ( startpos && value[i] != '\0' )
			*startpos = str_int64( value + i );
		while ( value[i] != '\0' && value[i] != '-'  )
			i++;
		if ( value[i] == '-' )
		{
			i++;
			if ( endpos )
				*endpos = str_int64( value + i );
		}
		while ( value[i] != '\0' && value[i] != '/'  )
			i++;
		if ( value[i] == '/' )
		{
			i++;
			if ( filesize )
				*filesize = str_int64( value + i );
		}
		return true;
	}
	return false;

}

/** 
 * @brief ȡ�ø����ļ���"Content-MD5:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value md5������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ���md5У��,NULL������
 */
char* sw_httpclient_get_content_md5( char* head_buf, int size,  char *value, int valuesize)
{
	return httpclient_get_field_value(head_buf, size, "Content-MD5:", 12, value, valuesize);
}

/** 
 * @brief ȡ�ø����ļ��ı��뷽ʽ"Content-Encoding:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value encoding������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ��ڱ��룬NULL�����ڱ����ֶ�,ʹ�÷�ѹ���ļ�����
 */
char* sw_httpclient_get_content_encoding(char* head_buf, int size,  char *value, int valuesize)
{
	return httpclient_get_field_value(head_buf, size, "Content-Encoding:", 17, value, valuesize);
}

/** 
 * @brief ȡ�ø����ļ��Ĵ�������"Transfer-Encoding:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value Transfer Encoding������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ��ڴ������룬NULL������
 */
 char* sw_httpclient_get_transfer_encoding(char* head_buf, int size,  char *value, int valuesize)
{
	return httpclient_get_field_value(head_buf, size, "Transfer-Encoding:", 18, value, valuesize);
}

/** 
 * @brief ȡ�ø����ļ��Ĵ�������"ETag:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value ETag:������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ��ڴ������룬NULL������
 */
 char* sw_httpclient_get_etag(char* head_buf, int size,  char *value, int valuesize)
{
	return httpclient_get_field_value(head_buf, size, "ETag:", 5, value, valuesize);
}


/** 
 * @brief ȡ�ø����ļ���"Location:"�ض���
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value location������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ����ض���NULL�������ض���
 */
 char* sw_httpclient_get_location(char* head_buf, int size,  char *value, int valuesize)
{
	return httpclient_get_field_value(head_buf, size, "Location:", 9, value, valuesize);
}
/** 
 * @brief ȡ�ø����ļ��Ĺ�����"Set-Cookie:"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 	value ������Cookie������,���valueΪ�շ��صľ����Ҫ����ʹ�û����п��ܱ�ı�
 *	valuesize����������
 * @return �ǿմ��ڹ���Cookie��NULL������
 */
char* sw_httpclient_get_set_cookie(char* head_buf, int size,  char *value, int valuesize)
{
	return httpclient_get_field_value(head_buf, size, "Set-Cookie:", 11, value, valuesize);
}

/** 
 * @brief �жϷ������Ƿ�֧��byte-range requests"
 * 
 * @param head_buf HTTP����ͷ���ջ�����
 * @param size ��������С
 * 
 * @return true֧��byte-range requests��false��֧��
 */
bool sw_httpclient_check_accept_ranges(char* head_buf, int size)
{
	char value[32];
	if( httpclient_get_field_value(head_buf, size, "Accept-Ranges:", 14, value, sizeof(value)) != NULL && !strncasecmp(value, "bytes", 5) )
        return true;
    else
	    return false;
}

/** 
 * @brief �õ�http header�ĳ���
 * 
 * @param head_buf HTTP����ͷ����
 * @param size ��������С
 * 
 * @return 
 */
int sw_httpclient_get_header_size( char* head_buf, int size )
{
  	int i           = 0;
	int j           = 0;
	int index       = 0;    //���ӱ��
	int sentencelen = 0;    //���ӳ���
	int headerlen   = 0;    //���о��ӵ��ܳ���
	i               = 0;
	index           = 0;
	headerlen       = 0;	

	while( i < size )
	{
		j = i;
		sentencelen = 0;
		for(;i < ( size -1 );i++)
		{
			if( head_buf[i] == '\r' &&  head_buf[i + 1] == '\n' )
			{
				sentencelen = i - j + 2;			
				break;
			}
		}
		
		i += 2;

		index++;
		//printf("Find  line %d length:%d i:%d size:%d\n",index,sentencelen,i,size);
		if( sentencelen > 0 )
		{
			headerlen += sentencelen;
			if( sentencelen <= 2 )
			{
				break;
			}
		}
		else
			headerlen = size;
	}
	
	return headerlen;
}

/** 
 * @brief ע��cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param cookies Ԥ���õ�cookies��Ż�����,��ʽCookie:  
 * 
 * @return 
 */
int sw_httpclient_register_cookies( HANDLE h_http_client, char* cookies )
{
	if( h_http_client == NULL )
	{
        memset(m_sz_cookies, 0, sizeof(m_sz_cookies));
		if( cookies )
			strlcpy( m_sz_cookies, cookies, sizeof( m_sz_cookies ));
		return 0;
	}

    memset(((http_client_t*)h_http_client)->sz_cookies, 0, sizeof(((http_client_t*)h_http_client)->sz_cookies));
	if( cookies && *cookies != '\0')
	{
		((http_client_t*)h_http_client)->include_Cookie = true;
		strlcpy( ((http_client_t*)h_http_client)->sz_cookies, cookies, sizeof(((http_client_t*)h_http_client)->sz_cookies));
	}

	return 0;
}

/** 
 * @brief ���cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return
 */
void sw_httpclient_clear_cookies( HANDLE h_http_client )
{
	if( h_http_client == NULL )
	{
		memset( m_sz_cookies, 0, sizeof( m_sz_cookies ) );
	}
	else
	{
		memset( ((http_client_t*)h_http_client)->sz_cookies,0,sizeof(((http_client_t*)h_http_client)->sz_cookies));
	}
}

/** 
 * @brief ȡ��cookies
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return h_http_client cookie �洢��
 */
char* sw_httpclient_get_cookies( HANDLE h_http_client )
{
	if(h_http_client == NULL)
	{
		return m_sz_cookies;
	}
	return	((http_client_t*)h_http_client)->sz_cookies;
}

/** 
 * @brief ע��User Agent
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param useragent Ԥ���õ�User Agent��Ż�����,��ʽUser-Agent: 
 * 
 * @return 
 */
int sw_httpclient_register_useragent( HANDLE h_http_client, const char* useragent )
{
	if( h_http_client == NULL )
	{
		memset(m_user_agent, 0, sizeof(m_user_agent));
		if( useragent )
			strlcpy( m_user_agent, useragent, sizeof( m_user_agent ));
		return 0;
	}

	if( useragent && *useragent != '\0')
	{
		((http_client_t*)h_http_client)->include_User_Agent = true;
		((http_client_t*)h_http_client)->user_agent[sizeof(((http_client_t*)h_http_client)->user_agent)-1] = '\0';
		strlcpy( ((http_client_t*)h_http_client)->user_agent, useragent, sizeof(((http_client_t*)h_http_client)->user_agent));
	}

	return 0;
}

/** 
 * @brief ���User Agent
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return
 */
void sw_httpclient_clear_useragent( HANDLE h_http_client )
{
	if( h_http_client == NULL )
	{
		memset( m_user_agent, 0, sizeof( m_user_agent ) );
	}
	else
	{
		memset( ((http_client_t*)h_http_client)->user_agent,0,sizeof(((http_client_t*)h_http_client)->user_agent));
	}
}

/** 
 * @brief ȡ��User Agent
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * 
 * @return h_http_client cookie �洢��
 */
char* sw_httpclient_get_useragent( HANDLE h_http_client )
{
	if(h_http_client == NULL)
	{
		return m_user_agent;
	}
	return	((http_client_t*)h_http_client)->user_agent;
}
/** 
 * @brief ����HTTP�汾��
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param version Ԥ���õ�HTTP�汾��
 * 
 * @return 
 */
void sw_httpclient_set_version( HANDLE h_http_client, char* version )
{
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if( p_http_client && version)
	{
		strlcpy(p_http_client->version, version, sizeof(p_http_client->version));
	}
}
/** 
 * @brief ����HTTP���ӿɽ��ܵı��뷽ʽ
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param encoding Ԥ���õ�HTTP���뷽ʽ
 * 
 * @return 
 */
void sw_httclient_set_encoding(HANDLE h_http_client, char* encoding )
{
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if( p_http_client && encoding && encoding[0] != '\0')
	{
		p_http_client->include_Accept_Encoding = true;
		p_http_client->encoding[sizeof(p_http_client->encoding) - 1] = '\0';
		strlcpy(p_http_client->encoding, encoding, sizeof(p_http_client->encoding));
	}	
}

/** 
 * @brief ����HTTP���ӿɽ��ܵ�����
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param encoding Ԥ���õ�HTTP���뷽ʽ
 * 
 * @return 
 */
void sw_httclient_set_language(HANDLE h_http_client, const char* language )
{
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if( p_http_client && language && language[0] != '\0')
	{
		p_http_client->include_Accept_language = true;
		p_http_client->language[sizeof(p_http_client->language) - 1] = '\0';
		strlcpy(p_http_client->language, language, sizeof(p_http_client->language));
	}	
}

/** 
 * @brief ����HTTP���ӵ�If-None-Match
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param etag	ETag��
 * 
 * @return 
 */
void sw_httpclient_set_etag(HANDLE h_http_client, char* etag )
{
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if( p_http_client && etag && *etag != '\0')
	{
		p_http_client->etag[sizeof(p_http_client->etag)-1] = '\0';
		strlcpy(p_http_client->etag, etag, sizeof(p_http_client->etag));
	}	
}

int sw_httpclient_get_skt( HANDLE h_http_client )
{
	if( h_http_client )
		return ( (http_client_t*)h_http_client )->skt;
	return 0;
}


int sw_httpclient_get_err_code(HANDLE p_http_client)
{
	if (p_http_client)
		return ((http_client_t*)p_http_client)->error_code;
	return 0;
}


void sw_httpclient_set_err_code(HANDLE p_http_client, int code)
{
	SWCOMMON_LOG_INFO("sw_httpclient_set_err_code %d\n",code);
	if (p_http_client)
		((http_client_t *)p_http_client)->error_code =code;
}

unsigned short sw_httpclient_get_listen_port( )
{
     return m_http_listen_port;
}

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
bool sw_httpclient_get_playtime_range( char* head_buf, int size,  int64_t *start, int64_t *end, int64_t *duration)
{
	if ( httpclient_get_field_value(head_buf, size, "PlayTime-Range:", 15, m_tmp_field, sizeof(m_tmp_field)) != NULL )
	{
		int i = 0;
		while ( m_tmp_field[i] != '\0' && (m_tmp_field[i] < '0' || '9' < m_tmp_field[i]) )
			i++;
		if ( start && m_tmp_field[i] != '\0' )
			*start = str_int64( m_tmp_field + i );
		while ( m_tmp_field[i] != '\0' && m_tmp_field[i] != '-'  )
			i++;
		if ( m_tmp_field[i] == '-' )
		{
			i++;
			if ( end )
				*end = str_int64( m_tmp_field + i );
		}
		while ( m_tmp_field[i] != '\0' && m_tmp_field[i] != '/'  )
			i++;
		if ( m_tmp_field[i] == '/' )
		{
			i++;
			if ( duration )
				*duration = str_int64( m_tmp_field + i );
		}
		return true;
	}
	return false;

}
/***********************************************************************************************
* CONTENT: ����HTTP Request TTNET��Ŀ���͵��ֶβ�һ���������һ����֧
* PARAM:
	[in] skt:�������Ӻ���׽��� 
	[in] method: ����ʽ
	[in] url:�����URL
	[in] host:��������
	[in] accept_type:���յ��ļ�����
	[in] content_type:post ����������
	[in] content:post ������
	[in] length: post ���ݳ���
	[in] soap_action: soap action URI
	[in] http_auth: ժҪ��֤��Ϣ
* RETURN:
	�����Ƿ��ͳɹ�
* NOTE:
************************************************************************************************/
bool sw_httpclient_request_ex( HANDLE h_http_client, char* method, char* url, char* host,
							char *accept_type, char* content_type, char* content, int length,
							int timeout, char* soap_action, http_authentcation_t* http_auth )
{
	if (h_http_client == NULL)
		return false;
	length = (length>0) ? length : 0;
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
#ifdef SUPPORT_HTTPS
	if (p_http_client->isHttps == true)
		return sw_httpsclient_request(h_http_client, method, url, host, accept_type, content_type, content, length, timeout, soap_action, http_auth);
#endif
	fd_set wset;	
	char* p_buf = NULL;
	char* p_req_buf = NULL;
	int   reqlen = length+8*1024;
	ahash_hex ha1;
	ahash_hex ha2 = "";
	ahash_hex m_response;
	char c_nonce[48];
	char sz_nonce_count[9] = "00000001";
	int i =0;
	int ret = 0;
	int pos = 0;
	
	if( p_http_client == NULL || method == NULL || url == NULL)
		return false;

	if(strcmp( method, "GET" ) == 0)
		reqlen = 8*1024;
	
	p_req_buf = (char*)malloc(reqlen);
	if( p_req_buf==NULL )
		return false;
	p_buf =  p_req_buf;

	memset( c_nonce, 0, sizeof(c_nonce) );

	/* method */
	pos += strlcpy( &p_buf[pos], method, pos < reqlen ? reqlen - pos : 0);
	pos += strlcpy( &p_buf[pos], " ", pos < reqlen ? reqlen - pos : 0);
	
	/* url */
	pos += strlcpy( &p_buf[pos], url, pos < reqlen ? reqlen - pos : 0);//sw_url_encode(url,p_buf);
	/* �汾 */
	pos += strlcpy( &p_buf[pos], " HTTP/1.1", pos < reqlen ? reqlen - pos : 0);
	
	/* host information */
	if(host && host[0] != '\0' )
	{
		pos += snprintf(&p_buf[pos], pos < reqlen ? reqlen - pos : 0,"\r\nHost: %s", host);
	}
	
	/* �������� */
	pos += strlcpy( &p_buf[pos], "\r\nAccept: ", pos < reqlen ? reqlen - pos : 0);
	if( accept_type && accept_type[0] != '\0' )
	{
		pos += strlcpy( &p_buf[pos], accept_type, pos < reqlen ? reqlen - pos : 0 );
	}
	else
	{
		pos += strlcpy( &p_buf[pos], "*/*", pos < reqlen ? reqlen - pos : 0);
	}
	
	/* soap action */
	if ( soap_action != NULL ) 
	{
		if( soap_action[0] != '\0' )
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\nSOAPAction: %s",soap_action);
		else
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\nSOAPAction: \"\"");		
	}
	/* ���ժҪ��֤��Ϣ */
	if ( http_auth != NULL ) 
	{
		if( strcmp(http_auth->arithmetic,"Basic") ==0 )
		{
			char user_pass[128];
			char enc_user_pass[256];
			int  enc_length = 0;
			pos += strlcpy( &p_buf[pos], "\r\nAuthorization: Basic ", pos < reqlen ? reqlen - pos : 0);
			if (snprintf(user_pass, sizeof(user_pass), "%s:%s", http_auth->user_name ,http_auth->user_pwd) >= (int)sizeof(user_pass))
				SWCOMMON_LOG_DEBUG("%s auth too long", url);
			enc_length = base64encode((const unsigned char*)user_pass,strlen(user_pass),(unsigned char*)enc_user_pass,sizeof(enc_user_pass)-1);
			enc_user_pass[enc_length] = '\0';
			pos += strlcpy( &p_buf[pos], enc_user_pass, pos < reqlen ? reqlen - pos : 0 );
		}
		else
		{
			// USERNAME
			pos += strlcpy( &p_buf[pos], "\r\nAuthorization: Digest username=\"", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->user_name, pos < reqlen ? reqlen - pos : 0 );

			// REALM
			pos += strlcpy( &p_buf[pos], "\", realm=\"", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->realm, pos < reqlen ? reqlen - pos : 0 );

			// NONCE
			pos += strlcpy( &p_buf[pos], "\", nonce=\"", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->nonce, pos < reqlen ? reqlen - pos : 0 );

			// URI
			pos += strlcpy( &p_buf[pos], "\", uri=\"", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->uri, pos < reqlen ? reqlen - pos : 0 );

			// QOP
			pos += strlcpy( &p_buf[pos], "\", qop=\"", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], http_auth->qop, pos < reqlen ? reqlen - pos : 0 );

			// NC
			pos += strlcpy( &p_buf[pos], "\", nc=", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], sz_nonce_count, pos < reqlen ? reqlen - pos : 0 );

			// CNONCE
			srand( (unsigned)time( NULL ) + rand()*2 );
			snprintf( c_nonce, sizeof(c_nonce), "%d", rand() ) ;
			//snprintf( c_nonce, sizeof(c_nonce), "%s", http_auth->nonce );

			pos += strlcpy( &p_buf[pos], ", cnonce=\"", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], c_nonce, pos < reqlen ? reqlen - pos : 0 );

			// RESPONSE
			digest_calc_ha1(http_auth->arithmetic, http_auth->user_name, http_auth->realm, http_auth->user_pwd, http_auth->nonce, c_nonce, ha1);
			digest_calc_respose(ha1, http_auth->nonce, sz_nonce_count, c_nonce, http_auth->qop, method, http_auth->uri, ha2, m_response);

			pos += strlcpy( &p_buf[pos], "\", response=\"", pos < reqlen ? reqlen - pos : 0);
			pos += strlcpy( &p_buf[pos], m_response, pos < reqlen ? reqlen - pos : 0 );
		
			// OPAQUE
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\", opaque=\"%s\"", http_auth->opaque);
		}
	}
	
	if( strcmp( method, "POST" ) == 0 )
	{
		if( length > 0 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\nContent-Length: %d", length );
		}
		else
		{
			pos += strlcpy( &p_buf[pos], "\r\nContent-Length: 0", pos < reqlen ? reqlen - pos : 0 );
		}
		if( length >0 )
		{ 
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\nContent-Type: %s", content_type );
		}
	}
	else if( strcmp( method, "GET" ) == 0 )
	{
		if( length >0 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\nRange: bytes=%d-%s", length, content && strchr(content, '-')==NULL ? content : "" );
		}
		else if( content && strchr(content, '-') )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\nRange: bytes=%s", content );
		}
	}
	/* ���������Լ�����:identityȱ��Ĳ�ʹ���κ�ת��,���ʹ��gzip, deflate�Ļ��еķ������ᴫ��ѹ�����ļ� */
	//if ( p_http_client->encoding[0] == '\0' )
	//	pos += strlcpy( &p_buf[pos],"\r\nAccept-language: zh-cn\r\nAccept-Encoding: identity", pos < reqlen ? reqlen - pos : 0);//identityȱ��Ĳ�ʹ���κ�ת������֧���ļ�ѹ��gzip, deflate
	//else
	//{
	//	pos += strlcpy( &p_buf[pos],"\r\nAccept-language: zh-cn\r\nAccept-Encoding: ", pos < reqlen ? reqlen - pos : 0);
	//	pos += strlcpy( &p_buf[pos], p_http_client->encoding, pos < reqlen ? reqlen - pos : 0);
	//}
	
	if (p_http_client->etag[0] != '\0')
	{
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\nIf-None-Match: %s", p_http_client->etag);
	}

	/* user-agent */
	if ( p_http_client->include_User_Agent )
	{
		if (p_http_client->user_agent[0] == '\0' && m_user_agent[0] == '\0')
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0,"\r\nUser-Agent: Mozilla/4.0 (compatible; MS IE 6.0; (ziva))");
		else if (p_http_client->user_agent[0] == '\0' && m_user_agent[0] != '\0')
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\n%s", m_user_agent);
		else
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0, "\r\n%s", p_http_client->user_agent);
	}

	if ( p_http_client->include_Pragma )
		pos += strlcpy( &p_buf[pos],"\r\nPragma: no-cache", pos < reqlen ? reqlen - pos : 0);
	
	if ( p_http_client->include_Cache_Control )
		pos += strlcpy( &p_buf[pos],"\r\nCache-Control: no-cache", pos < reqlen ? reqlen - pos : 0);

	if( p_http_client->include_Cookie && p_http_client->sz_cookies[0] != '\0' )
	{
		pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0,"\r\n%s", p_http_client->sz_cookies );
	}
	
	if( p_http_client->extra[0] != '\0' )
	{
		if( (pos + (int)strlen( p_http_client->extra ) ) < reqlen - 64 )
		{
			pos += snprintf( &p_buf[pos], pos < reqlen ? reqlen - pos : 0,"%s", p_http_client->extra );
		}
	}

	if( strcasecmp( p_http_client->version, "1.0" ) == 0 )
		pos += strlcpy( &p_buf[pos],"\r\nConnection: close\r\n\r\n", pos < reqlen ? reqlen - pos : 0 );
	else
		pos += strlcpy( &p_buf[pos],"\r\nConnection: Keep-Alive\r\n\r\n", pos < reqlen ? reqlen - pos : 0 );

	/* ����content */
	if( strcmp( method, "POST" ) == 0 )
	{
		if (content != NULL)
		{
			memcpy( &p_buf[pos], content, (length+pos) <= reqlen ? length : 0 );
			pos += length;
		}
	}
	if (reqlen < pos)
	{
		SWCOMMON_LOG_DEBUG("%s http header too long %d", url,  pos);
		free( p_req_buf );		
		sw_httpclient_set_err_code(p_http_client,HTTP_ERROR_EXCEPTION);
		return false;
	}
	i=0;
	reqlen = pos;
	while( i < reqlen )
	{
		/* ���״̬ */
		ret = sw_tcp_select( p_http_client->skt, NULL, &wset, NULL, timeout );
		if( ret<=0 )
		{
			if(ret == 0)
				sw_httpclient_set_err_code(p_http_client,HTTP_ERROR_TIMEOUT);
			else
				sw_httpclient_set_err_code(p_http_client,HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "send errno=%d(%s), ret=%d\n", errno, strerror(errno), ret );
			break;
		}
		if(p_http_client->skt < 0|| !FD_ISSET( p_http_client->skt, &wset ) )
		{
			sw_httpclient_set_err_code(p_http_client,HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "excute errno=%d(%s)\n", errno, strerror(errno) );
			break;
		}
		/* ����Httpͷ */
		ret =  sw_tcp_send( p_http_client->skt, p_req_buf+i, reqlen-i );
		if( ret <= 0 )
		{
			sw_httpclient_set_err_code(p_http_client,HTTP_ERROR_EXCEPTION);
			SWCOMMON_LOG_DEBUG( "send errno=%d(%s), ret=%d\n", errno, strerror(errno), ret );
			break;
		}
		i += ret;
	}
	free( p_req_buf );
	return i==reqlen;
}


/** 
 * @brief ����HTTP���ӵ�˽���ֶ�(����ͨ)
 * 
 * @param h_http_client �����ӿͻ��˵ľ��
 * @param extra ˽���ֶ�
 * 
 * @return 
 */
void sw_httpclient_set_extra(HANDLE h_http_client, char* extra)
{
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if( p_http_client && extra && *extra != '\0' )
	{
		strlcpy(p_http_client->extra, extra, sizeof(p_http_client->extra));
	}	
}

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
void sw_httpclient_set_exclude_field(HANDLE h_http_client, const char* ex_field)
{
	http_client_t* p_http_client = ( http_client_t* )h_http_client; 
	if( p_http_client && ex_field && *ex_field != '\0' )
	{
		if (strcasecmp(ex_field, "Accept-language") == 0)
				p_http_client->include_Accept_language = false;
		else if (strcasecmp(ex_field, "Accept-Encoding") == 0)
				p_http_client->include_Accept_Encoding = false;
		else if (strcasecmp(ex_field, "User-Agent") == 0)
				p_http_client->include_User_Agent = false;
		else if (strcasecmp(ex_field, "Pragma") == 0)
				p_http_client->include_Pragma = false;
		else if (strcasecmp(ex_field, "Cache-Control") == 0)
				p_http_client->include_Cache_Control = false;
		else if (strcasecmp(ex_field, "Cookie") == 0)
				p_http_client->include_Cookie = false;
	}
}
