/**
* @file swudp.c
* @brief UDP�����ӿ�
* @author huanghuaming
* @history
*           2004-04-28 huanghuaming created
*           2010-07-07 qushihui modified
*/

#include "swapi.h"
#include "swos_priv.h"
#include "swudp.h"

/** 
 * @brief ����UDP�׽���
 * 
 * @return �ɹ�,���ش������׽���; ����,����-1
 */
int sw_udp_socket()
{
	return socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
}

/** 
 * @brief �ر��׽���
 * 
 * @param skt �������׽���
 */
void sw_udp_close( int skt )
{
	if( skt != -1 )
	{
		close( skt );
	}

	return;
}

/** 
 * @brief �󶨱��ص�ַ�Ͷ˿�
 * 
 * @param skt UDP�׽���
 * @param ip ���ص�ip��ַ(�����ֽ���)
 * @param port ���ض˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_bind( int skt, unsigned long ip, unsigned short port )
{
	struct sockaddr_in local;

	/* �󶨱��ص�ַ�Ͷ˿� */
	memset( &local, 0, sizeof(local) );
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = ip;
	local.sin_port = port;

	if( bind ( skt, (struct sockaddr *)&local,	sizeof(local) ) < 0 )
	{
		OS_LOG_DEBUG( "cannot bind\n" );
		return -1;
	}

	return 0;
}

/** 
 * @brief �����鲥��
 * 
 * @param skt �׽���
 * @param ip �����鲥��ip��ַ(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_join( int skt, unsigned long ip )
{
	int ttl = 127;
	bool flag = true;
	struct ip_mreq mreq;

	/* ����ಥ�� */
	if( IS_MULTICAST_IP(ip) )
	{
		memset( &mreq, 0, sizeof(mreq) );
		mreq.imr_multiaddr.s_addr = ip;
		mreq.imr_interface.s_addr = INADDR_ANY;

 		if( setsockopt( skt, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) < 0 )
		{
			OS_LOG_DEBUG( "cannot add to multicast\n" );
			return -1;
		}
		if( setsockopt( skt, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast args. \n" );
		}
		if( setsockopt( skt, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&flag, sizeof(flag) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast ttl args. \n" );
		}
	}
	else
	{
		OS_LOG_DEBUG("not a multicast ip\n");
		return -1;
	}

	return 0;
}

/** 
 * @brief �˳��鲥��
 * 
 * @param skt �׽���
 * @param ip �˳��鲥���ip��ַ
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_drop( int skt, unsigned long ip )
{
	/* ���������ַ���鲥��ַ��Χ�ڣ����˳������鲥 */
	if( IS_MULTICAST_IP(ip) )
	{
		struct ip_mreq mreq;

		memset( &mreq, 0, sizeof(mreq) );
		mreq.imr_multiaddr.s_addr = ip;
		mreq.imr_interface.s_addr = INADDR_ANY;
		if( setsockopt( skt, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) <0 )
		{
			OS_LOG_DEBUG( "cannot drop from multicast\n" );
			return -1;
		}
	}

	return  0;
}

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
int sw_udp_send( int skt, unsigned long ip, unsigned short port, char *buf, int size )
{
	struct sockaddr_in sn;
	unsigned int slen = sizeof(sn);

	memset( &sn, 0, sizeof(sn) );
	slen = sizeof(sn);
	sn.sin_family = AF_INET;
	sn.sin_addr.s_addr = ip;
	sn.sin_port=  port;

	return sendto( skt, buf, size, 0, (struct sockaddr *)&sn, slen );
}

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
int sw_udp_recv( int skt, unsigned long *ip, unsigned short *port, char *buf, int size )
{
	struct sockaddr_in from;
	int slen = sizeof( from );

	memset( &from, 0, sizeof(from) );
	slen = recvfrom( skt, buf, size, 0, (struct sockaddr *)&from, &slen );

	if( ip )
	{
	   	*ip = from.sin_addr.s_addr;
	}
	if( port )
	{
		*port = from.sin_port;
	}

	return slen;
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
int sw_udp_ioctl( int skt, int type, unsigned long *val )
{
	return ioctl( skt, (long)type, val );
}

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
int sw_udp_select( int skt, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, int timeout )
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
		FD_SET( (unsigned int)skt, writefds );
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


/** 
 * @brief ����UDP�׽���
 * 
 * @return �ɹ�,���ش������׽���; ����,����-1
 */
int sw_udp_socket_v6()
{
	return socket( AF_INET6, SOCK_DGRAM, IPPROTO_UDP );
}

/** 
 * @brief �󶨱��ص�ַ�Ͷ˿�
 * 
 * @param skt UDP�׽���
 * @param ip ���ص�ip��ַ(�����ֽ���)
 * @param port ���ض˿�(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_bind_v6( int skt, struct in6_addr ip, unsigned short port )
{
	struct sockaddr_in6 local;

	/* �󶨱��ص�ַ�Ͷ˿� */
	memset( &local, 0, sizeof(local) );
	local.sin6_family = AF_INET6;
	local.sin6_addr = ip;
	local.sin6_port = port;

	if( bind ( skt, (struct sockaddr *)&local,	sizeof(local) ) < 0 )
	{
		OS_LOG_DEBUG( "cannot bind\n" );
		return -1;
	}

	return 0;
}

/** 
 * @brief �����鲥��
 * 
 * @param skt �׽���
 * @param ip �����鲥��ip��ַ(�����ֽ���)
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_join_v6( int skt, struct in6_addr ip )
{
	int ttl = 127;
	bool flag = true;
	struct ipv6_mreq mreq;

	/* ����ಥ�� */
	if( IS_MULTICAST_IPV6(ip) )
	{
		memset( &mreq, 0, sizeof(mreq) );
		mreq.ipv6mr_multiaddr = ip;
		mreq.ipv6mr_interface = 0;

 		if( setsockopt( skt, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) < 0 )
		{
			OS_LOG_DEBUG( "cannot add to multicast\n" );
			return -1;
		}
		if( setsockopt( skt, IPPROTO_IPV6, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast args. \n" );
		}
		if( setsockopt( skt, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (char *)&flag, sizeof(flag) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast ttl args. \n" );
		}
	}
	else
	{
		OS_LOG_DEBUG("not a multicast ip\n");
		return -1;
	}

	return 0;
}

/** 
 * @brief �˳��鲥��
 * 
 * @param skt �׽���
 * @param ip �˳��鲥���ip��ַ
 * 
 * @return ʧ��,���ظ���(С��0); �ɹ�,��������ֵ
 */
int sw_udp_drop_v6( int skt, struct in6_addr ip )
{
	/* ���������ַ���鲥��ַ��Χ�ڣ����˳������鲥 */
	if( IS_MULTICAST_IPV6(ip) )
	{
		struct ipv6_mreq mreq;

		memset( &mreq, 0, sizeof(mreq) );
		mreq.ipv6mr_multiaddr = ip;
		mreq.ipv6mr_interface = 0;
		if( setsockopt( skt, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) <0 )
		{
			OS_LOG_DEBUG( "cannot drop from multicast\n" );
			return -1;
		}
	}

	return  0;
}

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
int sw_udp_send_v6( int skt, struct in6_addr ip, unsigned short port, char *buf, int size )
{
	struct sockaddr_in6 sn;
	unsigned int slen = sizeof(sn);

	memset( &sn, 0, sizeof(sn) );
	slen = sizeof(sn);
	sn.sin6_family = AF_INET6;
	sn.sin6_addr = ip;
	sn.sin6_port=  port;

	return sendto( skt, buf, size, 0, (struct sockaddr *)&sn, slen );
}

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
int sw_udp_recv_v6( int skt, struct in6_addr *ip, unsigned short *port, char *buf, int size )
{
	struct sockaddr_in6 from;
	int slen = sizeof( from );

	memset( &from, 0, sizeof(from) );
	slen = recvfrom( skt, buf, size, 0, (struct sockaddr *)&from, &slen );

	if( ip )
	{
	   	*ip = from.sin6_addr;
	}
	if( port )
	{
		*port = from.sin6_port;
	}

	return slen;
}

