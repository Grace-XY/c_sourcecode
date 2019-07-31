/** 
 * @file swftpclient.h
 * @brief FTP�����ӿ�
 * @author ...
 * @date 2007-09-06
 * @history
 * @2010-8-5 xusheng modify the format
 */


#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_

typedef struct
{
	/* ftp��������������׽��� */
	int cmdskt;
	/* ftp���ݷ����������׽��� */
	int datskt;
	/* ��������	 */
	int type;
	/* ������ip */
	unsigned long ip;
	/* ����������˿� */
	unsigned short cmdport;
	/* ���������ݶ˿� */
	unsigned short datport;
	/* ftp����������ظ� */
	char response[512];
}sftpclient_t;

#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @brief �����������������
 * 
 * @param ip ��������ip (ip��port��Ϊ�����ֽ���)
 * @param port ���Ӷ˿�
 * @param timeout ��ʱʱ��
 * 
 * @return �������
 */
HANDLE sw_ftpclient_connect( unsigned long ip, unsigned short port, int timeout );

/** 
 * @brief �Ͽ���ftp������������
 * 
 * @param hftp �������
 * @param timeout ��ʱʱ��
 */
void sw_ftpclient_disconnect( HANDLE hftp, int timeout );

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
bool sw_ftpclient_login( HANDLE hftp, char* username, char* password, int timeout );

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
int sw_ftpclient_recv_data( HANDLE hftp, char *buf, int size, int timeout );

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
int sw_ftpclient_send_data( HANDLE hftp, char *buf, int size, int timeout );

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
char* sw_ftpclient_send_command( HANDLE hftp, char *type, char* command, int timeout );


#ifdef __cplusplus
}
#endif
#endif /* _FTP_CLIENT_H_ */
