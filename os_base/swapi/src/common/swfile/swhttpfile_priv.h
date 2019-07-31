/** 
 * @file swhttpfile.h
 * @brief HTTP���������ļ�
 * @author ...
 * @date 2007-09-06
 */

#ifndef __SWHTTPFILE_H__
#define	__SWHTTPFILE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief ��ʼ��http��������
 * 
 * @param url 
 * @param timeout 
 * 
 * @return httpfile���,�ɹ�;ʧ��ΪNULL
 */
HANDLE sw_httpfile_priv_init( char *url, int timeout );

/** 
 * @brief �˳�httpfile
 * 
 * @param h_file 
 * @param timeout 
 */
void sw_httpfile_priv_exit( HANDLE h_file, int timeout );

/** 
 * @brief ��ӡ��Ϣ
 */
void sw_httpfile_priv_print();

/** 
 * @brief ȡ��httpclient���
 * 
 * @param h_file 
 * 
 * @return httpclient���
 */
HANDLE sw_httpfile_priv_get_client( HANDLE h_file );

/** 
 * @brief ȡ��HTTP���������ļ��Ĵ�С
 * 
 * @param h_file 
 * 
 * @return 
 */
int64_t sw_httpfile_priv_get_size( HANDLE h_file );

/** 
 * @brief ��ȡ�������ļ�
 * 
 * @param h_file 
 * @param buf 
 * @param size 
 * @param timeout 
 * 
 * @return 
 */
int sw_httpfile_priv_get_file( HANDLE h_file, char *buf, int size, int timeout );

/** 
 * @brief ȡ��ʵ��ʹ�õ�URL����Ϊ�п����ض�����
 * 
 * @param h_file 
 * 
 * @return 
 */
char *sw_httpfile_priv_get_url( HANDLE h_file );

int sw_httpfile_priv_allocmem_getfile( HANDLE h_file, char **p_buf, int *p_size, int timeout );

#ifdef __cplusplus
}
#endif

#endif	/* __SWHTTPFILE_H__ */
