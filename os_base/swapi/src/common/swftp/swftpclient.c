/**
 * @file swftpclient.h
 * @brief FTP�����ӿ�
 * @author ...
 * @date 2007-09-06
 * @history
 * @2010-8-5 xusheng modify the format
 */


#include "swapi.h"
#include "swftpclient.h"
#include "swmem.h"
#include "swthrd.h"
#include "swtcp.h"
#include "swcommon_priv.h"

#define MAX_FTPFILE_NUM		4

static sftpclient_t m_all[MAX_FTPFILE_NUM];
static int m_ref = 0;

/** 
* @brief �����������������
* 
* @param ip ��������ip (ip��port��Ϊ�����ֽ���)
* @param port ���Ӷ˿�
* @param timeout ��ʱʱ��
* 
* @return �������
*/

HANDLE sw_ftpclient_connect( unsigned long ip, unsigned short port, int timeout )
{
	sftpclient_t *pftpclient = NULL;
	unsigned long unblock = 1;
	fd_set rset;
	int i;

	if( m_ref <= 0 )
	{
		memset( m_all, 0, sizeof(m_all) );
		for( i=0; i<MAX_FTPFILE_NUM; i++ )
		{	
			m_all[i].cmdskt = m_all[i].datskt = -1;
		}
		m_ref = 0;
	}

	for( i=0; i<MAX_FTPFILE_NUM; i++ )
	{
		if( m_all[i].cmdskt < 0 )
		{
			pftpclient = m_all + i;  	//ʹpftpclientָ����е�XXX
			break;
		}
	}
	if( pftpclient == NULL )
		goto ERROR_EXIT;

	memset( pftpclient, 0, sizeof(*pftpclient) );
	pftpclient->cmdskt = -1;
	pftpclient->datskt = -1;
	pftpclient->ip = ip;
	pftpclient->cmdport = port;		//Ϊʲô����dataport��ֵ
	pftpclient->type = 1;

	/* ����socket */
	pftpclient->cmdskt = sw_tcp_socket();
	if( pftpclient->cmdskt < 0 )
	{
		goto ERROR_EXIT;
	}
	/* ����Ϊ������ */
	if( sw_tcp_ioctl( pftpclient->cmdskt, FIONBIO, &unblock ) < 0 )
	{
		goto ERROR_EXIT;
	}
	/* ����... */
	sw_tcp_connect( pftpclient->cmdskt, ip, port );
	/* �ȴ����ӳɹ� */
	if( sw_tcp_select( pftpclient->cmdskt, &rset, NULL, NULL, timeout ) < 0 )
	{
		goto ERROR_EXIT;
	}

	m_ref++;
	return pftpclient;

ERROR_EXIT:

	if( pftpclient != NULL && 0 <= pftpclient->cmdskt )
	{
		sw_tcp_close( pftpclient->cmdskt );
		pftpclient->cmdskt = -1;
	}

	return NULL;
}

/** 
* @brief �Ͽ���ftp������������
* 
* @param hftp �������
* @param timeout ��ʱʱ��
*/
void sw_ftpclient_disconnect( HANDLE hftp, int timeout )
{
	int ret = 0;
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;

	if( pftpclient && 0 <= pftpclient->cmdskt )
	{
		/* �ر����ݶ˿ڵ����� */
		if( 0 <= pftpclient->datskt )
		{	
			/* �ȴ����ݴ������ */
			while( ! pftpclient->type && 0 < timeout )
			{	
				ret = atoi( sw_ftpclient_send_command(hftp,NULL,NULL,timeout) );
				if( ret == 226 || ret == 426 )     //226��???
				{
					break;
				}
				else
				{
					sw_thrd_delay(500);
					timeout -= 500;
				}
			}

			sw_tcp_close( pftpclient->datskt );
			pftpclient->datskt = -1;
		}
		/* �ر�ָ������ */
		sw_tcp_close( pftpclient->cmdskt );
		pftpclient->cmdskt = -1;
		m_ref--;
	}
}


/** 
* @brief ����ftp������,�������ݶ˿�����
* 
* @param hftp �������
* @param username �û���
* @param password ����
* @param timeout ��ʱʱ��
* 
* @return true,�ɹ�;����, false
*/
bool sw_ftpclient_login( HANDLE hftp, char* username, char* password, int timeout )
{
	unsigned long unblock = 1;
	char *ptr, *recvdata;
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set rset, wset;
    int ret;

	/* ���շ����������� */
	recvdata = sw_ftpclient_send_command( hftp, NULL, NULL, timeout );
	if( atoi(recvdata) != 220 )     //�����220,�������331,230����ʲô����?  220��ʾ:���û�����׼������ 
	{
		return false;
	}

	/* ����û��� */
	recvdata = sw_ftpclient_send_command( hftp, "USER", username, timeout );
	if( atoi(recvdata) != 331 && atoi(recvdata ) != 230 )  //331��ʾ:�û�����ȷ����Ҫ���230��ʾ���û���¼
	{
		return false;
	}
	/* ��Ҫ���� */
	if( atoi( recvdata ) != 230 )    //recvdata==331:USER��ȷ����ҪPASS.
	{
		recvdata = sw_ftpclient_send_command( hftp, "PASS", password, timeout );
		if( atoi( recvdata) != 230 )   //recvdata==230:USER��¼��
		{
			return false;
		}
	}
	/* ����ģʽ */
	recvdata = sw_ftpclient_send_command( hftp,"PASV", NULL, timeout );

	/* ��ȡ���ݶ˿� */
	ptr = strrchr( recvdata, ',' ) ;
	if( ptr )        //GOOD�� ���磺140.252.12.34.4.150  
	{
		*ptr = '\0';
	     pftpclient->datport = atoi( ptr + 1 );
	     ptr = strrchr( recvdata, ',' );

		 if( ptr )
		 {
	         *ptr = '\0';
	         pftpclient->datport += atoi( ptr + 1 ) * 256;
		 }
	}

	/* ������������ */
	pftpclient->datskt = sw_tcp_socket();
	if( pftpclient->datskt < 0 )
	{
		goto ERROR_EXIT;
	}
	/* ����Ϊ������ */
	if( sw_tcp_ioctl( pftpclient->datskt, FIONBIO, &unblock ) < 0 )
	{
		goto ERROR_EXIT;
	}
	/* ����... */
	sw_tcp_connect( pftpclient->datskt, pftpclient->ip, htons(pftpclient->datport) );
	/* �ȴ����ӳɹ� */
    ret = sw_tcp_select( pftpclient->datskt, &rset, &wset, NULL, timeout );
    if (ret < 0)
	{
		goto ERROR_EXIT;
	}
    else if (FD_ISSET(pftpclient->datskt, &rset))
	{
		goto ERROR_EXIT;
	}

	return true;

ERROR_EXIT:
	SWCOMMON_LOG_ERROR( "connect ftp data port failed !\n" );
	if( 0 <= pftpclient->datskt )
	{
		sw_tcp_close( pftpclient->datskt );
		pftpclient->datskt = -1;
	}

	return false;
}

/** 
* @brief ��ftp���������ݶ˿ڽ�������
* 
* @param hftp �������
* @param buf ���ջ���
* @param size �����С
* @param timeout ��ʱʱ��
* 
* @return ���մ�С
*/
int sw_ftpclient_recv_data( HANDLE hftp, char *buf, int size, int timeout )
{
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set rset;

	/* ���״̬ */
	if( sw_tcp_select( pftpclient->datskt, &rset, NULL, NULL, timeout ) < 0 )
	{
		return 0;
	}

	/* �������� */
	if( FD_ISSET(pftpclient->datskt, &rset) )
	{
		return sw_tcp_recv( pftpclient->datskt, buf, size );
	}
	else
	{
		return 0;
	}
}

/** 
* @brief ��ftp���������ݶ˿ڷ�������
* 
* @param hftp �������
* @param buf ���ͻ���
* @param size �����С
* @param timeout ��ʱʱ��
* 
* @return ���ʹ�С
*/
int sw_ftpclient_send_data( HANDLE hftp, char *buf, int size, int timeout )
{
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set wset;

	/* ���״̬ */
	if( sw_tcp_select( pftpclient->datskt, NULL, &wset, NULL, timeout ) < 0 )
	{
		return -1;
	}
	
	/* �������� */
	if( FD_ISSET( pftpclient->datskt, &wset ) )
	{
		return sw_tcp_send( pftpclient->datskt, buf, size );
	}
	else
	{
		return 0;
	}
}


/** 
* @brief ��ftp��������������
* 
* @param hftp �������
* @param type ��������
* @param command ����
* @param timeout ��ʱʱ��
* 
* @return �������Ļظ�
*/
char* sw_ftpclient_send_command( HANDLE hftp, char *type,  char* command, int timeout )
{
	sftpclient_t *pftpclient = (sftpclient_t*)hftp;
	fd_set rset, wset;
	char ftpcmd[2048];

	char return_end[8] = "\0";

	/* ��ջش� */
	memset( pftpclient->response, 0, sizeof(pftpclient->response) );
	memset(return_end, 0, sizeof(return_end));

	/* ����ftp���� */
	memset( ftpcmd,0 , sizeof(ftpcmd) );
	if( type )
	{
		if( command )
		{
			snprintf( ftpcmd, sizeof(ftpcmd), "%s %s\r\n", type, command );
		}
		else
		{
			snprintf( ftpcmd, sizeof(ftpcmd), "%s\r\n", type );
		}

		/* ���״̬ */
		sw_tcp_select( pftpclient->cmdskt, NULL, &wset, NULL, timeout );

		/* �������� */
		if( FD_ISSET( pftpclient->cmdskt, &wset ) )
		{
			sw_tcp_send( pftpclient->cmdskt, ftpcmd,strlen(ftpcmd));
		}
	}

	/* ���״̬ */
	sw_tcp_select( pftpclient->cmdskt, &rset, NULL, NULL, timeout );

	/* ���ջش� */
	if( FD_ISSET( pftpclient->cmdskt, &rset ) )
	{
		if(sw_tcp_recv( pftpclient->cmdskt, pftpclient->response, sizeof(pftpclient->response)-1 ) <= 0)
		{
			return pftpclient->response;
		}
		strncpy(return_end, pftpclient->response, 3);   //copy Ӧ���ǰ3���ַ����磺231//�Ѿ���return_end��0��
		strlcat(return_end, " ", sizeof(return_end));//�Ӹ��ո�

		while(strstr(pftpclient->response, return_end) == NULL)   //������Ϊ"\r\n"ʱ����
		{

			int return_bytes;
			char ftp_return_cmd[512] = "\0";
			memset( ftp_return_cmd, 0, sizeof(ftp_return_cmd));

			/* ���״̬ */
			if(	sw_tcp_select( pftpclient->cmdskt, &rset, NULL, NULL, timeout ) <= 0 )
			{
				break;
			}
			/* ���ջش� */
			if( FD_ISSET( pftpclient->cmdskt, &rset ) )
			{
				if(( return_bytes = sw_tcp_recv( pftpclient->cmdskt, ftp_return_cmd, sizeof(ftp_return_cmd)-1 )) > 0)
				{
					int len = strlcat(pftpclient->response, ftp_return_cmd, sizeof(pftpclient->response));
					if (len >= (int)sizeof(pftpclient->response))//�Ѿ��޷��ڽ��ܶ����ַ���
						break;
				}
				else
				{
					break;
				}
			}

		}
	}

	if( type && strncmp(type,"STOR",4) == 0 && atoi(pftpclient->response) != 550  )
	{
		pftpclient->type = 0;
	}

	return pftpclient->response;
}
