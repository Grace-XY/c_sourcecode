/** 
 * @file swudp.h
 * @brief UDP�����ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16 created
 */

#ifndef __SWUDP_H__
#define __SWUDP_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "swapi.h"
/** 
 * @brief ����UDP�׽���
 * 
 * @return �ɹ�,���ش������׽���; ����,����-1
 */
int sw_udp_socket();
int sw_udp_socket_v6();

/** 
 * @brief �ر��׽���
 * 
 * @param skt �������׽���
 */
void sw_udp_close( int skt );

/** 
 * @brief �󶨱��ص�ַ�Ͷ˿�
 * 
 * @param skt UDP�׽���
 * @param ip ���ص�ip��ַ(�����ֽ���)
 * @param port ���ض˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_bind( int skt, unsigned long ip, unsigned short port );
int sw_udp_bind_v6( int skt, struct in6_addr ip, unsigned short port );

/** 
 * @brief �����鲥��
 * 
 * @param skt �׽���
 * @param ip �����鲥��ip��ַ(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_join( int skt, unsigned long ip );
int sw_udp_join_v6( int skt, struct in6_addr ip );

/** 
 * @brief �˳��鲥��
 * 
 * @param skt �׽���
 * @param ip �˳��鲥���ip��ַ
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_drop( int skt, unsigned long ip );
int sw_udp_drop_v6( int skt, struct in6_addr ip );

/** 
 * @brief ��������
 * 
 * @param skt �׽���
 * @param ip Զ�˵�ip��ַ(�����ֽ���)
 * @param port Զ�˵Ķ˿�(�����ֽ���)
 * @param buf �������ݵĻ�����
 * @param size �������ݻ������Ĵ�С
 * 
 * @return �ɹ�,����ʵ�ʷ��͵��ֽ���; ����,����-1
 */
int sw_udp_send( int skt, unsigned long ip, unsigned short port, char *buf, int size );
int sw_udp_send_v6( int skt, struct in6_addr ip, unsigned short port, char *buf, int size );

/** 
 * @brief ��������
 * 
 * @param skt �׽���
 * @param ip ���յ���Զ��ip
 * @param port ���յ���Զ�˶˿�
 * @param buf �������ݵĻ�����
 * @param size �������ݻ������Ĵ�С
 * 
 * @return �ɹ�,����ʵ�ʽ��յ��ֽ���; ����,����-1
 */
int sw_udp_recv( int skt, unsigned long *ip, unsigned short *port, char *buf, int size );
int sw_udp_recv_v6( int skt, struct in6_addr *ip, unsigned short *port, char *buf, int size );

/** 
 * @brief �����׽��ֵ�����
 * 
 * @param skt �׽���
 * @param type �׽��ֵ�����
 * @param val ָ�����ֵ,ͨ��ȡֵ��ΧΪ(0,1)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_ioctl( int skt, int type, unsigned long *val );

/** 
 * @brief ����׽���״̬�ı仯
 * 
 * @param skt �׽���
 * @param readfds ����������
 * @param writefds д��������
 * @param exceptfds ������������
 * @param timeout �ȴ��ĳ�ʱʱ��(ms)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_select( int skt, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, int timeout );


#ifdef __cplusplus
}
#endif

#endif /*__SWUDP_H__*/
