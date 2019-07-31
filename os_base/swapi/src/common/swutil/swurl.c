/***************************************************************************
* AUTHOR:  
* CONTENT: ʵ��URL��������
* NOTE:	
* HISTORY:
* [2005-10-05] created
****************************************************************************/
#include "swapi.h"
#include "swurl.h"
#include "swsignal.h"
#include "swmutex.h"
#include "swthrd.h"
#include <setjmp.h>
#include <netdb.h>
#include "swgethostbyname.h"


/* �����ǲ���һ����Ч�ĵ��ʮ����IP��ַ */
static bool IsAddress(const char* buf);
/* ����url�еİٷֺţ�������Ŀ�껺������ */
static int url_copy(const char* url, int len, char* buf, int size);

/* ����url��destURL�ṹ��,0��ʾ�����ɹ�,-1����ʧ�� */
int sw_url_parse(sw_url_t* dst, const char* url)
{
	const char* p = NULL;
	const char* q = NULL;
	const char* r = NULL;
	const char* s = NULL;
	const char* end = url + strlen(url);
	int i=0,j=0;
	
	memset(dst, 0, sizeof(sw_url_t));

	/* find head */
	p = strchr( url,':');
	if( p != NULL && (*(p+1)=='/' || *(p+1)=='\\') )
	{
		q = p -1;
		for(;q>url;q--)
		{
			if( !( (*q>='a' && *q<='z') || (*q>='A' && *q<='Z') || (*q>='0' && *q<='9') ) ) 
			{
				q++;
				break;	
			}
		}
		for( i=0;i< (int)sizeof(dst->head) - 1 && q<p;i++,q++)
			dst->head[i]= *q;	
		
		dst->head[i]='\0';
		
		/* skip ':' */
		p++;
		/* skip "// " */
		for(i=0;i<2;i++)
		{
			if( *p=='/' || *p=='\\' )	
				p++;
		}	
	}
	else
	{
		p = url;
	}

	/* find user name & password */
	q = strchr( p,'@');
	r = strchr( p, '/' );
	if( r == NULL )
		r = strchr( p, '\\' );
    
    if( q != NULL && (r==NULL || q<r) )
    {
      //���⴦�������д���@    
        while( q!=NULL && strchr(q+1,'@' ) )
        {
            if( r == NULL || q < r )
                q = strchr(q+1,'@' );
            else   
                break;
       }  
    }
    
	if( q != NULL && (r==NULL || q<r) )
	{
		i=0;j=-1;
		for( ;p<q; p++)
		{
			if( i < (int)sizeof( dst->user ) - 1 && j < 0 && *p!=':')
				dst->user[i++] = *p;
			else if( j >= 0 && j< (int)sizeof( dst->pswd)-1 )
				dst->pswd[j++] = *p;
			else if( *p == ':')
				j=0;
		}
		p++;
	}
	
	/* find hostname & port	 */
	i = 0;
	q = end;
	//һ��ipv6��ʽΪ:igmp://[FF01::101]:1234
	if( *p == '[' && 0 != strchr(p,']') )
	{
		dst->has_ipv6 = true;
		p++;
		r = strchr(p,']');
		if( r && r>p )
		{
            memcpy(dst->hostname,p, (sizeof(dst->hostname) <= (size_t)(r-p)) ? sizeof(dst->hostname)-1 : (size_t)(r-p));
			if (sizeof(dst->hostname) > (size_t)(r-p))dst->hostname[r-p] = '\0';
			r++;
			if( *r == ':' )
			{
				r++;
				dst->port =(uint16_t)strtol(r,(char**)&q,0);
				p = q;
			}
			else
				dst->port = 0;	
		}
		else
			return -1;	
	}
	else
	{
		dst->has_ipv4 = true;
		for( ;p<q; )
		{
			/* ��header */
			if( *p == ':' || *p=='/' || *p=='\\' || *p=='?' )
			{
				/* find port */
				if( *p == ':' )
				{
					/* skip it */
					p++;
					dst->port =(uint16_t)strtol(p,(char**)&q,0);/* �˶˿ڱ�׼��10���Ƹ�ʽ */
					dst->port = (uint16_t)atoi(p);
					p = q;
				}
				else
					dst->port = 0;
				break;
			}
			else if( i< (int)sizeof(dst->hostname)-1 ) 
			{
				dst->hostname[i] = *p;  	
				i++;
				p++;
			}	
			else
			{
				i=0;
			}
		}
		dst->hostname[i] = '\0';	
	}	
	/* find path  */
	i = 0;
	q = end;
	r = NULL;
	for( ;p < q && i < (int)sizeof( dst->path ) - 1; p++)
	{
		dst->path[i] = *p; 	
		if( *p == '?' || *p ==';')
			r = p;
		else if( !r && ( *p=='/' || *p =='\\') )
			s =p;
			
		i++;
	}
	dst->path[i] = '\0';
	if( !r )
		r = q;		
		
	/* find tail & suffix from path */
	i=0;
	j=0;
	p = s;
	q = r;
	if( p != NULL )
	{
		/* skip '/' or '\\' */
		p++;
		for(;p<=q ;p++)
		{
			if( *p != '?' &&  *p != ';' && *p != '\0' ) 
			{
				if( i < (int)sizeof(dst->tail)-1 )
				{
					dst->tail[i]= *p ;
					i++;	
				}
								
				if( *p == '.' || j>0 )
				{
				  if(j>0 && *p == '.')
				    {
				      j = 0;
				      dst->suffix[j] = *p;
				    }
				  if( j < (int)sizeof(dst->suffix)-1 )
					{
						dst->suffix[j] = *p;
						j++;
					}
				    
				}
			}
			else
				break;
		}
		
		dst->tail[i] = '\0';
		dst->suffix[j] = '\0';
	}
	
	if ( strcasecmp(dst->head, "file") != 0 )
	{/* file��ͷ��url�̶���Ϊ�����ͱ����ļ� */
		if( dst->hostname[0] != '\0' )
		{
			//��������ipv6�ĵ�ַ����ô����Ϊ�䲻��������
			if(dst->has_ipv6 == true)
			{
				memset(&dst->ipv6,0,sizeof(dst->ipv6));
				//�����жϿ��ܲ��Ǻܱ�׼����Ҫ�Ľ�
				if(strchr(dst->hostname,':'))
					inet_pton(AF_INET6, dst->hostname, &dst->ipv6);
			}
			else
			{
				/* ����ת��ip��port */
				if( !IsAddress(dst->hostname)  ) //!inet_aton(dst->hostname, NULL)
				{/* ʵ������internet�ϵ�����ip��ַ���ʽ��׼:a,a.b,a.b.c,a.b.c.d����8��10��16����������),��inet_atonȥ�жϸ�Ϊ׼ȷ */
#ifdef VXWORKS		
					dst->ip = hostGetByName( dst->hostname);
					if( dst->ip != ERROR )
					{
						char szBuf[INET_ADDR_LEN];
						memset( (void*)szBuf, 0, sizeof(szBuf) );
						inet_ntoa_b( *((struct in_addr*)&(dst->ip)), szBuf );
						hostAdd( dst->hostname, szBuf );
					}
#else
					// ���gethostbyname��ס������ �����̸߳���������
					dst->ip = sw_gethostbyname2(dst->hostname,10000);
					if( dst->ip != 0 )
					{
						printf("%s:%d parse domain success.\n",__FUNCTION__,__LINE__);
						dst->has_ipv4 = true;
					}
					else
					{
						printf("%s:%d parse domain failed.\n",__FUNCTION__,__LINE__);
						return -1;
					}
#if 0
					struct hostent *h = gethostbyname2(dst->hostname,AF_INET);
					struct hostent *h6 = gethostbyname2(dst->hostname,AF_INET6);
					if( h != NULL )
					{
						dst->has_ipv4 = true;
						memcpy(&(dst->ip), h->h_addr_list[0], sizeof(dst->ip));	//��δ���û�б����ȥ
					}
					if( h6 != NULL )
					{
						dst->has_ipv6 = true;
						memcpy(&(dst->ipv6), h6->h_addr, sizeof(dst->ipv6));	//��δ���û�б����ȥ 
					}
#endif
#endif
				}
				else
				{
					dst->has_ipv4 = true;
					dst->ip = inet_addr(dst->hostname);
				}
			}
		}
	}
    //host is used for "Host:" in http request
    if (dst->port == 0)
        strlcpy(dst->host, dst->hostname, sizeof(dst->host));
    else
        snprintf(dst->host, sizeof(dst->host), "%s:%d", dst->hostname, dst->port);
	/* sw_url_t�˿�Ϊ������ */
	dst->port =  htons(dst->port);
	
	return 0;
}

/* ��ȡURL������ */
char *sw_url_get_header(const char *url, char *header, int size)
{
	if ( url == NULL || header == NULL || size <= 0 )
		return NULL;
	const char *p = strchr( url,':');
	const char *q;
	int i;
	if( p != NULL && (*(p+1)=='/' || *(p+1)=='\\') )
	{
		q = p -1;
		for(;q>url;q--)
		{
			if( !( (*q>='a' && *q<='z') || (*q>='A' && *q<='Z') || (*q>='0' && *q<='9') ) ) 
			{
				q++;
				break;	
			}
		}
		for( i=0; i< size - 1 && q<p;i++,q++)
			header[i]= *q;
		header[i]='\0';
	}
	return header;
}
/* ��ȡURL�е�path·�� */
char *sw_url_get_path(const char *url, char *path, int size)
{
	if ( url == NULL || path == NULL || size <= 0 )
		return NULL;
	const char *q, *r;
	const char *p = strchr( url,':');
	int i;
	p = strchr( url,':');
	if( p != NULL && (*(p+1)=='/' || *(p+1)=='\\') )
	{
		p++;/* skip ':' */	
		for(i=0;i<2;i++)
		{/* skip "// " */
			if( *p=='/' || *p=='\\' )	
				p++;
		}	
	}
	else
		p = url;
	/* find user name & password */
	q = strchr( p,'@');
	r = strchr( p, '/' );
	if( r == NULL )
		r = strchr( p, '\\' );
	if( q != NULL && (r==NULL || q<r) )
		p = q+1;
	/* find hostname & port */
	while ( !(*p == ':' || *p=='/' || *p=='\\') && *p != '\0' )
	 	p++;
	if (*p == ':' )
	{
		p++;
		strtol(p,(char**)&q,0);
		p = q;
	}
	if ( p != NULL )
		url_copy(p, strlen(p), path, size);
	else
		*path = '\0';
	return path;
}
/* ȡ��URL�еĲ��� */
char* sw_url_get_param_value( const char* url, const char* name, char *value, int valuesize )
{
	int i=0;
	const char *p;

	p = strstr( url, name );
	/* ȷ���ҵ�name���� */
	while( p != NULL )
	{
		/* param name ǰ������Ƿֽ��,��������ǵȺ� */
		if( ( p == url  || ( p > url && 
			( *(p-1) == '?' || *(p-1) == '&' ) ) )
			&&  *(p+strlen(name)) == '=' )	 
		{
			/* ��λ��'=' */
			p = p + strlen(name);
			/* skip '=' */
			p++;
			
			/* ����һ���ֽ��,��Ϊ�ǽ��� */
			for( i = 0; p[i] && p[i]!='&'; i++ );
			url_copy( p, i, value, valuesize );
			return value;
		}
   		else
		    p = p + strlen(name);
		
		/* û���ҵ�param name,���������param name */	
		p = strstr(p,name);
	}
	
	return NULL;
}


/* ȡ��URL�еĲ��� */
char*  sw_url_get_param(char* url,char* name)
{
	static char value[128];
	if( sw_url_get_param_value( url, name, value, sizeof(value) ) )
		return value;
	else
		return NULL;
}


/* ��URL����ȡ���� */
int sw_url_get_param_int( const char* url, const char* name )
{
	char value[128];
	char* p = sw_url_get_param_value( url, name, value, sizeof(value) );
	if( p )
		return atoi(p);
	else
		return -1;
}


/* ��URL���б��� */
int sw_url_encode(const char* in, char* out)
{
	int i = 0;
	char table[] ={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	int count = 0;
	int len = strlen(in);
	for(i=0;i<len;i++)
	{
	 	if( in[i]>=0x21 && in[i]<=0x7E && in[i]!='<' && in[i]!='>' && in[i]!='^' && in[i]!='[' 
			 && in[i]!=']' && in[i]!='{' && in[i]!='}' && in[i]!='%' )
		{
			out[count++] = in[i];
		}
		else
		{
	    	out[count++]='%';
	    	out[count++]=table[ ((unsigned char)in[i])/16];
	    	out[count++]=table[ ((unsigned char)in[i])%16];
		}
	}
	out[count++]='\0';
	return strlen(out);
}
/* �����ǲ���һ����Ч�ĵ��ʮ����IP��ַ */
static bool IsAddress(const char* buf)
{
	int i, j, k;
	int sip;
	j = i = 0 ;
	k = 0;
	//to do 
	//IPv6�Ϸ�������ж���Ҫ�о�������������buf��:��Ϊ��ipv6�ĵ�ַ���������ҵ����ǺϷ���
	if( strchr(buf,':') != 0 )
		return true;
	while (k < 4)
	{
		sip = 0;
		while (buf[i] != '.' && buf[i] != '\0')
		{
			if ( 2 < (i-j) || buf[i] < '0' || '9' < buf[i] )
				return false;
			sip = sip * 10 + buf[i] - '0';
			i++;
		}
		if ( j == i || sip > 255 )/* û������.9..0���ߴ���255 */
			return false;
		i++;
		j = i;
		++k;
	}
	return (k == 4 && buf[i-1] == '\0' && buf[i-2] != '.') ? true : false;
}


#define IsHex(c)	('0'<=(c) && (c)<='9' || 'a'<=(c) && (c)<='f' || 'A'<=(c) && (c)<='F')
#define Char2Hex(c) ('0'<=(c) && (c)<='9' ? ((c)&0xf) : ((c)&0xf)+9)

static int url_copy(const char* url, int len, char* buf, int size)
{
	const char* p;
	int i = 0;

	for(p = url; p-url < len && *p; p++)
	{
		if( i >= size )
			break;
		if( p[0] == '%' && IsHex(p[1]) && IsHex(p[2]) )
		{
			buf[i] = Char2Hex(p[1])<<4 | Char2Hex(p[2]);
			p += 2;
		}
		else
		{
			buf[i] = *p;
			if( p[0] == '%' && p[1] == '%' )
				p ++;
		}
		i++;
	} 
	if( i >= size )
		i = size - 1;
	buf[i] = '\0';
	return i;
}

#if 0
int main( int argc, char *argv[] )
{

	sw_url_t url;
  	sw_url_parse(&url, argv[1]);

	sw_url_get_param(argv[1], "DRM"); 

	getchar(); 
	sw_url_parse(&url, argv[1]);

	printf ("tail=%s\n", url.tail);
  	return 0;
}
#endif

