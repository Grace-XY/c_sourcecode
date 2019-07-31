/** 
 * @file swtcp.h
 * @brief  TCP�����ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16 created
 */
 
#include "swapi.h"
#include "swos_priv.h"
#include "swtcp.h"

/** 
 * @brief ����TCP�׽���
 * 
 * @return �ɹ�,���ش�����sokcet; ����,����-1
 */
int sw_tcp_socket()
{
	return socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
}

/** 
 * @brief �ر��׽���
 * 
 * @param skt �������׽���
 */
void sw_tcp_close( int skt )
{
	if( skt != -1 )
	{
		close( skt );	
	}

	return;
}

/** 
 * @brief �ж�����
 * 
 * @param skt �������׽���
 */
void sw_tcp_shutdown( int skt )
{
	if( skt != -1 )
	{
		shutdown( skt, 2 );	
	}

	return;
}
/** 
 * @brief ����Զ��
 * 
 * @param skt �׽���
 * @param ip Զ��ip��ַ(�����ֽ���)
 * @param port Զ�˶˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_connect( int skt, unsigned long ip, unsigned short port )
{
	struct sockaddr_in addr;

	memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip;
	addr.sin_port = port;
	
	return connect( skt, (struct sockaddr *)&addr, sizeof(addr) );
}

/** 
 * @brief �󶨱��ؽ��յ�ַ�Ͷ˿�
 * 
 * @param skt �׽���
 * @param ip ����ip��ַ(�����ֽ���)
 * @param port ���ض˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_bind( int skt, unsigned long ip, unsigned short port )
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip;
	addr.sin_port = port;
	
	return bind( skt, (struct sockaddr*)&addr, sizeof(addr) );
}

/** 
 * @brief ��ʼ���� 
 * 
 * @param skt �׽���
 * @param max ���Ҷ��г��ȣ�������Ϊ5
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_listen( int skt, int max )
{
	return listen( skt, max );
}

/** 
 * @brief �ȴ�Զ������
 * 
 * @param skt �׽���
 * @param ip �ȴ����յ���Զ��ip��ַ
 * @param port �ȴ����յ���Զ�̶˿�
 * 
 * @return �ɹ�,�����µ��׽���; ����,����-1
 */
int sw_tcp_accept( int skt, unsigned long *ip, unsigned short *port )
{
	struct sockaddr_in from;
	int slen = sizeof(from);

	/*�ȴ�CLIENT������*/
	skt = accept( skt, (struct sockaddr *)&from, &slen );
	if( 0 <= skt )
	{
		if( ip )
		{
			*ip = from.sin_addr.s_addr;
		}
		if( port )
		{
			*port = from.sin_port;
		}
	}

	return skt;
}

/** 
 * @brief ��������
 * 
 * @param skt �׽���
 * @param buf �������ݵĻ�����
 * @param size �������ݻ������Ĵ�С
 * 
 * @return �ɹ�,����ʵ�ʷ��͵��ֽ���; ����,����-1
 */
int sw_tcp_send( int skt, char *buf, int size )
{
	return send( skt, buf, size, 0 );
}

/** 
 * @brief ��������
 * 
 * @param skt �׽���
 * @param buf �������ݵĻ�����
 * @param size �������ݻ������Ĵ�С
 * 
 * @return �ɹ�,����ʵ�ʽ��յ��ֽ���; ����,����-1
 */
int sw_tcp_recv( int skt, char *buf, int size )
{
	return recv( skt, buf, size, 0 );
}

/** 
 * @brief �����׽��ֵ�����
 * 
 * @param skt �׽���
 * @param type �׽��ֵ�����
 * @param val ָ�����ֵ,ͨ��ȡֵ��ΧΪ(0,1)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_ioctl( int skt, int type, unsigned long *val )
{
	return ioctl( skt, (long)type, val );
}

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
int sw_tcp_select( int skt, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, int timeout )
{
	struct timeval tv;

	if( readfds )
	{
		FD_ZERO( readfds );
		FD_SET( (unsigned int)skt, readfds );
	}
	if( writefds )
	{
		FD_ZERO( writefds );
		FD_SET(  (unsigned int)skt, writefds );
	}
	if( exceptfds )
	{
		FD_ZERO( exceptfds );
		FD_SET( (unsigned int)skt, exceptfds );
	}

	if( 0 <= timeout )
	{
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;
	
		return select( skt+1, readfds, writefds, exceptfds, &tv );
	}
	else
	{
		return select( skt+1, readfds, writefds, exceptfds, NULL );
	}
}

/***************����ipv6��tcp socket�ӿ�*****************/
/** 
 * @brief ����IPv6 TCP�׽���
 * 
 * @return �ɹ�,���ش�����sokcet; ����,����-1
 */
int sw_tcp_socket_v6()
{
	return socket( AF_INET6, SOCK_STREAM, IPPROTO_TCP );
}

/** 
 * @brief ����Զ��
 * 
 * @param skt �׽���
 * @param ip Զ��ip��ַ(�����ֽ���)
 * @param port Զ�˶˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_connect_v6( int skt, struct in6_addr ip, unsigned short port )
{
	struct sockaddr_in6 addr;

	memset( &addr, 0, sizeof(addr) );
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = ip;
	addr.sin6_port = port;

	return connect( skt, (struct sockaddr *)&addr, sizeof(addr) );
}

/** 
 * @brief �󶨱��ؽ��յ�ַ�Ͷ˿�
 * 
 * @param skt �׽���
 * @param ip ����ip��ַ(�����ֽ���)
 * @param port ���ض˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_tcp_bind_v6( int skt, struct in6_addr ip, unsigned short port )
{
	struct sockaddr_in6 addr;

	addr.sin6_family = AF_INET6;
	addr.sin6_addr = ip;
	addr.sin6_port = port;
	
	return bind( skt, (struct sockaddr*)&addr, sizeof(addr) );
}

/** 
 * @brief �ȴ�Զ������
 * 
 * @param skt �׽���
 * @param ip �ȴ����յ���Զ��ip��ַ
 * @param port �ȴ����յ���Զ�̶˿�
 * 
 * @return �ɹ�,�����µ��׽���; ����,����-1
 */
int sw_tcp_accept_v6( int skt, struct in6_addr *ip, unsigned short *port )
{
	struct sockaddr_in6 from;
	int slen = sizeof(from);

	/*�ȴ�CLIENT������*/
	skt = accept( skt, (struct sockaddr *)&from, &slen );
	if( 0 <= skt )
	{
		if( ip )
		{
			*ip = from.sin6_addr;
		}
		if( port )
		{
			*port = from.sin6_port;
		}
	}
	return skt;
}
