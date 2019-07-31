/** 
 * @file swtcp.h
 * @brief  TCP�����ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16 created
 */

#ifndef __SWTCP_H__
#define __SWTCP_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "swapi.h"

/** 
 * @brief ����TCP�׽���
 * 
 * @return �ɹ�,���ش�����sokcet; ����,����-1
 */
int sw_tcp_socket();
int sw_tcp_socket_v6();

/** 
 * @brief �ر��׽���
 * 
 * @param skt �������׽���
 */
void sw_tcp_close( int skt );

void sw_tcp_shutdown(int skt);

/** 
 * @brief ����Զ��
 * 
 * @param skt �׽���
 * @param ip Զ��ip��ַ(�����ֽ���)
 * @param port Զ�˶˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_connect( int skt, unsigned long ip, unsigned short port );
int sw_tcp_connect_v6( int skt, struct in6_addr ip, unsigned short port );

/** 
 * @brief �󶨱��ؽ��յ�ַ�Ͷ˿�
 * 
 * @param skt �׽���
 * @param ip ����ip��ַ(�����ֽ���)
 * @param port ���ض˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_bind( int skt, unsigned long ip, unsigned short port );
int sw_tcp_bind_v6( int skt, struct in6_addr ip, unsigned short port );

/*  */
/** 
 * @brief ��ʼ���� 
 * 
 * @param skt �׽���
 * @param max ���Ҷ��г��ȣ�������Ϊ5
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_listen( int skt, int max );

/** 
 * @brief �ȴ�Զ������
 * 
 * @param skt �׽���
 * @param ip �ȴ����յ���Զ��ip��ַ
 * @param port �ȴ����յ���Զ�̶˿�
 * 
 * @return �ɹ�,�����µ��׽���; ����,����-1
 */
int sw_tcp_accept( int skt, unsigned long *ip, unsigned short *port );
int sw_tcp_accept_v6( int skt, struct in6_addr *ip, unsigned short *port );

/** 
 * @brief ��������
 * 
 * @param skt �׽���
 * @param buf �������ݵĻ�����
 * @param size �������ݻ������Ĵ�С
 * 
 * @return �ɹ�,����ʵ�ʷ��͵��ֽ���; ����,����-1
 */
int sw_tcp_send( int skt, char *buf, int size );

/** 
 * @brief ��������
 * 
 * @param skt �׽���
 * @param buf �������ݵĻ�����
 * @param size �������ݻ������Ĵ�С
 * 
 * @return �ɹ�,����ʵ�ʽ��յ��ֽ���; ����,����-1
 */
int sw_tcp_recv( int skt, char *buf, int size );

/** 
 * @brief �����׽��ֵ�����
 * 
 * @param skt �׽���
 * @param type �׽��ֵ�����
 * @param val ָ�����ֵ,ͨ��ȡֵ��ΧΪ(0,1)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_ioctl( int skt, int type, unsigned long *val );

/** 
 * @brief ����׽���״̬�ı仯 
 * 
 * @param skt �׽���
 * @param readfds ����������
 * @param writefds д��������
 * @param exceptfds �쳣��������
 * @param timeout �ȴ��ĳ�ʱʱ��(ms)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_select( int skt, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, int timeout );


#ifdef __cplusplus
}
#endif

#endif /*__SWTCP_H__*/
