/** 
 * @file swtcp.h
 * @brief  TCP函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16 created
 */
 
#include "swapi.h"
#include "swos_priv.h"
#include "swtcp.h"

/** 
 * @brief 创建TCP套接字
 * 
 * @return 成功,返回创建的sokcet; 否则,返回-1
 */
int sw_tcp_socket()
{
	return socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
}

/** 
 * @brief 关闭套接字
 * 
 * @param skt 创建的套接字
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
 * @brief 切断连接
 * 
 * @param skt 创建的套接字
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
 * @brief 连接远端
 * 
 * @param skt 套接字
 * @param ip 远端ip地址(网络字节序)
 * @param port 远端端口(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
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
 * @brief 绑定本地接收地址和端口
 * 
 * @param skt 套接字
 * @param ip 本地ip地址(网络字节序)
 * @param port 本地端口(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
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
 * @brief 开始监听 
 * 
 * @param skt 套接字
 * @param max 悬挂队列长度，建议设为5
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_tcp_listen( int skt, int max )
{
	return listen( skt, max );
}

/** 
 * @brief 等待远程连接
 * 
 * @param skt 套接字
 * @param ip 等待接收到的远程ip地址
 * @param port 等待接收到的远程端口
 * 
 * @return 成功,返回新的套接字; 否则,返回-1
 */
int sw_tcp_accept( int skt, unsigned long *ip, unsigned short *port )
{
	struct sockaddr_in from;
	int slen = sizeof(from);

	/*等待CLIENT的连接*/
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
 * @brief 发送数据
 * 
 * @param skt 套接字
 * @param buf 发送数据的缓冲区
 * @param size 发送数据缓冲区的大小
 * 
 * @return 成功,返回实际发送的字节数; 否则,返回-1
 */
int sw_tcp_send( int skt, char *buf, int size )
{
	return send( skt, buf, size, 0 );
}

/** 
 * @brief 接收数据
 * 
 * @param skt 套接字
 * @param buf 接收数据的缓冲区
 * @param size 接收数据缓冲区的大小
 * 
 * @return 成功,返回实际接收的字节数; 否则,返回-1
 */
int sw_tcp_recv( int skt, char *buf, int size )
{
	return recv( skt, buf, size, 0 );
}

/** 
 * @brief 配置套接字的类型
 * 
 * @param skt 套接字
 * @param type 套接字的类型
 * @param val 指向的数值,通常取值范围为(0,1)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_tcp_ioctl( int skt, int type, unsigned long *val )
{
	return ioctl( skt, (long)type, val );
}

/** 
 * @brief 检测套接字状态的变化 
 * 
 * @param skt 套接字
 * @param readfds 读描述词组
 * @param writefds 写描述词组
 * @param exceptfds 异常描述词组
 * @param timeout 等待的超时时间(ms)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
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

/***************增加ipv6的tcp socket接口*****************/
/** 
 * @brief 创建IPv6 TCP套接字
 * 
 * @return 成功,返回创建的sokcet; 否则,返回-1
 */
int sw_tcp_socket_v6()
{
	return socket( AF_INET6, SOCK_STREAM, IPPROTO_TCP );
}

/** 
 * @brief 连接远端
 * 
 * @param skt 套接字
 * @param ip 远端ip地址(网络字节序)
 * @param port 远端端口(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
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
 * @brief 绑定本地接收地址和端口
 * 
 * @param skt 套接字
 * @param ip 本地ip地址(网络字节序)
 * @param port 本地端口(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
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
 * @brief 等待远程连接
 * 
 * @param skt 套接字
 * @param ip 等待接收到的远程ip地址
 * @param port 等待接收到的远程端口
 * 
 * @return 成功,返回新的套接字; 否则,返回-1
 */
int sw_tcp_accept_v6( int skt, struct in6_addr *ip, unsigned short *port )
{
	struct sockaddr_in6 from;
	int slen = sizeof(from);

	/*等待CLIENT的连接*/
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
