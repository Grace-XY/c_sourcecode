/** 
 * @file swftpfile.h
 * @brief FTP���������ļ�
 * @author ...
 * @date 2007-09-06
 */


#ifndef __FTP_FILE_H__
#define __FTP_FILE_H__

#ifdef __cplusplus
extern "C"
{
#endif 

/** 
 * @brief ��ʼ��
 * 
 * @param url ָ���ļ����ص�URL
 * @param timeout ��ʱʱ��
 * 
 * @return �������,�ɹ�; ʧ��ΪNULL
 */
HANDLE sw_ftpfile_priv_init( char *url, int timeout );

/** 
 * @brief �˳�
 * 
 * @param hfile �������
 * @param timeout ��ʱʱ��
 */
void sw_ftpfile_priv_exit( HANDLE hfile, int timeout );

/** 
 * @brief ��ӡ��Ϣ
 */
void sw_ftpfile_priv_print();

/** 
 * @brief ȡ��ftpclient���
 * 
 * @param hfile 
 * 
 * @return ftpclient���
 */
HANDLE sw_ftpfile_priv_get_client( HANDLE hfile );

/** 
 * @brief ��ȡ�����ļ��Ĵ�С
 * 
 * @param hfile �������
 * @param timeout ��ʱʱ��
 * 
 * @return �ļ��Ĵ�С,�ɹ�;����, -1
 */
int sw_ftpfile_priv_get_size( HANDLE hfile, int timeout );

/** 
 * @brief �����ļ�
 * 
 * @param hfile �������
 * @param buf ָ��洢�ļ��Ļ�����
 * @param size ��������С
 * @param timeout ��ʱʱ��
 * 
 * @return ��ȡ���ݵĴ�С,�ɹ�; -1,ʧ��    //ԭ���ǣ�return 0,�ɹ�; -1,ʧ��
 */
int sw_ftpfile_priv_get_file( HANDLE hfile, char *buf, int size, int timeout );

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
int sw_ftpfile_priv_upload_file( HANDLE hfile, char *buf, int size, int timeout );



#ifdef __cplusplus
}
#endif

#endif /*__FTP_FILE_H__*/
