#include <stddef.h>
#include <sys/types.h>
#include "swapi.h"
#include "swmutex.h"
#include "swurl.h"
#include "swlog.h"
#include "swhttpclient.h"
#include "swhttpsclient.h"
#include "swtxtparser.h"
#include "swfile_priv.h"
#include "swcommon_priv.h"

#define MAX_PACKET_SIZE ( 1024 * 8 )
#define SWHTTPFILE_TIMEOUT_DEFAULT         5000

extern void sw_httpclient_set_exclude_field(HANDLE h_http_client, const char* ex_field);

//����֧��rrs���ּ�¼�Ĵ���rrs��ַ
typedef struct sw_rrsip{
	unsigned long	ip;	/* rrs���ֵ�ip��ַ */
	unsigned short	port;	/* rrs���ֵ�port��ַ */
	int		seq;	/* ��¼��ӽ�ȥ����� */
}sw_rrsip_t;
static sw_rrsip_t	m_bad_iplist[] = {
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}
};

static int	m_bad_ip_seq = 0;

/* ��������rrs����rrsip=ip:port */
typedef struct sw_httpfile
{
	swfile_t 	impl;
	HANDLE 	httpclient; 
	sw_url_t 	url;
	int64_t 	filesize;	//�ļ���С,��һ������Content-Length,ʵ���ļ���С
	int64_t 	curr_pos;	//�ļ�����λ��
	int64_t	start_pos;	//�����ļ����صĿ�ʼλ��
	int64_t	end_pos;	//�����ļ����صĽ���λ��,���ļ��ֶ�����������,ע��::����п�ʼ�ͽ���λ�ñ�ʾ�Ƿ�Ƭ���أ���seek����ֻ���ڱ�������seek
	int64_t	content_len;	//�����ļ����ص�http�������ݴ�С,���ڷֶ�������Ҫ������
	int 	timeout;
	bool	bchunk;	//�Ƿ���chunk����,chunk���䲻֧���ļ���λ
	int	chunksize;	//���յ�chunksizeΪ0ʱ��ʾ�յ�������,�ļ���ȡ���
	int	chunkpos;		//��ǰ��ȡ��chunkλ��������ڵ���chunksize��Ҫ���½���chunk��
	int	bchunkend;	//�Ƿ��յ�chunksizeΪ0�İ�?
	int	bfstchunk;	//�Ƿ�տ�ʼ����chunk
	
	int 	recv_error;
	char	sz_cookies[1024];	//��http�ļ���cookies,Ӧ�ò㴫������
	char	cookie[512];	//httpЭ�齻��ʱ��cookies
	char  user_agent[128]; //��http�ļ���User-Agent,Ӧ�ò㴫������
	char  sz_etag[128]; //�ϴ�http�ļ���etag,Ӧ�ò㴫������
	char  etag[128]; //httpЭ�齻��ʱ��etag
	char	encoding[32];		//���ص��ĵ�����(�еĻ�)
	char	sz_encoding[32];	//����Ľ��յ��ĵ�ѹ����ʽ,Ӧ�ò㴫������
	char	language[256];	//���յ��ĵ����Ա���,Ӧ�ò㴫������
	char	md5[32];	//���յ��ĵ�MD5(�еĻ�)	
	char	exclude_field[64];	//HTTP����ͷ���в��������ֶ�[Accept-language,Accept-Encoding,User-Agent,Pragma,Cache-Control,Cookie]���ŷָ�
	char  *server_extension;	//��������չ�ֶ�
	char  *extension;	//��չ�ֶε�һ������ʱ����
	
	char	redirect_url[2048];	//�ض���url
	sw_url_t	rediurl;		//��¼�ض����url�����Ϣ
	int 	tmp_data_size;
	
	//���������ض���������rrs��������(�ϴ�������������)
	sw_rrsip_t	rrsip[9];	//��¼rrs���ֵĽṹ��
	int	rrsipstatus[9];	//��¼��rrs���ֵ�ip��ַ״̬1:�Ѿ�ʹ�ù�,�������ô˵�ַ��������,����:δʹ��
	int	rrscnt;		//��¼rrs���ֵ�ַ����(����Ǻ���rrs����)
	int	rrspos;		//��¼rrs���ֵ�ʹ�õ�ַλ��
	
	//����http����ʱ�ĳ�ʱ����,���֧��6��
	int	connecttimes[6];
	
	char	*soap;	//��¼soap��Ϣ
	int		contentlength;	//content���ݳ����ݲ�ʹ��
	char	*content;	//content�����ݲ�ʹ��
	char	content_type[64];	//content���������ݲ�ʹ��
	char	accept[128];	//accept�ֶ�
	http_authentcation_t	*http_auth;	//HTTPժҪ��֤

	bool b_shutdown;
	uint8_t	tmp_buf[MAX_PACKET_SIZE];
	bool isHttps;   //�Ƿ���https�ļ�
} swhttpfile_t; 


#define IsHex(c)    (('0'<=(c) && (c)<='9') || ('a'<=(c) && (c)<='f') || ('A'<=(c) && (c)<='F'))
#define Char2Hex(c) ('0'<=(c) && (c)<='9' ? ((c)&0xf) : ((c)&0xf)+9)

static HANDLE sw_httpfile_connect_server(swhttpfile_t *http, unsigned long ip, unsigned short port)
{
	if ( http->b_shutdown )
		return NULL;
	int i = 0;
	HANDLE client;
	do {
		client = sw_httpclient_connect_ext(ip, port, http->connecttimes[i], http->isHttps);
		i++;
	} while ( client == NULL && i < 6 && 0 < http->connecttimes[i] && http->b_shutdown == false );
	return client;
}

static HANDLE sw_httpfile_connect_server_v6(swhttpfile_t *http, struct in6_addr ip, unsigned short port)
{
	if ( http->b_shutdown )
		return NULL;
	int i = 0;
	HANDLE client;
	do {
		client = sw_httpclient_connect_v6(ip, port, http->connecttimes[i]);
		i++;
	} while ( client == NULL && i < 6 && 0 < http->connecttimes[i] && http->b_shutdown == false );
	return client;
}

/* ���ip:port�Ƿ��Ǵ����״̬,����ڴ����з���false,��������true */
static bool rrs_check_ip_status(unsigned long ip, unsigned short port)
{
	unsigned int i = 0;
	for ( ; i < sizeof(m_bad_iplist)/sizeof(m_bad_iplist[0]) ; i++)
	{
		if ( m_bad_iplist[i].ip == ip && m_bad_iplist[i].port == port )
			return false;
	}
	return true;
}
/* ���뵽ʧ�ܵĵ�ַ���� */
static void rrs_add_bad_ip(unsigned long ip, unsigned short port)
{
	if ( !rrs_check_ip_status(ip, port) )
		return;
	unsigned int i = 0, pos;
	int seq;
	for ( ; i < sizeof(m_bad_iplist)/sizeof(m_bad_iplist[0]) ; i++)
	{
		if ( m_bad_iplist[i].ip == 0 )
		{/* �п��е�λ�� */
			m_bad_iplist[i].ip = ip;
			m_bad_iplist[i].port = port;
			m_bad_iplist[i].seq = m_bad_ip_seq++;
			return ;
		}
		
	}
	pos = 0;
	i = 0;
	seq = 0x7fffffff;
	for ( ; i < sizeof(m_bad_iplist)/sizeof(m_bad_iplist[0]) ; i++)
	{/*û�п��е�,���ҵ�seq��С��,���������linux�ȶ����е�seq�������0x7fffffff*/
		if ( m_bad_iplist[i].seq < seq )
		{
			pos = i;
			seq = m_bad_iplist[i].seq;
		}	
	}
	m_bad_iplist[pos].ip = ip;
	m_bad_iplist[pos].port = port;
	m_bad_iplist[pos].seq = m_bad_ip_seq++;	
	/* ���û�пռ�����ַ�Ļ�,�ҵ��ʱ��ûʹ�õ�ip:port */
}
/* ��ip��ַ���� */
static void rrs_remove_bad_ip(unsigned long ip, unsigned short port)
{
	unsigned int i = 0;
	for ( ; i < sizeof(m_bad_iplist)/sizeof(m_bad_iplist[0]) ; i++)
	{
		if ( m_bad_iplist[i].ip == ip && m_bad_iplist[i].port == port )
		{
			m_bad_iplist[i].ip = 0;
			m_bad_iplist[i].port = 0;
			m_bad_iplist[i].seq = 0;
			return ;
		}
	}
}
/* ����url��rrsip�����Ϣ */
static void rrs_parse_url(swhttpfile_t *http, const char *url)
{
	int rrscnt = 1;
	char *p = strstr((char*)url, "rrsip=");
	while ( p != NULL && rrscnt < (int)(sizeof(http->rrsip)/sizeof(http->rrsip[0])) )
	{
		while ( ! ( (*(p-1) == '?' || *(p-1) == ';' || *(p-1) == '&' || *(p-1) == ',' )  ) )
		{
			p = strstr(p+6, "rrsip=");
			if ( p == NULL )
				break;
		}
		if ( p != NULL )
		{/* �д˲��� */
			char szip[32];
			int i;
			p += 6;
			for( i=0; p[i]=='.' || ('0'<=p[i] && p[i]<='9'); i++ );
			if ( i < 32 )
			{
				memcpy(szip, p, (i < (int)sizeof(szip)) ? i : (int)sizeof(szip));//��Ҫ�ǹ�����
				szip[i] = '\0';
				if ( sw_txtparser_is_address(szip) )
				{
					http->rrsip[rrscnt].ip = inet_addr(szip);
					if ( p[i] == ':' )
						http->rrsip[rrscnt].port = htons(atoi((char*)(p+i+1)));
					else
						http->rrsip[rrscnt].port = htons(80);
					rrscnt++;
				}
			}
			p = strstr(p, "rrsip=");
		}
	}
	http->rrscnt = rrscnt-1;
	http->rrsip[0].ip = http->url.ip;
	http->rrsip[0].port = (http->url.port == 0) ? htons(80) : http->url.port;
	SWCOMMON_LOG_DEBUG("%s, rrscnt:%d\n", __FUNCTION__, rrscnt-1);
}
/* rrsģʽ�½���http���� */
static HANDLE rrs_http_connect(swhttpfile_t *http)
{
	HANDLE hclient = NULL;
	int pos = http->rrspos;
	int stpos = pos;
	do 
	{/* ��ѭ��������ַ */
		if ( http->b_shutdown == false && http->rrsipstatus[pos] != 1 && rrs_check_ip_status(http->rrsip[pos].ip, http->rrsip[pos].port) )
		{/* ʹ�ô˵�ַ�������� */
			hclient = sw_httpfile_connect_server(http, http->rrsip[pos].ip, http->rrsip[pos].port);
			http->rrsipstatus[pos] = 1;
			if ( hclient != NULL )
			{
				http->rrspos = pos;
				return  hclient;
			}
			if ( http->b_shutdown == false )
				rrs_add_bad_ip(http->rrsip[pos].ip, http->rrsip[pos].port);
		}
		pos = (pos+1) % (http->rrscnt +1);
	} while ( stpos != pos );
	do 
	{/* ��ѭ����������ַ */
		if ( http->b_shutdown == false && http->rrsipstatus[pos] != 1 )
		{/* ʹ�ô˵�ַ�������� */
			hclient = sw_httpfile_connect_server(http, http->rrsip[pos].ip, http->rrsip[pos].port);
			http->rrsipstatus[pos] = 1;
			if ( hclient != NULL )
			{
				http->rrspos = pos;
				rrs_remove_bad_ip(http->rrsip[pos].ip, http->rrsip[pos].port);
				return  hclient;
			}
			if ( http->b_shutdown == false )
				rrs_add_bad_ip(http->rrsip[pos].ip, http->rrsip[pos].port);
		}
		pos = (pos+1) % (http->rrscnt +1);
	} while ( stpos != pos );
	return NULL;
}
static int bcd2str( char* buf )
{
	int i, j;
	j = 0;
	for( i=0; buf[i]; i++ )
	{
		if( buf[i]=='%' && IsHex(buf[i+1]) && IsHex(buf[i+2]) )
		{
			buf[j] = (Char2Hex(buf[i+1])<<4) | Char2Hex(buf[i+2]);                                                                                                     
			i += 2;
		}
		else if( i!=j )
		{
			buf[j] = buf[i];
		}
		j++;
	}
	buf[j] = 0;
	return j;
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
static void sw_httpfile_disconnect( swhttpfile_t *http )
{
	HANDLE httpclient = http->httpclient;
	http->httpclient = NULL;
	if ( httpclient != NULL )
		sw_httpclient_disconnect( httpclient );
}

static int sw_httpfile_connect( swhttpfile_t *http, int64_t seek_pos )
{/* ��Ҫռ�ö�ջԼ1K�ռ�,�ڵ���ʱ��Ҫע���� */
	char seek_range[64];
	int recvsize,size;
	int retcode;
	int i;
	unsigned char *p;
	int redicnt = 0;
	char value[256];
	char host_buf[256];
	seek_range[0] = value[0] = http->cookie[0] = http->redirect_url[0] = '\0';
	if ( http->end_pos <= 0 )//����ֱ�Ӷ�λ���ļ�����
		snprintf( seek_range, sizeof(seek_range), "%lld-", seek_pos );
	else if ( seek_pos >= 0 ) //�������ڶ�λ
		snprintf( seek_range, sizeof(seek_range), "%lld-%lld", seek_pos, http->end_pos);
	SWCOMMON_LOG_DEBUG( "( %s,%p ) seek range:%s\n", __FUNCTION__, http, seek_range );
	http->bfstchunk = true;
	http->bchunkend = false;
	http->chunkpos = 0;
	http->chunksize = 0;
	http->recv_error = 0;
HTTPFILE_CONNET_BEG:
	http->bchunk = false;
	sw_httpfile_disconnect( http );
	if(http->url.port == 0)
	{
#ifdef SUPPORT_HTTPS
		if(http->isHttps)
			http->url.port = htons(443);
		else
#endif
			http->url.port = htons(80);
	}
	if ( redicnt == 0 && http->rrscnt > 0 )
		http->httpclient = rrs_http_connect( http );
	else
	{
		if(http->url.has_ipv6 == true)
		{
			http->httpclient = sw_httpfile_connect_server_v6( http, http->url.ipv6, http->url.port ); 
		}
		else
		{
			http->httpclient = sw_httpfile_connect_server( http, http->url.ip, http->url.port ); 
		}
	}
	if ( http->httpclient == NULL || http->b_shutdown )
	{
		sw_httpfile_disconnect( http );
		SWCOMMON_LOG_ERROR( "( %s,%p ) connect server failed!\n", __FUNCTION__, http );
		return -1; 
	}
	
	if (http->exclude_field[0] != '\0')
	{
		char exclude_field[64];
		char *pre = &exclude_field[0], *p = NULL;
		strlcpy(exclude_field, http->exclude_field, sizeof(exclude_field));
		while (pre)
		{
			p = strchr(pre, ',');
			if (p)*p++ = '\0';
			sw_httpclient_set_exclude_field(http->httpclient, pre);
			pre = p;
		}
	}
	if( http->cookie[0] != '\0' )//����ض����http������cookie����http��cookie
		sw_httpclient_register_cookies( http->httpclient, http->cookie );
	else if ( http->sz_cookies[0] != '\0' )
		sw_httpclient_register_cookies( http->httpclient, http->sz_cookies);

	if (http->user_agent[0] != '\0')
		sw_httpclient_register_useragent( http->httpclient, http->user_agent);

	if (http->sz_etag[0] != '\0')
		sw_httpclient_set_etag( http->httpclient, http->sz_etag);
		
	if (http->sz_encoding[0] != '\0')
		sw_httclient_set_encoding( http->httpclient, http->sz_encoding);

	if (http->language[0] != '\0')
		sw_httclient_set_language( http->httpclient, http->language);
		
	if (http->server_extension != NULL || http->extension != NULL)
	{
		if ( http->extension && *(http->extension) != '\0')
		{
			int len = strlen(http->server_extension) + strlen(http->extension);
			char *extension = malloc(len+4);
			if (extension)
			{
				snprintf(extension, len + 4, "%s\r\n%s", http->server_extension, http->extension);
				sw_httpclient_set_extra( http->httpclient, extension);
				free(extension);
			}
			*(http->extension) = '\0';
		}
		else if (http->server_extension != NULL)
			sw_httpclient_set_extra( http->httpclient, http->server_extension);
	}
	/* ����Get����host�����Ƿ���Ҫ���϶˿�?? */
	if(http->url.has_ipv6 == true)
		snprintf(host_buf, sizeof(host_buf), "[%s]:%d", http->url.hostname, (http->url.port == 0) ? 80:ntohs(http->url.port));
	else
		snprintf(host_buf, sizeof(host_buf), "%s:%d", http->url.hostname, (http->url.port == 0) ? 80:ntohs(http->url.port));

	if ( !sw_httpclient_request( http->httpclient, "GET", http->url.path, host_buf, 
	        http->accept, NULL, (seek_pos > 0  || http->end_pos > 0 ) ? seek_range : NULL, 0, http->timeout, http->soap, http->http_auth ) )
	{
		SWCOMMON_LOG_ERROR( "( %p ) get %s from %s failed!\n", http, http->url.path, host_buf );
		return -1;
	}
	//�����յ�http response, ����˵��������������
	size = recvsize = 0;
	while ( recvsize < MAX_PACKET_SIZE-1 && http->b_shutdown == false)
	{
		size = sw_httpclient_recv_data( http->httpclient, ( char* )http->tmp_buf+recvsize, (MAX_PACKET_SIZE-recvsize)>2048 ? 2048 : (MAX_PACKET_SIZE-recvsize-1), http->timeout ); 
		if ( size <= 0 )
			break;
		recvsize += size;
		//\r\nΪhttpЭ��ͷ�������ֶεķָ�
		http->tmp_buf[recvsize] = '\0';//���ݳ��Ȳ��ᳬ��MAX_PACKET_SIZE
		if ( strstr(( char* )http->tmp_buf+4, "\r\n\r\n") != NULL )
			break;
	}
	if ( recvsize > 0 && http->b_shutdown == false)
	{
		int i_http_header_len = sw_httpclient_get_header_size( ( char* )http->tmp_buf, recvsize );
		
		retcode=sw_httpclient_get_ret_code( ( char * )http->tmp_buf, recvsize );
		if ( i_http_header_len <= 0 || i_http_header_len > recvsize )
		{/* ͷ�򳤶Ȳ���ȷ */
			return -1;
		}
		if(retcode/100 == 3)
		{//�ض���
			i=0;
			/* ��ȡLocation�ֶ�ֵ,ֻ����ͷ���в��� */
			if( sw_httpclient_get_location( (char*)(http->tmp_buf),  i_http_header_len, http->redirect_url, sizeof(http->redirect_url) )  == NULL )
			{
				SWCOMMON_LOG_ERROR("Not found Location for %s\n", http->url.path);
				return -1;
			}

			http->cookie[0] = '\0';
			if( sw_httpclient_get_set_cookie( (char*)(http->tmp_buf), i_http_header_len, &http->cookie[7], sizeof(http->cookie)-7 ) != NULL )
				memcpy( http->cookie, "Cookie:", 7 );//cookie:1024byte

			//����http->redirect_url������BCD��ת��
			bcd2str( http->redirect_url );
			//if( !strcmpd(url, http->redirect_url) )        /* ���Է��֣�url���ض������� */

			/* ����URL */
			p = (unsigned char*)strchr(http->redirect_url, ';');
			if ( p ) 
				*p = 0;

			if ( sw_url_parse( &http->rediurl, http->redirect_url )!=0)
			{
				SWCOMMON_LOG_ERROR("%s Can not parse rediurl:%s\n", http->url.path, http->redirect_url);
				return -1;
			}

			if(memcmp(&http->rediurl,&(http->url),sizeof(http->rediurl)))
			{/* �ض��� */
				redicnt += 5;
			}
			else
				redicnt++;
			if ( redicnt >= 20 )
			{
				SWCOMMON_LOG_DEBUG("Redirect too many times for %s\n", http->url.path);
				return -1;
			}	
			memcpy(&(http->url),&http->rediurl,sizeof(sw_url_t));//ͬһ�ṹ��
			SWCOMMON_LOG_DEBUG("\nnew url:\nhead:%s\npath:%s\nport:%d\nhostname:%s\n",http->url.head,http->url.path,http->url.port,http->url.hostname);
			//����ʹ�õݹ���� �������ӣ��ݹ����������ò���̫��Ļ����е��߳��޷���������
	 		memset(&http->rediurl, 0, sizeof( sw_url_t ) );
	 		goto HTTPFILE_CONNET_BEG;
		}
		if ( retcode >= 400 || retcode <= 0)
		{/* http���� */
			http->tmp_buf[2048] = '\0';
			SWCOMMON_LOG_DEBUG("retcode:%d for %s\n%s\n", retcode, http->url.path, ( char * )http->tmp_buf);
			return -1;
		}
		sw_httpclient_get_etag( (char*)(http->tmp_buf), i_http_header_len, http->etag, sizeof(http->etag));
		sw_httpclient_get_content_md5(( char* )http->tmp_buf, i_http_header_len, http->md5, sizeof(http->md5));
		if ( sw_httpclient_get_content_encoding(( char* )http->tmp_buf, i_http_header_len, value, sizeof(value) )  )
		{/* ��ѹ�����ļ�,��swhttpfile�ﲻ֧��,Ҫ֧�ֵĻ�������� */
			SWCOMMON_LOG_DEBUG("The content encoding is:%s\n",  value);
			if ( strcasecmp(value, "zip") == 0 || strcasecmp(value, "gzip") == 0 || strcasecmp(value, "compress") == 0 || strcasecmp(value, "deflate") == 0 || strcasecmp(value, "x-compress") == 0 || strcasecmp(value, "x-gzip") == 0)
			{
				SWCOMMON_LOG_ERROR("Not support content encoding:%s\n", value);
				//return -1; 
			}
			strlcpy(http->encoding, value, sizeof(http->encoding));
		}
		if ( sw_httpclient_get_transfer_encoding(( char* )http->tmp_buf, i_http_header_len, value, sizeof(value) )  )
		{/* ��������(Transfer-Encoding),������ڴ�������Ļ��ļ�������chunkȷ��,��ʱ�ļ�������λ��ȡ */
			SWCOMMON_LOG_DEBUG("The transfer encoding is:%s,not need filesize\n",  value);
			if ( strcasecmp(value, "chunked") == 0 )
			{/*����������ֵ�Ƿǡ���dentity���������ֵ����ô���䳤�ȣ�transfer-length�������顱��chunked��������붨��*/
				http->bchunk = true;
			}
			else if ( strcasecmp(value, "dentity") == 0 )
			{/* ѹ���Ĵ�����httpfile�ﲻ֧�� */
				SWCOMMON_LOG_ERROR("Not support transfer encoding:%s for %s\n", value, http->url.path);
				return -1; 
			}
		}
		int64_t  contentsize = 0, startrange = 0, endrange = 0, filesize = 0; 
		bool bcontentlen = false;
		http->content_len = 0;
		if ( !http->bchunk && (bcontentlen=sw_httpclient_check_content_size(( char* )http->tmp_buf, i_http_header_len,  &contentsize)) )
		{/*��chunk�����Content-Length����http�����ʵ�峤��, ��identity�Ĵ�����룬���ݳ��ȣ�Content-Length��ͷ����뱻����*/
			http->filesize = http->content_len = contentsize;
			if ( retcode == 206 )
				http->filesize = seek_pos + contentsize;
		}
		else if ( !http->bchunk )
		{
			SWCOMMON_LOG_DEBUG("Not exist Content-Length\n");
		}
		if ( !http->bchunk && sw_httpclient_get_content_range(( char* )http->tmp_buf, i_http_header_len, &startrange, &endrange, &filesize) )
		{/* ��chunk�������Ҫ�ж�Content-Range����� */
			SWCOMMON_LOG_DEBUG("Content-Range:%lld-%lld/%lld\n", startrange, endrange, filesize);
			if ( bcontentlen )
			{/*endrange-startrange+1�������Content-Length*/
				if ( endrange != 0 && retcode == 206 && (endrange - startrange + 1 != contentsize ) )
				{
					SWCOMMON_LOG_ERROR("retcode is 206, %s ret Content-Length not enq Content-Range\n", http->url.path);
					return -1;
				}
			}
			else
			{
				http->content_len = endrange <= 0 ? 0 : (endrange - startrange + 1);
			}
			if ( filesize > 0 && filesize >= endrange )
				http->filesize = filesize;
			else if ( endrange > 0 )
				http->filesize = endrange + 1;
		}
		if ( recvsize > i_http_header_len )
		{
			memmove( http->tmp_buf, http->tmp_buf + i_http_header_len, recvsize - i_http_header_len );
			http->tmp_data_size = recvsize - i_http_header_len;
		}
		else
		{
			http->tmp_data_size = 0;
		}
	}
	else
		return -1;
    return 0;
}

static inline int64_t strhex_int64(char *str)
{
	int64_t id= 0;
	while (*str == ' ' || *str == '\t' )
		str++;
	while ( ('0' <= *str && *str <= '9') || ('a' <= *str && *str <= 'f') || ('A' <= *str && *str <= 'F') ) 
	{
		if ( '0' <= *str && *str <= '9' )
			id = id * 16 + (*str - '0');
		else if ( 'a' <= *str && *str <= 'f' )
			id = id * 16 + (*str - 'a' + 10);
		else
			id = id * 16 + (*str - 'A' + 10);
		str++;
	}
	return id;
}

/* ��ȡ�ļ�����,����chunk��ȡ,���������chunk�����Ϣ,������,ֻ���Զ�ȡ */
static int httpfile_read_file_data( swhttpfile_t *http, char *buf, int size, int retrycount)
{
	int cpos = 0;
	int rlen = 0;
	int trycont = 0;
	if ( http->tmp_data_size > 0 )
	{
		if ( size >  http->tmp_data_size )
		{
			memcpy(buf, http->tmp_buf, http->tmp_data_size);//����ǰ���Ѿ���ȫ�ж���
			cpos = http->tmp_data_size;
			http->tmp_data_size = 0;
		}
		else
		{
			memcpy(buf, http->tmp_buf, size);
			cpos = size;
			http->tmp_data_size -= size;
			if ( http->tmp_data_size > 0 )
				memmove(http->tmp_buf, (char*)(http->tmp_buf)+size, http->tmp_data_size);
		}
	}
	while (cpos < size)
	{
		rlen = sw_httpclient_recv_data(http->httpclient, buf+cpos, size - cpos, http->timeout);
		if ( rlen > 0 )
		{
			cpos += rlen;
			trycont = 0;
		}
		else
		{
			if ( trycont++ > retrycount)
				break;
		}
	}
	return cpos;
}
/* chunk���뷽ʽ���ļ���ȡ */
static int64_t httpfile_chunk_read( swhttpfile_t *http, void *buf, int size )
{/*chunk��= chunk-size [ chunk-extension ] \r\nchunk-data\r\n
	chunk-size = 1* HEX
	last-chunk�� = 0 [ chunk-extension ] \r\n
	������ʱ��Ҫȷ��������chunk-data����
*/
	if ( http->bchunkend || http->recv_error == 1)	//�Ѿ��յ�chunkΪ0�Ŀ���,���߳�ʱ��
		return 0;
	char *p_buf = buf;
	int64_t curr_read_size = 0;
CHUNK_READ_BEG:
	if ( http->chunksize <= http->chunkpos)
	{/* �����ݺͽ���chunk����,������Ҫ�յ�\r\n��ȷ��chunksize */
		int i = 0;
		bool chunkhead = false;	
		if ( http->bfstchunk == false )
		{//������ǵ�һ��chunk��Ҫȥ��ǰ���\r\n�ֽ�
			if ( http->tmp_data_size >= 2 )
			{/* ֱ��ȥ��ǰ2���ַ� */
				http->tmp_data_size = http->tmp_data_size - 2;
				if ( http->tmp_data_size > 0 )
					memmove(http->tmp_buf, &http->tmp_buf[2], http->tmp_data_size);
			}
			else
			{//��ȡʣ���\r\n
				int trycont = 0;
				i = 2-http->tmp_data_size;
				while (  trycont < 10 )
				{
					int j =  sw_httpclient_recv_data(http->httpclient, (char*)http->tmp_buf, i, http->timeout );
					if ( j >= i )
						break;
					if ( j >  0 )
					{
						i--;
						trycont = 0;
					}
					else
						trycont++;
				}
				if ( trycont >= 10 )
				{
					SWCOMMON_LOG_ERROR("Can not receive chunk header\n");
					http->recv_error = 1;
					return curr_read_size;
				}
				http->tmp_data_size = 0;
			}
		}
		i = 0;
		http->bfstchunk = false;	
		while ( i <  http->tmp_data_size-1)
		{/*ȥ��������,����ǵ�һ�ξ�\r\n,������Ҫ\r\n�ڽ���chunkͷ*/
			if ( http->tmp_buf[i] == '\r' && http->tmp_buf[i+1] == '\n' )//�յ�chunkͷ��Ϣ
			{
				chunkhead = true;
				break;
			}
			i++;
		}
		if ( chunkhead == false )
		{/*��������,���û����Ҫ�ŵ�*/
			int trycont = 0;
			while ( http->tmp_data_size <  MAX_PACKET_SIZE)
			{
				trycont = 0;
				while ( sw_httpclient_recv_data( http->httpclient, (char*)http->tmp_buf+http->tmp_data_size, 1, http->timeout ) != 1 && trycont < 10)
					trycont++;
				if ( trycont >= 10 )
				{
					SWCOMMON_LOG_ERROR("Wait too long time:%d ms\n", http->timeout*trycont);
					http->recv_error = 1;
					return curr_read_size;
				}
				http->tmp_data_size++;
				if (  http->tmp_data_size >= 2 )
				{
					i = http->tmp_data_size-2;
					if ( http->tmp_buf[i] == '\r' && http->tmp_buf[i+1] == '\n' )
					{
						chunkhead = true;
						break;
					}
				}
				
			}
			if ( chunkhead == false )//����
			{
				SWCOMMON_LOG_ERROR("Can not receive chunk header\n");
				http->recv_error = 1;
				return curr_read_size;
			}
			/* ��һλ���ǿ�ʼ�����ݴ�Сhex */
			http->chunksize = strhex_int64( (char*)http->tmp_buf );
			http->tmp_data_size = 0;
		}
		else
		{/* ��tmp_buf����������chunkͷ��ʾ */
			http->chunksize = strhex_int64(  (char*)http->tmp_buf );
			/* �ҵ���һ��\r\nλ���ƶ�����,iָʾ��\r\n��λ�� */
			i += 2;
			http->tmp_data_size -= i;
			if ( http->tmp_data_size > 0 )
				memmove(http->tmp_buf, ((char*)http->tmp_buf) + i, http->tmp_data_size);
		}
		if ( http->chunksize == 0 )
		{/* �Ѿ���ȡ�����ݽ�����ʾ,�˴�Ӧ�û����жϺ����Ƿ���\r\n(�ݲ��ж���) */
			http->bchunkend = true;
			SWCOMMON_LOG_ERROR("Parse Chunk file end\n");
			return curr_read_size;
		}
		http->chunkpos = 0;		
	}
	int nsize = (size - curr_read_size) > ( http->chunksize - http->chunkpos ) ? ( http->chunksize - http->chunkpos ) : (size - curr_read_size);
	int rlen = httpfile_read_file_data(http, p_buf+curr_read_size, nsize, 3);
	curr_read_size += rlen;
	http->chunkpos += rlen;
	if ( rlen != nsize )
	{/* ���� */
		http->recv_error = 1;
		return curr_read_size;
	}
	else
		http->recv_error = 0;
	//δ����,����һ��chunk�������Ѿ�������
	if ( (int64_t)size > curr_read_size )
		goto CHUNK_READ_BEG;
	return curr_read_size;	
}
/* �����ļ���ʽ��ȡ */
static int64_t httpfile_whole_read( swhttpfile_t *http, void *buf, int size )
{
	int64_t nsize = (int64_t)size;
	int64_t total_read_size = 0;

	if ( http->filesize > 0 && http->curr_pos >= http->filesize )
	{
		SWCOMMON_LOG_ERROR( "( %s,%p ) EOF reached...\n", __FUNCTION__,  http );
		return 0;
	}
	if ( http->filesize > 0 && http->curr_pos + nsize > http->filesize )
	{
		SWCOMMON_LOG_INFO( "( %s,%p ) %lld + %lld = %lld > %lld\n", 
					__FUNCTION__, http, http->curr_pos, nsize, http->curr_pos + nsize, http->filesize );
		nsize = http->filesize - http->curr_pos;
		SWCOMMON_LOG_INFO( "( %s,%p ) size:%d, new size: %lld\n", __FUNCTION__, http, size, nsize );
	}
	total_read_size = httpfile_read_file_data(http, (char*)buf, nsize, http->filesize > 0 ? 0 : 3 );
	if ( total_read_size != nsize )
	{
		SWCOMMON_LOG_ERROR( "( %s,%p ) receive ERROR and file size is (%lld), set to EOF\n", __FUNCTION__,
					http, http->filesize);
		http->recv_error = 1;
	}
	else
		http->recv_error = 0;
	return total_read_size;
}
/* �ֶ��ļ���ʽ��ȡ,�˲�֧�ֶ���ֶ� */
static int64_t httpfile_sect_read( swhttpfile_t *http, void *buf, int size )
{
	int64_t nsize = (int64_t)size;
	int64_t total_read_size = 0;

	if ( http->curr_pos >= http->end_pos)
	{
		SWCOMMON_LOG_ERROR( "( %s,%p ) EOF reached...\n", __FUNCTION__,  http );
		return 0;
	}
	if ( http->curr_pos + nsize > http->end_pos )
	{
		SWCOMMON_LOG_INFO( "( %s,%p ) %lld + %lld = %lld > %lld\n", 
					__FUNCTION__, http, http->curr_pos, nsize, http->curr_pos + nsize, http->end_pos );
		nsize = http->end_pos - http->curr_pos;
		SWCOMMON_LOG_INFO( "( %s,%p ) size:%d, new size: %lld\n", __FUNCTION__, http, size, nsize );
	}
	total_read_size = httpfile_read_file_data(http, (char*)buf, (int)nsize, 0 );
	if ( total_read_size != nsize )
	{
		SWCOMMON_LOG_ERROR( "( %s,%p ) receive ERROR and file size is (%lld), set to EOF\n", __FUNCTION__,
					http, http->filesize);
		http->recv_error = 1;
	}
	else
		http->recv_error = 0;
	return total_read_size;
}
static int sw_httpfile_read( swfile_t *file, void *buf, int size )
{
	if( buf == NULL || size < 0 )
		return -1;
	if ( size == 0 )
		return 0;
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	if ( http->httpclient == NULL )//���http�Ѿ��Ͽ�����??
	{
		SWCOMMON_LOG_ERROR("The http:%p connect has close\n", http);
		return 0;
	}
	int total_read_size = 0;
	//SWCOMMON_LOG_DEBUG( "( %s,%p ) readsize:%d, curr:%lld\n", __FUNCTION__, http, size, http->curr_pos );

	if ( http->bchunk )
	{/* ��Ƭ���������,�������3������Ͽ��˾ͶϿ�,(��û�յ�chunk��ʾʱ����10��) */
		total_read_size = (int)httpfile_chunk_read(http, buf, size);
	}
	else if ( http->end_pos <= 0 )
	{/* ���Զ�λ��ȫ�ļ���ȡ */
		total_read_size = (int)httpfile_whole_read(http, buf, size);
	}
	else
	{/* �ļ��ֶζ�ȡ,����http����ֻ��������һ��������, */
		total_read_size = (int)httpfile_sect_read(http, buf, size);
	}
	if ( size > total_read_size )
		SWCOMMON_LOG_ERROR( "( %s,%p ) read file failed! require:%d, real:%d\n", 
				__FUNCTION__, http, size, total_read_size );
	if ( total_read_size > 0 )
		http->curr_pos += total_read_size;
	
	//ע�����Ҫ�������ӷ�������Ҫ�ڴ˴�����,�����¶�����(����ڶ�����ʱ��������,�����ӵĻ��������ݲ��ô���)
	//�ж���chunk����Ķ�ȡҲ���ô���
	return total_read_size;
}


static int sw_httpfile_write( swfile_t *file, void *buf, int size )
{
	SW_UNUSED(file);
	SW_UNUSED(buf);
	SW_UNUSED(size);
	return 0;
}


static int sw_httpfile_seek( swfile_t *file, int64_t offset, int origin )
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	int64_t pos;
	
	SWCOMMON_LOG_DEBUG( "( %s,%p ) offset:%lld, origin:%d, filesize:%lld, curr:%lld\n",
				__FUNCTION__, http, offset, origin, http->filesize, http->curr_pos );
	if ( http->bchunk )
	{
		SWCOMMON_LOG_ERROR("( %s,%p ) is chunk transfer not all seek\n", __FUNCTION__, http);
		return -1;
	}
	if (http->filesize <= 0)
	{
		SWCOMMON_LOG_ERROR( "( %s,%p ) filesize is :%lld, not allow seek\n", __FUNCTION__, http, http->filesize );
		return -1;
	}

	if ( origin == SEEK_SET )
		pos = offset;
	else if ( origin == SEEK_CUR )
		pos = http->curr_pos + offset;
	else if ( origin == SEEK_END )
		pos = http->filesize + offset;
	else
	{
		SWCOMMON_LOG_ERROR( "( %s,%p ) invalid origin:%d\n", __FUNCTION__, http, origin );
		return -1;
	}

	if ( pos < 0 )
	{
		pos = 0;
		SWCOMMON_LOG_DEBUG( "( %s,%p ) invalid seek pos:%lld\n", __FUNCTION__, http, pos );
	}
	//ATTENTION �˴�����seek���ļ�ĩβ��
	if ( pos > http->filesize )
	{
		SWCOMMON_LOG_ERROR( "( %s,%p ) seek pos ( %lld ) exceed file size:%lldn",
					__FUNCTION__, http, pos, http->filesize );
		http->curr_pos = pos;
		sw_httpfile_disconnect( http );
		return 0;
	}
	if ( pos == http->filesize )
	{
		sw_httpfile_disconnect( http );
		http->curr_pos = http->filesize;
		return 0;
	}
	if ( http->end_pos > 0 && ( pos < http->start_pos || http->end_pos <= pos ) )
	{
		sw_httpfile_disconnect( http );
		http->curr_pos = pos;
		return 0;
	}
	if ( sw_httpfile_connect( http, pos ) != 0 )
	{
		return -1;
	}

	SWCOMMON_LOG_DEBUG( "( %s,%p ) new curr pos:%lld\n", __FUNCTION__, http, pos );
	
	http->curr_pos = pos;
    	return 0;
}


static int64_t sw_httpfile_tell( swfile_t *file )
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	//SWCOMMON_LOG_DEBUG( "( %s,%p ) %lld\n", __FUNCTION__, http, http->curr_pos );
	return http->curr_pos;
}


static int sw_httpfile_eof( swfile_t *file )
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	if (http->recv_error == 1)
		return 1;
	if ( http->curr_pos == http->filesize && http->filesize != 0 )
		return 1;
	if ( http->bchunk && http->bchunkend )
		return 1;
	if ( http->end_pos > 0 && http->curr_pos == http->end_pos )
		return 1;
	return 0;
}


static int sw_httpfile_getc( swfile_t *file )
{
	char c = 0;
	if ( sw_httpfile_read( file, &c, 1 ) == 1 )
		return c;
	return EOF;
}


static int sw_httpfile_is_seekable( swfile_t *file )
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	if (http->filesize <= 0)
		return -1;
	if ( http->bchunk )
		return -1;
	return 0;
}


static int64_t sw_httpfile_get_size( swfile_t *file )
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
    if(http->filesize == 0 && http->bchunk == true )
        return HTTP_CHUNK_SIZE;
    else 
	    return http->filesize;
}


static int sw_httpfile_close( swfile_t *file )
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	SWCOMMON_LOG_INFO( "( %s ) %p\n", __FUNCTION__, file );
	sw_httpfile_disconnect( http );
	if ( http->soap != NULL )
		free( http->soap );
	if ( http->http_auth != NULL )
		free( http->http_auth );
	if ( http->content != NULL )
		free( http->content );
	if ( http->server_extension != NULL)
		free( http->server_extension );
	if ( http->extension != NULL)
		free( http->extension );
	free( http );
	return 0;
}

static int sw_httpfile_shutdown( swfile_t *file )
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	http->b_shutdown = true;
	sw_httpclient_close(http->httpclient);
	//sw_httpclient_shutdown(http->httpclient);
	return 0;
}

static int sw_httpfile_ioctl( swfile_t *file, int cmd, va_list ap)
{
	swhttpfile_t *http = C2P( file, impl, swhttpfile_t );
	va_list args;
	args = ap;
	int ret = -1;
	
#define	UC(b)	(((int)b)&0xff)
	switch ( cmd )
	{
	case SW_FILE_IOCTL_SET_TIMEOUT:
	{
		int    timeout = va_arg(args, int);
		if ( timeout > 0 )
		{
			http->timeout = timeout;
			ret = 0;
		}
		else
			ret = -1;
		break;
	}
	case SW_FILE_IOCTL_GET_COOKIE:
	{
		char *buf = va_arg(args, char* );
		int    size = va_arg(args, int );
		if ( buf == NULL || size <= 0)
		{
			ret = -1;
			break;
		}
		strlcpy(buf, (http->cookie[0] != '\0') ? http->cookie : http->sz_cookies , size);
		ret = 0;
		break;
	}
	case SW_FILE_IOCTL_GET_ETAG:
	{
		char *buf = va_arg(args, char* );
		int    size = va_arg(args, int );
		if ( buf == NULL || size <= 0)
		{
			ret = -1;
			break;
		}
		strlcpy(buf, (http->etag[0] != '\0') ? http->etag : http->sz_etag , size);
		ret = 0;
		break;
	}
	case SW_FILE_IOCTL_GET_CONTENT_ENCODING:
	{
		char *buf = va_arg(args, char* );
		int    size = va_arg(args, int );
		if ( buf == NULL || size <= 0)
		{
			ret = -1;
			break;
		}
		strlcpy(buf, http->encoding, size);
		ret = 0;
		break;
	}
	case SW_FILE_IOCTL_GET_CONTENT_MD5:
	{
		char *buf = va_arg(args, char* );
		int    size = va_arg(args, int );
		if ( buf == NULL || size <= 0)
		{
			ret = -1;
			break;
		}
		strlcpy(buf, http->md5, size);
		ret = 0;
		break;
	}
	case SW_FILE_IOCTL_GET_FPATH:
	{
		char *buf = va_arg(args, char* );
		int    size = va_arg(args, int );
		if ( buf == NULL || size <= 0)
		{
			ret = -1;
			break;
		}
		ret = 0;
		if ( http->redirect_url[0] != '\0' )
		{
			strlcpy(buf, http->redirect_url, size);
		}
		else if ( http->rrscnt > 0  && http->rrspos > 0)
		{/* ���ҵ�ʵ�����ӵĵ�ַrrsposָ����ʹ�õ����Ǹ�http���� */
			char *p = (char*)&(http->rrsip[http->rrspos].ip);
			short port = ntohs(http->rrsip[http->rrspos].port);
			snprintf(buf, size, "http://%u.%u.%u.%u:%d%s", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]), ((int)port)&0xffff, http->url.path);
		}
		else
		{
			short port = ntohs(http->url.port);
			if ( port != 0 )
				snprintf(buf, size, "http://%s:%d%s", http->url.hostname, ((int)port)&0xffff, http->url.path);
			else
				snprintf(buf, size, "http://%s%s", http->url.hostname,  http->url.path);
		}
		break;
	}
	case SW_FILE_IOCTL_GET_CLIENT:
	{
		HANDLE *client = va_arg(args, HANDLE * );
		if ( client != NULL )
			*client = http->httpclient;
		break;
	}
	}
	va_end(args);
	return ret;
}
/* �����������Ϣ*/
static void sw_httpfile_parse_extension(swhttpfile_t *http, char *extension)
{
	if ( extension == NULL || *extension == '\0' )
		return ;
	char *p = extension;
	while ( *p != '\0' )
	{/* һ�еĿ�ʼλ�� */
		if ( strncasecmp(p, "cookies=", 8) == 0 )
		{
			p += 8;
			p += strlcpy(http->sz_cookies, p, sizeof(http->sz_cookies));
		}
		else if ( strncasecmp(p, "connecttimes=", 13) == 0 )
		{
			p += 13;//13=strlen("connecttimes=");
			sscanf(p, "%d,%d,%d,%d,%d,%d", &http->connecttimes[0], &http->connecttimes[1], &http->connecttimes[2], 
					&http->connecttimes[3], &http->connecttimes[4], &http->connecttimes[5]);
			p += strlen(p);
		}
		else if ( strncasecmp(p, "SOAPAction=", 11) == 0 )
		{
			p += 11;
			int len = strlen(p);
			if ( http->soap != NULL )
				free(http->soap);
			http->soap = strdup(p);
			p += len;
		}
		else if ( strncasecmp(p, "ContentType=", 12) == 0 )
		{
			p += 12;
			p += strlen(p);
		}
		else if ( strncasecmp(p, "ContentLength=", 14) == 0 )
		{
			p += 14;
			p += strlen(p);
		}
		else if ( strncasecmp(p, "Content=", 8) == 0 )
		{
			p += 8;
			p += strlen(p);
		}
		else if ( strncasecmp(p, "Accept=", 7) == 0 )
		{
			p += 14;
			p += strlcpy(http->accept, p, sizeof(http->accept));
		}
		else if ( strncasecmp(p, "authorization=", 14) == 0 )
		{
			p += 14;
			if ( http->http_auth == NULL )
				http->http_auth = (http_authentcation_t*)malloc(sizeof(http_authentcation_t));
			if ( http->http_auth )
				p += strlcpy(http->http_auth->arithmetic, p, sizeof(http->http_auth->arithmetic));
			else
				p += strlen(p);
		}
		else if ( strncasecmp(p, "authenname=", 11) == 0 )
		{
			p += 11;
			if ( http->http_auth == NULL )
				http->http_auth = (http_authentcation_t*)malloc(sizeof(http_authentcation_t));
			if ( http->http_auth )
				p += strlcpy(http->http_auth->user_name, p, sizeof(http->http_auth->user_name));
			else
				p += strlen(p);
		}
		else if ( strncasecmp(p, "authenpwd=", 10) == 0 )
		{
			p += 10;
			if ( http->http_auth )
				p += strlcpy(http->http_auth->user_pwd, p, sizeof(http->http_auth->user_pwd));
			else
				p += strlen(p);
		}
		else if ( strncasecmp(p, "authenrealname=", 15) == 0 )
		{
			p += 15;
			if ( http->http_auth )
				p += strlcpy(http->http_auth->realm, p, sizeof(http->http_auth->realm));
			else
				p += strlen(p);			
		}
		else if ( strncasecmp(p, "authennonce=", 12) == 0 )
		{
			p += 12;
			if ( http->http_auth )
				p += strlcpy(http->http_auth->nonce, p, sizeof(http->http_auth->nonce));
			else
				p += strlen(p);	
		}
		else if ( strncasecmp(p, "authenuri=", 10) == 0 )
		{
			p += 10;
			if ( http->http_auth )
				p += strlcpy(http->http_auth->uri, p, sizeof(http->http_auth->uri));
			else
				p += strlen(p);	
		}
		else if ( strncasecmp(p, "authenqop=", 10) == 0 )
		{
			p += 10;
			if ( http->http_auth )
				p += strlcpy(http->http_auth->qop, p, sizeof(http->http_auth->qop));
			else
				p += strlen(p);
		}
		else if ( strncasecmp(p, "authenopaque=", 13) == 0 )
		{
			p += 13;
			if ( http->http_auth )
				p += strlcpy(http->http_auth->opaque, p, sizeof(http->http_auth->opaque));
			else
				p += strlen(p);
		}
		else if ( strncasecmp(p, "openpos=", 8) == 0 )
		{
			p += 8;
			http->start_pos = str_int64(p);
			http->start_pos = (http->start_pos > 0) ? http->start_pos : 0;
			p += strlen(p);
		}
		else if ( strncasecmp(p, "endpos=", 7) == 0 )
		{
			p += 7;
			http->end_pos = str_int64(p);
			p += strlen(p);
		}
		else if ( strncasecmp(p, "etag=", 5) == 0 )
		{
			p += 5;
			p += strlcpy(http->sz_etag, p, sizeof(http->sz_etag));
		}
		else if ( strncasecmp(p, "user-agent=", 11) == 0 )
		{
			p += 11;
			p += strlcpy(http->user_agent, p, sizeof(http->user_agent));
		}
		else if ( strncasecmp(p, "extension=", 10) == 0 )
		{
			p += 10;
			http->extension = strdup(p);
			p += strlen(p);
		}
		else if ( strncasecmp(p, "server-extension=", 17) == 0 )
		{
			p += 17;
			http->server_extension = strdup(p);
			p += strlen(p);
		}
		else if ( strncasecmp(p, "encoding=", 9) == 0 )
		{
			p += 9;
			p += strlcpy(http->sz_encoding, p, sizeof(http->sz_encoding));
		}
		else if ( strncasecmp(p, "language=", 9) == 0 )
		{
			p += 9;
			p += strlcpy(http->language, p, sizeof(http->language));
		}
		else if ( strncasecmp(p, "exclude-field=", 14) == 0 )
		{
			p += 14;
			p += strlcpy(http->exclude_field, p, sizeof(http->exclude_field));
		}
		else
			break;
		p++;
	}
	SWCOMMON_LOG_DEBUG("Cookies:%s\nconnect:%d,%d,%d,%d,%d,%d\n", !SENSITIVE_PRINT ? "..." : http->sz_cookies, http->connecttimes[0], 
			http->connecttimes[1], http->connecttimes[2], http->connecttimes[3], http->connecttimes[4], http->connecttimes[5]);
}

static swfile_t* httpfile_open( const char *name, const char *mode, int timeout, char *extension )
{
	if (mode != NULL && strchr(mode, 'w') != NULL)
	{
		SWCOMMON_LOG_ERROR( "http file write not support yet! mode:%s\n", mode );
		return NULL;
	}
	swhttpfile_t *http = NULL;
	int64_t openpos = 0;
	char value[32];
	http = ( swhttpfile_t * )malloc( sizeof( swhttpfile_t ) );
	if ( http == NULL )
		goto FAIL;
	memset( http, 0, sizeof( swhttpfile_t ) );
#ifdef SUPPORT_HTTPS
	if (strncasecmp(name, "https://", 8) == 0)
		http->isHttps = true;
#endif
	
	SWCOMMON_LOG_INFO( "( %s ) %p\n", __FUNCTION__, http );
	value[0] = '\0';
	sw_url_get_param_value((char*)name, "openpos", value, sizeof( value));
	if (value[0] != '\0')
		openpos = str_int64(value);
	value[0] = '\0';
	sw_url_get_param_value((char*)name, "endpos", value, sizeof( value));
	if (value[0] != '\0')
		http->end_pos = str_int64(value);
	else
		http->end_pos = -1;
	//�˴���endpos,openpos�Ƿ����ȡ��??	
	openpos = openpos >= 0 ? openpos : 0;	
	http->start_pos = openpos;
	if ( sw_url_parse( &(http->url), (char*)name ) != 0 )
		goto FAIL;
	
	http->connecttimes[0] = http->timeout = timeout;
	/* �����������Ϣ*/
	sw_httpfile_parse_extension(http, extension);

	//����rrsip��ַ��Ϣ
	rrs_parse_url(http, name);
	
	if ( sw_httpfile_connect( http, http->start_pos ) != 0 )
		goto FAIL;
	http->curr_pos = http->start_pos;

	http->impl.read  = sw_httpfile_read;
	http->impl.write = sw_httpfile_write;
	http->impl.seek  = sw_httpfile_seek;
	http->impl.tell  = sw_httpfile_tell;
	http->impl.eof   = sw_httpfile_eof;
	http->impl.agetc = sw_httpfile_getc;
	http->impl.is_seekable = sw_httpfile_is_seekable;
	http->impl.get_size    = sw_httpfile_get_size;
	http->impl.close       = sw_httpfile_close;
	http->impl.ioctl	= sw_httpfile_ioctl;
	http->impl.shutdown = sw_httpfile_shutdown;
	return &http->impl;
	
FAIL:
	if ( http != NULL )
	{
		sw_httpfile_disconnect( http );
		if ( http->soap != NULL )
			free( http->soap );
		if ( http->http_auth != NULL )
			free( http->http_auth );
		if ( http->content != NULL )
			free( http->content );
		free( http );
	}
	return NULL;
}
swfile_t* sw_httpfile_open( const char *name, const char *mode )
{
	return httpfile_open( name, mode, SWHTTPFILE_TIMEOUT_DEFAULT, NULL );
}
swfile_t* sw_httpfile_open_ex( const char *name, const char *mode, int timeout, char *extension )
{
	return httpfile_open(name, mode, timeout, extension);
}
