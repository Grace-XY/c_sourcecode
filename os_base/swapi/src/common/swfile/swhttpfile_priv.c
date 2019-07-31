#include "swapi.h"
#include "swhttpclient.h"
#include "swhttpfile_priv.h"
#include "swmem.h"
#include "swthrd.h"
#include "swurl.h"
#include "swtxtparser.h"
#include "swlog.h"
#include "swbase.h"
#include "swmutex.h"
#include "swcommon_priv.h"

#define MAX_HTTFILE_NUM		16

/* ��HTTPͷ�л�ȡ����ֵ */
static int sw_httpfile_priv_find_ret_code( char* p_buf, int size );
static int bcd2str( char* buf );

typedef struct
{
	HANDLE h_client; 
	/* �ļ���С */
	int64_t filesize;
	/* ���ջ����� */
	unsigned char buf[2048];
	/* �ض���URL */
	char redirect_url[2048];
	/* ռ�ñ�־ */
	int  used;
}http_file_t;

static http_file_t m_all[MAX_HTTFILE_NUM];
static int m_ref = -1;
static HANDLE m_mutex = NULL;

//����֧��rrs���ּ�¼�Ĵ���rrs��ַ
#define MAX_BADLIST_CNT 64
static unsigned long m_bad_iplist[MAX_BADLIST_CNT];
static int m_bad_ip_pos = 0;
static int find_bad_ip(unsigned long ip)
{
	int i;
	for( i=MAX_BADLIST_CNT-1; i>=0 && m_bad_iplist[i]!=ip; i-- );
	return i;
}
static int add_bad_ip(unsigned long ip)
{
	int i = find_bad_ip(ip);
	if( i<0 )
	{
		for( i=MAX_BADLIST_CNT-1; i>=0 && m_bad_iplist[i]; i-- );
		if( i<0 )
		{
			m_bad_ip_pos = (m_bad_ip_pos+1)%MAX_BADLIST_CNT;
			i = m_bad_ip_pos;
		}
		m_bad_iplist[i] = ip;
	}
	return i;
}

static int64_t str_int64( char* str )
{
	int64_t id = 0;
	while(*str==' ' || *str=='\t')
		str++;
	while( '0'<=*str && *str<='9' )
	{
		id = id*10 + (*str-'0');
		str++;
	}
	return id;
}

/* ��ȡ�ֶ�ֵ */
static char *xstrgetval( char *buf, char *name, char *value, int valuelen )
{
	char *s, *e, *p;
	int len = strlen(buf );

	memset( value, 0, valuelen );

	/* �����汾��Ϣ�ļ� */
	p = s = buf;
	while( p < buf+len )
	{
		/* ȥ����ʼ�Ŀո� */
		while( *p == ' ' ||  *p == ',' ||  *p == '\t' || *p == '\r' || *p == '\n' )
			p++;

		/* ��¼�п�ʼ */
		s = p;

		/* �ҵ��н��� */
		while( *p != '\r' && *p != '\n' && *p != '\0' )
			p++;
		e = p;
		/* �ҵ���Ҫ���ֶ� */
		if( strncasecmp( s, name, strlen(name) ) == 0 )
		{
			/* �ҵ����ƽ��� */
			p = s;
			while( *p != ':' &&  *p != '=' &&  *p != '\t' && *p != '\r' && *p != '\n' && *p != '\0' )
				p++;
			if( *p == 0 || e <= p )
				goto NEXT_LINE;

			/* �ҵ�ֵ��ʼ */
			p++;
			while( *p == ':' ||  *p == '=' ||  *p == ',' ||  *p == '\t' || *p == '\r' || *p == '\n' )
				p++;
			strncpy( value, p, valuelen-1<e-p ? valuelen-1:e-p );//�Ѿ���0�ˣ�ʹ��memcpy��׼ȷ
			return value;
		}
NEXT_LINE:
		p = e + 1;
	}
	return NULL;
}

/*��ȡ��ǰ�Ự��error code*/
int sw_httpfile_priv_get_err_code(HANDLE file)
{
	http_file_t *p_http_file = file;

	return sw_httpclient_get_err_code( p_http_file->h_client);

}

/* ��ʼ��httpfile */
HANDLE sw_httpfile_priv_init( char *url, int timeout )
{
	http_file_t* p_http_file = NULL; 
	int size, recvsize;
	int retcode;
	int64_t startpos=0, length=0;
	sw_url_t surl;
	unsigned char *p, *p1;
	int i, n, repeat;
	char value[256], cookie[512], url2[2048];
	char host_buf[256], end_pos[128];
	unsigned long rrsIP[8];
	unsigned short rrsPort[8];
	int rrsidx, rrscnt;
	char* orgurl;
	
	SWCOMMON_LOG_DEBUG( "sw_httpfile_priv_init(\"%s\")\n", url );
	if(	m_mutex == NULL)	
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);

	if( m_ref < 0 )
	{
		m_ref = 0;
		memset( m_all, 0, sizeof(m_all) );
	}
	for( i=0; i<MAX_HTTFILE_NUM; i++ )
	{
		if( m_all[i].used == 0 )
		{
			m_all[i].used = 1;
			p_http_file = m_all + i;
			break;
		}
	}
	m_ref++;
	if(m_mutex)
		sw_mutex_unlock(m_mutex);

	SWCOMMON_LOG_DEBUG("[HTTPFILE]: %p, m_ref:%d timeout:%d\n", p_http_file, m_ref, timeout);
	
	if( p_http_file == NULL )
		goto ERROR_EXIT;
	*cookie = 0;
	repeat = 0;
	rrsidx = 0;
	rrscnt = 0;
	orgurl = url;

REDO:
	/* ����URL */
	p = (unsigned char*)strchr(url, ';');
	if( p )
		*p = 0;
	sw_url_parse( &surl, (char*)url );
 
	//��Ϊ�����rrs���ַ���������url������rrsip=ip:port�Ĳ���
	if( rrscnt==0 )
	{
		char szIP[32];
		memset( rrsIP, 0, sizeof(rrsIP) );
		memset( rrsPort, 0, sizeof(rrsPort) );
		rrsIP[0] = surl.ip;
		rrsPort[0] = surl.port;
		rrscnt++;

		p1 = (unsigned char*)url;     
		while( rrscnt < (int)(sizeof(rrsIP)/sizeof(rrsIP[0])) )
		{
			p1 = (unsigned char*)strstr((char*)p1, "rrsip=");
			if( p1==NULL )
				break;
			p1 += 6;
			//������ip:port
			for( i=0; p1[i]=='.' || ('0'<=p1[i] && p1[i]<='9'); i++ );
			if( i >= (int)sizeof(szIP) )
				break;
			memcpy( szIP, p1, i );
			szIP[i] = 0;
			if( !sw_txtparser_is_address(szIP) )
				break;
			rrsIP[rrscnt] = inet_addr(szIP);
			if( p1[i]==':' )
				rrsPort[rrscnt] = htons(atoi((char*)(p1+i+1)));
			rrscnt++;
		}

		//�������rrs�����ڴ���״̬�����ȫ������״̬
		for( n=0; n<rrscnt && find_bad_ip(rrsIP[n])>=0; n++ );
		if( n >= rrscnt )
		{
			for( n=0; n<rrscnt; n++ )
			{
				i = find_bad_ip(rrsIP[n]);
				if( i>=0 )
					m_bad_iplist[i] = 0;
			}
		}
	}
	if( repeat==0 && rrscnt>1 )
	{
		while( rrsidx<rrscnt && find_bad_ip(rrsIP[rrsidx])>=0 )
			rrsidx++;
		if( rrsidx>=rrscnt )
			goto ERROR_EXIT;
		surl.ip = rrsIP[rrsidx];
		surl.port = rrsPort[rrsidx];
	}

	if( p )
	{
		*p = ';';
		p++;
		p1 = (unsigned char*)strstr( (char*)p, "startpos=" );
		if( p1 )
			startpos = str_int64((char*)(p1+9));
		p1 = (unsigned char*)strstr( (char*)p, "length=" );
		if( p1 )
		{
			length = str_int64((char*)(p1+7));
			snprintf( end_pos, sizeof(end_pos), "%lld-%lld", startpos, startpos + length-1 );
		}
	}
	if( surl.port == 0 )
		surl.port = htons(80);
	/* ���������������	 */  
	SWCOMMON_LOG_DEBUG("[surl.ip=%x] %x\n",surl.ip,surl.port);
	if(surl.ip ==0)
		goto ERROR_EXIT;
	p_http_file->h_client = sw_httpclient_connect( surl.ip, surl.port, timeout );	
	if( p_http_file->h_client == NULL )
	{
		if( repeat==0 )	//�������rrsʧ�ܲŽ�������
		{
			add_bad_ip(surl.ip);
			url = orgurl;
			rrsidx++;
			if( rrsidx<rrscnt )
				goto REDO;
		}
		goto ERROR_EXIT;
	}
	if( *cookie )
		sw_httpclient_register_cookies( p_http_file->h_client, cookie );

	/* ����Get���� */
	SWCOMMON_LOG_DEBUG( "sw_httpclient_request 1 surl.ip=%x,surl.port=%x \n",  surl.ip,surl.port);
	snprintf(host_buf, sizeof(host_buf), "%s:%d", surl.hostname, ntohs(surl.port));

	if( ! sw_httpclient_request( p_http_file->h_client ,"GET",
			surl.path, host_buf, NULL, NULL, length>0 ? end_pos : NULL, length>0 ? 0 : startpos, timeout,NULL,NULL) )
	{
		sw_httpclient_set_err_code(p_http_file->h_client, HTTP_ERROR_HTTP_NORESPONSE);
		return p_http_file;
		//goto ERROR_EXIT;
	}
  
	/* ��ȡ�ļ�ͷ���ڻ�ȡ�ļ���Ϣ */
	size = recvsize = 0;
	while( recvsize < (int)sizeof(p_http_file->buf)-1 )
	{
		size = sw_httpclient_recv_data( p_http_file->h_client, (char*)p_http_file->buf+recvsize, 1, timeout );
		if( size <= 0 )
		{
			break;
		}
		recvsize += size;
		p = p_http_file->buf + recvsize - 4;
		if( 4 < recvsize && p[0]=='\r' && p[1]=='\n' && p[2]=='\r' && p[3]=='\n' )
		{
			break;
		}
	}
	if( recvsize <= 0 )
	{
		sw_httpclient_set_err_code(p_http_file->h_client, HTTP_ERROR_HTTP_NORESPONSE);
		return p_http_file;
	}

	p_http_file->buf[recvsize] = 0;

	retcode = sw_httpfile_priv_find_ret_code( (char*)p_http_file->buf, recvsize );
	p_http_file->filesize = sw_httpclient_get_content_size( (char*)p_http_file->buf, recvsize );
	if( retcode==206 )
		p_http_file->filesize += startpos;
	/* �ض��� */
	if( retcode/100 == 3 )
	{
		if(p_http_file->h_client)
		{
			sw_httpclient_disconnect(p_http_file->h_client);
			p_http_file->h_client = NULL;
		}

		i = 0;

		/* ��ȡLocation�ֶ�ֵ */
		memset( value, 0, sizeof(value) );
		SWCOMMON_LOG_DEBUG( "sw_httpfile_priv_init redirect %s\n", p_http_file->buf );
		if( xstrgetval( (char*)(p_http_file->buf), "Location", value, sizeof(value) ) )
			i += sprintf( url2+i, "%s", value );
		else
			goto ERROR_EXIT;
		/* ��ȡcookie�ֶ�ֵ */
#if 0
		/* ���Ա����� Set-Cookie �ֶο������ö������������֮���÷ֺŸ�������Ͳ���ֱ�Ӹ��� */
		memset( value, 0, sizeof(value) );
		if( xstrgetval( (char*)(p_http_file->buf), "Set-Cookie", value, sizeof(value) ) )
		{
			char seps[] = ";";
			char *last = NULL;

			n = 0;
			p = strtok_r(value, seps, &last);
			while( p )
			{
				while( *p == ' ' || *p == '\r' || *p == '\n' )
					p++;
				if( n == 0 )
					i += sprintf( url2+i, "?%s", p );
				else
					i += sprintf( url2+i, "&%s", p );
				p = strtok_r(NULL, seps, &last);
				n++;
			}
		}
#else
		*cookie = 0;
		if( xstrgetval( (char*)(p_http_file->buf), "Set-Cookie", cookie+7, sizeof(cookie)-7 ) )
			memcpy( cookie, "Cookie:", 7 );
#endif

		//����url2������BCD��ת��
		bcd2str( url2 );
		SWCOMMON_LOG_DEBUG( "redirect url: %s\n", url2 );

		if( !strcmp(url, url2) )		/* ���Է��֣�url���ض������� */
			repeat += 5;
		if( ++repeat>20 )
			goto ERROR_EXIT;
		strlcpy( p_http_file->redirect_url, url2, sizeof(p_http_file->redirect_url));
		url = p_http_file->redirect_url;
		goto REDO;
	}
	else if( retcode<=0 || retcode >=400 )
	{
		sw_httpclient_set_err_code(p_http_file->h_client, retcode);
		SWCOMMON_LOG_DEBUG( "Download url not exist (%s) retcode=%d\n", url, retcode );
		return p_http_file;
		//goto ERROR_EXIT;
	}
    else if( retcode == 220 )
    {
        sw_httpclient_set_err_code( p_http_file->h_client, retcode);
        SWCOMMON_LOG_DEBUG( "avoid 21 port return 220 code.\n" );
		return p_http_file;
        //goto ERROR_EXIT;
    }
	sw_httpclient_set_err_code(p_http_file->h_client,  HTTP_OK);
	return p_http_file;

ERROR_EXIT:

	SWCOMMON_LOG_DEBUG( "error: sw_httpfile_priv_init(\"%s\") failed\n", url );

	if( p_http_file )
	{
		if( p_http_file->h_client )
			sw_httpclient_disconnect( p_http_file->h_client );
		if(m_mutex)
			sw_mutex_lock(m_mutex);
		memset( p_http_file, 0, sizeof(*p_http_file) );
		if(m_mutex)
			sw_mutex_unlock(m_mutex);
	}
	m_ref--;
	
	return NULL;
}


/* �˳� */
void sw_httpfile_priv_exit( HANDLE h_file, int timeout )
{
	http_file_t* p_http_file = (http_file_t*)h_file;

	if( p_http_file && p_http_file->h_client )
	{
		sw_httpclient_disconnect( p_http_file->h_client );
		if(m_mutex)
			sw_mutex_lock(m_mutex);
		memset( p_http_file, 0, sizeof(*p_http_file) );
		m_ref--;
		if(m_mutex)
			sw_mutex_unlock(m_mutex);
	}
}

/* ȡ��HTTP���������ļ��Ĵ�С */
int64_t sw_httpfile_priv_get_size( HANDLE h_file )
{
	http_file_t* p_http_file = (http_file_t*)h_file;
	return  p_http_file ? p_http_file->filesize : 0;
}

/* ��ȡ�������ļ� */
int sw_httpfile_priv_get_file( HANDLE h_file, char *buf, int size, int timeout )
{
	int bufpos = 0, recvsize = 0;
	http_file_t* p_http_file = (http_file_t*)h_file;

	while( bufpos < size && bufpos < p_http_file->filesize )
	{
		recvsize = sw_httpclient_recv_data( p_http_file->h_client, buf+bufpos, size-bufpos, timeout );
		if( recvsize <= 0 )
			break;

		bufpos += recvsize;
	}
	return bufpos; 		
}


int sw_httpfile_priv_allocmem_getfile( HANDLE h_file, char **p_buf, int *p_size, int timeout )
{
	http_file_t* p_http_file = (http_file_t*)h_file;
	char *buf;
	int size, recvsize;
	int err = 0;

	struct mem_node
	{
		char buf[64*1024];
		int  size;
		struct mem_node *next;
	}*head, *tail, *p;

	*p_buf = NULL;
	*p_size = 0;
	recvsize = 0;
	tail = NULL;
	if( p_http_file == NULL )
	{
		return -1;
	}

	if( p_http_file->filesize > 0 )
	{
		buf = (char*)malloc( p_http_file->filesize + 1 );
		if( buf==NULL )
		{
			return -4;
		}
		size = sw_httpfile_priv_get_file( h_file, buf, p_http_file->filesize, timeout );
		*p_buf = buf;
		*p_size = size;
		if( size < p_http_file->filesize )
			return -3;
		buf[size] = 0;
		return 0;
	}

	head = NULL;
	size=0;
	while( 1 )
	{
		p = (struct mem_node*)malloc( sizeof(struct mem_node) );
		if( p==NULL )
		{
			printf( "HttpFile_AllocMemGetFile: not enough memory!\n" );
			err = -2;
			break;
		}
		p->size=0;
		p->next=NULL;
		while( p->size < (int)(sizeof(p->buf)) )
		{
			recvsize = sw_httpclient_recv_data( p_http_file->h_client, p->buf+p->size, sizeof(p->buf)-p->size, timeout );
			if( recvsize <= 0 )
				break;
			p->size += recvsize;
		}
		if( head==NULL )
			head=p;
		else
			tail->next=p;
		tail=p;
		size += p->size;
		if( recvsize<0 )
		{
			err = -3;
			break;
		}
		if( recvsize==0 )
			break;
	}

	buf = NULL;
	if( size > 0 && err==0 )
	{
		p_http_file->filesize = size;
		buf = (char*)malloc( p_http_file->filesize + 1 );
		if( buf == NULL )
		{
			err = -4;
			printf( "HttpFile_AllocMemGetFile: OOM!\n" );
		}
		else
		{
			recvsize=0;
			for( p=head; p; p=p->next )
			{
				memcpy( buf+recvsize, p->buf, p->size );
				recvsize += p->size;
			}
			buf[recvsize]=0;
		}
	}
	for( p=head; p; p=tail )
	{
		tail = p->next;
		free(p);
	}

	*p_buf = buf;
	*p_size = size;

	return buf||err ? err : -5;
}




/* ȡ��ʵ��ʹ�õ�URL����Ϊ�п����ض�����*/
char *sw_httpfile_priv_get_url( HANDLE h_file )
{
	return ((http_file_t*)h_file)->redirect_url;
}

/* ȡ��httpclient��� */
HANDLE sw_httpfile_priv_get_client( HANDLE h_file )
{
	return ((http_file_t*)h_file)->h_client;
}

/* ��HTTPͷ�л�ȡ����ֵ */
static int sw_httpfile_priv_find_ret_code( char* p_buf, int size )
{
	int i;
	char sz_buffer[20];

	char* buf = p_buf;
	if( p_buf==NULL )
		return -1;

	if( !strncasecmp(buf, "http/", 5) )
	{
		buf += 5;
		while( (*buf>='0'&&*buf<='9') || (*buf=='.') )
			buf++;
	}

	while( *buf==' ' )
		buf++;

	for(i=0; i<10 && *buf>='0' && *buf<='9'; i++, buf++)
	{
		sz_buffer[i] = *buf;
	}
	sz_buffer[i] = 0;
	return atoi( sz_buffer );
}

#define IsHex(c)	(('0'<=(c) && (c)<='9') || ('a'<=(c) && (c)<='f') || ('A'<=(c) && (c)<='F'))
#define Char2Hex(c) ('0'<=(c) && (c)<='9' ? ((c)&0xf) : ((c)&0xf)+9)

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

