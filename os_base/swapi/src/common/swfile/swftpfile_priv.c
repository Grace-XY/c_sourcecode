#include "swapi.h"
#include "swftpclient.h"
#include "swmem.h"
#include "swthrd.h"
#include "swtxtparser.h"
#include "swurl.h"
#include "swlog.h"
#include "swmutex.h"
#include "swcommon_priv.h"

#define MAX_FTPCLIENT_NUM		4

typedef struct tagftppara
{
	char ftpserver[16];
	char ftpport[8];
	char username[64];
	char userpswd[32];
	char ftppath[1024];
	char ftpfile[128];
	
}sftpparam_t;

/* FtpFile�� */
typedef struct
{
	/* ��ftpclient��� */
	HANDLE hftpclient;
	/* �ļ���С */
	int filesize;
	/*��ǰ·��*/
	char curpath[1024];
	/*�ļ�·��*/
	char ftppath[1024];
	/* �ļ��� */
	char ftpfile[128];
	
}sftpfile_t;

static sftpfile_t m_all[MAX_FTPCLIENT_NUM];
static int m_ref = 0;
static HANDLE m_mutex = NULL;
/* find size */
static unsigned long findsize( char *buf, int len );
/* ����url */
static bool ftp_url_parse( char *url, sftpparam_t *sftp_parm );

/** 
* @brief ��ʼ��
* 
* @param url ָ���ļ����ص�URL
* @param timeout ��ʱʱ��
* 
* @return �������,�ɹ�; ʧ��ΪNULL
*/
HANDLE sw_ftpfile_priv_init( char* url, int timeout )
{
	sftpfile_t *pftpfile = NULL;
	char *recvdata = NULL;
	sftpparam_t ftppara;
	unsigned long ip;
	unsigned short port;
	int i;
	char* p= NULL;

	if( m_mutex == NULL)
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);

	if( m_ref <= 0 )
	{
		memset( m_all, 0, sizeof(m_all) );
		m_ref = 0;
	}
	for( i=0; i<MAX_FTPCLIENT_NUM; i++ )
	{
		if( m_all[i].hftpclient == NULL )
		{
			pftpfile = m_all + i;
			break;
		}
	}
	m_ref++;
	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	if( pftpfile == NULL )
	{
		goto ERROR_EXIT;
	}

	/* ����URL */
	memset( &ftppara, 0, sizeof(ftppara) );   //memset ftpparaָ�� ��ʱ�ͷ�?
	ftp_url_parse( url, &ftppara );
	if( ftppara.ftpfile[0] == '\0' )
	{
		goto ERROR_EXIT;
	}

	/* ����FTP������ */
	memset( pftpfile, 0, sizeof(*pftpfile) );
	strlcpy( pftpfile->ftpfile, ftppara.ftpfile, sizeof(pftpfile->ftpfile) );	
	strlcpy( pftpfile->ftppath, ftppara.ftppath, sizeof(pftpfile->ftppath) );
	pftpfile->filesize = -1;
	if( !sw_txtparser_is_address(ftppara.ftpserver) )
	{
		struct hostent *h = gethostbyname(ftppara.ftpserver);  //
		if( h!= NULL && h->h_addr_list != NULL && h->h_addr_list[0] != NULL)
			memcpy(&ip, h->h_addr_list[0], sizeof(ip));//copy����sizeof(unsigned long)
		else
		{
			goto ERROR_EXIT;
		}	
	}
	else
	{
		ip = inet_addr( ftppara.ftpserver );
	}
	
	port = htons( (unsigned short)atoi(ftppara.ftpport) );   //host����  to  net ����
	pftpfile->hftpclient = sw_ftpclient_connect( ip, port,timeout );
	if( pftpfile->hftpclient == NULL )
	{
		goto ERROR_EXIT;
	}
	
	/* ��¼ */
	if( ! sw_ftpclient_login( pftpfile->hftpclient, ftppara.username, ftppara.userpswd, timeout ) )
	{
		goto ERROR_EXIT;
	}
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "SYST", NULL, timeout );

	memset(pftpfile->curpath,0,sizeof(pftpfile->curpath));
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "PWD", NULL, timeout );
	if( recvdata == NULL )
	{
		goto ERROR_EXIT;
	}
	if( atoi(recvdata) == 257 )    //���ܣ��ַ���ת����������,ԭ��: int atoi(const char *nptr); 
								  //����˵��: ����nptr�ַ����������һ���ǿո��ַ������ڻ��߲�������Ҳ�����������򷵻��㣬
								 //����ʼ������ת����֮���⵽�����ֻ������ \0 ʱֹͣת��������������
	{	
		p =strchr(recvdata,'\"');
		if( p != NULL )
		{	
			p++;
			i=0;
			while( *p !='\"' && *p !='\0' && i < (int)sizeof(pftpfile->curpath)-1 )
			{
				pftpfile->curpath[i]=*p;
				i++;
				p++;
			}
			pftpfile->curpath[i]= '\0';
		}
	}

	/* ���ô���ģʽΪ������ */
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "TYPE I", NULL, timeout );  //TYPE type˵���ļ�����,I��ʾͼ��A�ԣ� 
	if( atoi(recvdata) != 200 )    //200��ʾ������׼������
	{
		goto ERROR_EXIT;     
	}

	//m_ref++;
	return pftpfile;

ERROR_EXIT:
	SWCOMMON_LOG_ERROR( "init(\"%s\") failed\n", url );
	if( pftpfile )
	{
		if( pftpfile->hftpclient )
		{
			sw_ftpclient_disconnect( pftpfile->hftpclient, timeout );
		}
		if(m_mutex)
		{
			sw_mutex_lock(m_mutex);
		}
		memset( pftpfile, 0, sizeof(*pftpfile) );
        if(m_mutex)
		{
			sw_mutex_unlock(m_mutex);
		}
	}
	if(m_mutex)
	{
		sw_mutex_lock(m_mutex);
	}
	m_ref--;
	if(m_mutex)
	{
		sw_mutex_unlock(m_mutex);
	}
	return NULL;
}

/** 
* @brief �˳�
* 
* @param hfile �������
* @param timeout ��ʱʱ��
*/
void sw_ftpfile_priv_exit( HANDLE hFile, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;

	if( pftpfile && pftpfile->hftpclient )
	{
		sw_ftpclient_disconnect( pftpfile->hftpclient, timeout );
		memset( pftpfile, 0, sizeof(*pftpfile) );
		m_ref--;
	}
}

/** 
* @brief ��ӡ��Ϣ
*/
void sw_ftpfile_priv_print()
{
	int i;

	for( i=0; i<MAX_FTPCLIENT_NUM; i++ )
	{
		if( m_all[i].hftpclient )
		{
			SWCOMMON_LOG_INFO( "ftpfile(%d): %s\n", m_all[i].filesize, m_all[i].ftpfile );
		}
	}
}

/* ȡ��ftpclient��� */
HANDLE sw_ftpfile_priv_get_client( HANDLE hFile )
{
	return ((sftpfile_t *)hFile)->hftpclient;
}

/* ��ȡ�����ļ��Ĵ�С */
int sw_ftpfile_priv_get_size( HANDLE hFile, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;
	char *recvdata = NULL;
	int request = 0;
	
	char path[1024];
	if (snprintf(path, sizeof(path), "%s%s", pftpfile->ftppath, pftpfile->ftpfile) > sizeof(path))
	{
		SWCOMMON_LOG_ERROR("%s too long", path);
		return 0;
	}
	/* ֧��SIZE ����ķ�����	 */
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "SIZE", path, timeout );
	if( atoi(recvdata)==213 )     //213��ʾ�ļ�״̬��Ѷ
	{
		pftpfile->filesize = atol( recvdata+4 );
	}
	else
	{
		pftpfile->filesize = 0;
	}
	
	/* ���������ļ�(ע����ҪҲ��Ϊ�˻�ȡ�ļ���С,��Щ��������֧��SIZE����) */
	request =  atoi( recvdata);
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "RETR", path, timeout );
	if( atoi(recvdata) <= 150 && request == 500 )   //С��150��Ӧ����120,125,150,����ʾRETR�����ʶ�� 
													//�����֧��SIZE,�����500,��ʾ�﷨���󣬲���ʶ�������
	{
		pftpfile->filesize = findsize( recvdata, strlen(recvdata) );
	}

	return pftpfile->filesize;	
}


/** 
* @brief ��ȡ�����ļ��Ĵ�С
* 
* @param hfile �������
* @param timeout ��ʱʱ��
* 
* @return �ļ��Ĵ�С,�ɹ�;����, -1
*/

int sw_ftpfile_priv_get_file( HANDLE hFile, char *buf, int size, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;
	int totalsize = 0;
	int recvsize = 0 ,times = 0; //�������ӵĴ���
	int maxsize = 0;
	int bufpos = 0;
	
	/* û�е���sw_ftpfile_priv_get_size */
	if( pftpfile->filesize < 0 )
	{
		sw_ftpfile_priv_get_size( hFile, timeout );	
	}
     	
	totalsize = size < pftpfile->filesize ? size : pftpfile->filesize;
	if( totalsize <= 0 )
	{
		return 0;
	}
	
	memset( buf,0,totalsize );
	
	/* �������� */
	while( bufpos < totalsize )
	{
		maxsize = 4096 < (totalsize - bufpos) ? 4096 : (totalsize - bufpos);
		if( ( recvsize = sw_ftpclient_recv_data( pftpfile->hftpclient,  buf+bufpos, maxsize, timeout ) ) > 0 )
		{
			bufpos += recvsize;
			times = 0;
			if( bufpos == totalsize )
			{
				break;
			}
		}
		else
		{
			sw_thrd_delay(10);
			times++;
			if( times >= 10 )
			{
				//printf("ftp unable read all the data\n");
				break;
			}
		}
	}
	
	return bufpos;		
}

/** 
* @brief �ϴ�����
* 
* @param hfile �������
* @param buf ָ��洢�ļ��Ļ�����
* @param size ��������С
* @param timeout ��ʱʱ��
* 
* @return �ļ���С���ɹ���-1��ʧ��  //return 0,�ɹ�; -1,ʧ��
*/
int sw_ftpfile_priv_upload_file( HANDLE hFile, char *buf, int size, int timeout )
{
	sftpfile_t *pftpfile = (sftpfile_t *)hFile;
	char *recvdata = NULL;
	int sendsize = 0,times = 0;
	int bufpos = 0;
	if(pftpfile->curpath[0] != '\0' && pftpfile->curpath[strlen(pftpfile->curpath)-1] == '/')   //�޸ĵ�ǰ·��
	{
		pftpfile->curpath[strlen(pftpfile->curpath)-1] = '\0'; 	
	}
	if (strlcat(pftpfile->curpath,pftpfile->ftppath, sizeof(pftpfile->curpath)) >= sizeof(pftpfile->curpath))
		SWCOMMON_LOG_ERROR("%s too long", pftpfile->curpath);

	/*�ı䵱ǰ��·��*/
    if (strlen(pftpfile->curpath) > 0)
    {
        recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "CWD", pftpfile->curpath, timeout );//???>250ʲô����
        if( atoi( recvdata) > 250 )
            return 0;
    }
	/* �ϴ��ļ� */
	recvdata = sw_ftpclient_send_command( pftpfile->hftpclient, "STOR", pftpfile->ftpfile, timeout );  //STOR�洢�ļ�
	if( atoi( recvdata) <=150 )
	{	
		while ( times < 10 )
		{
			if( size <= 1024 )
			{
				sendsize = sw_ftpclient_send_data( pftpfile->hftpclient, buf+bufpos,size, timeout );  //��ʼ������
				if ( 0 < sendsize )
				{	
					bufpos += sendsize;
					break;
				}
				else 
				{
					sw_thrd_delay(100);
					times++;													
				}
			}
			else
			{
				sendsize = sw_ftpclient_send_data( pftpfile->hftpclient, buf+bufpos,1024, timeout );
				if ( sendsize > 0 )
				{
					bufpos += sendsize;
					size = size - sendsize;
					times = 0;		
				}
				else
				{
					sw_thrd_delay(100);
					times++;													
				}
			}
		}
	}
	return bufpos;	
}


/* find size */
static unsigned long findsize( char *buf, int len )  //???
{
	char *p = NULL;
	if( buf && atoi(buf)<=150 )
	{
		p = strchr( buf, '(' );  //Ӧ���ʽ:150 Opening BINARY mode data connection for bj.c (1010 bytes). 226 Transfer complete.
		if( p && (p+1) )
		{
			return atoi( p+1 ); //atoi(),����˵��: ����nptr�ַ�����
			                   //�����һ���ǿո��ַ������ڻ��߲�������Ҳ�����������򷵻��㣬����ʼ������ת����
							   //֮���⵽�����ֻ������ \0 ʱֹͣת���������������� 
		}
	}
	return 0;
}

/* ����url */
static bool ftp_url_parse( char *url, sftpparam_t *sftp_parm  )
{
	sw_url_t surl;
	char* p = NULL;
	if( !url || strncmp( url,"ftp://",6)||!sftp_parm  )
	{
		return false;
	}

	sw_url_parse(&surl,url);      //ftp��ʽ  ftp://�û���:����@��ַ(������):�˿�/    ��ftp://j00395:123456@210.42.35.26:8080/ 

	strlcpy(sftp_parm->ftpserver,surl.hostname,sizeof(sftp_parm->ftpserver));   //hostname ��������IP��ַ��������

	if( strlen(surl.user) !=0 )	
	{
		strlcpy(sftp_parm->username,surl.user,sizeof(sftp_parm->username)); //username �û���
	}
	else
		strlcpy( sftp_parm->username,"anonymous", sizeof(sftp_parm->username));
	
	if( strlen(surl.pswd) !=0 )	
	{
		strlcpy(sftp_parm->userpswd,surl.pswd,sizeof(sftp_parm->userpswd));  //pswd ����
	}

	if( surl.port == 0 )
	{
		strlcpy( sftp_parm->ftpport, "21", sizeof(sftp_parm->ftpport) );    //port �˿ں�   20���ڿ������ӣ�21������������,��P320
	}
	else	
	{
		snprintf(sftp_parm->ftpport, sizeof(sftp_parm->ftpport), "%d", ntohs(surl.port));
	}

	strlcpy( sftp_parm->ftppath, surl.path, sizeof(sftp_parm->ftppath) );   //path �ļ�·��
	p = strrchr(sftp_parm->ftppath,'/');
	if(p)
	{
		*(p+1) = '\0';
	}
	strlcpy( sftp_parm->ftpfile, surl.tail, sizeof(sftp_parm->ftpfile) );  //ftpfile �ļ���


	return true;
}

