#ifndef __SWIOT_PLATFORM_H__
#define __SWIOT_PLATFORM_H__

#include "swiot_common.h"
#if 0
#include <errno.h>
#include <sys/socket.h>
#include<netinet/in.h>>
#include <pthread.h>
#include<fcntl.h>
#endif

//#include<unistd.h>

#define SWIOT_Malloc   malloc
#define SWIOT_Memset   memset
#define SWIOT_Free     free
#define SWIOT_htons(x) htons(x)

#define SWIOT_Thread_Create( handle,callTask,threadName,stack_size,parameters, priority ) ((0==pthread_create(&handle, NULL, callTask, parameters))?1:0)
//#define SWIOT_Thread_Destroy(handle) (pthread_join(handle,NULL))

//#define SWIOT_msleep(time)    os_//usleep((time) * 1000)

//#define SWIOT_Get_Time()    time(NULL)
#define SWIOT_Inet_Addr     inet_addr
#define SWIOT_Select        select
//#define SWIOT_GethostByName gethostbyname

void* SWIOT_Mutex_Create();
void SWIOT_Mutex_Destroy( void* mutex );
void SWIOT_Mutex_Lock( void* mutex );
void SWIOT_Mutex_Unlock( void* mutex );

/**
 * [�����׽���]
 * @param  domain   [Э���壬����Э������AF_INET��Ҫʹ��ipv4��ַ����AF_INET6��AF_LOCAL ]
 * @param  type     [ָ��Socket����,���õ�SOCKET������SOCK_STREAM��SOCK_DGRAM��SOCK_RAW��SOCK_PACKET��SOCK_SEQPACKET]
 * @param  protocol [ָ��Э�飬����Э����IPPROTO_TCP,IPPROTO_UDP,IPPROTO_STCP,IPPROTO_TIPC����type�����������]
 * @return          [�ɹ������׽��֣����� < 0]
 */
int SWIOT_Creat_Socket(int domain,int type,int protocol);

/**
 * [�����׽���]
 * @param  skt [�׽���]
 * @return     [���� < 0]
 */
int SWIOT_Close_Socket(int skt);

/**
 * [���׽���]
 * @param  skt  [��Ҫ�󶨵��׽���]
 * @param  ip   [�󶨵�Ip��ַ]
 * @param  port [�󶨵Ķ˿�]
 * @return      [�ɹ�����0]
 */
int SWIOT_Bind_Socket(int skt,unsigned int ip,unsigned short port);

/**
 * [�����׽���ѡ��]
 * @param  skt        [�׽���]
 * @param  level      [����Э���]
 * @param  optname    [��Ҫ���ʵ�ѡ����]
 * @param  optval     [ѡ��ֵ�Ļ���]
 * @param  optval_len [ѡ��ĳ���]
 * @return            [ʧ��<0]
 */
int SWIOT_Setsocket_Opt(int skt,int level,int optname,void* optval,int optval_len);


/**
 * [��������]
 * @param  skt  [�׽���]
 * @param  ip   [����ip]
 * @param  port [����port]
 * @return      [ʧ�� < 0 ���Ϊ������ ��errno == EINPROGRESS ��ʾ�������ڽ�����]
 */
int SWIOT_Connect( int skt,unsigned int ip,unsigned short port );


int SWIOT_Fcntl(int s, int cmd, int val);

/**
 * [��������]
 * @param  skt  [�׽���]
 * @param  ip   [�Զ�ip]
 * @param  port [�Զ�port]
 * @return      [<0 ����ʧ��]
 */
int SWIOT_Accept( int skt,unsigned int *ip,unsigned short *port );


/**
 * [��������]
 * @param  skt [���ܵ��׽���]
 * @param  buf [���ܻ�����]
 * @param  len [��������С]
 * @return     [== 0 ��ʾ�Է������ж� <0 �� errno == EAGAIN(ջ�����) || errno == EINTR(��ʾϵͳ���ñ��ж�) ��ʾ���Լ����� ����Ϊ�쳣]
 */
int SWIOT_Recv( int skt,char *buf,int len );

/**
 * [��������]
 * @param  skt [���͵��׽���]
 * @param  buf [���ͻ�����]
 * @param  len [����������]
 * @return     [== 0 ��ʾ�Է������ж� <0 �� errno == EAGAIN(ջ�����) || errno == EINTR(��ʾϵͳ���ñ��ж�) ��ʾ���Լ���д ����Ϊ�쳣]
 */
int SWIOT_Send( int skt,char *buf,int len );


/**
 * [����һ�����ݱ������������ݱ�Դ��ַ]
 * @param  skt       [�׽ӿ�]
 * @param  buf       [���ջ�����]
 * @param  len       [��������С]
 * @param  from_ip   [Դ��ַip]
 * @param  from_port [Դ��ַport]
 * @return           []
 */
int SWIOT_Recv_From(int skt,char* buf,int len,unsigned int* from_ip,unsigned short*from_port);

/**
 * [�������ݱ�]
 * @param  skt     [�׽ӿ�]
 * @param  buf     [���ͻ�����]
 * @param  len     [��������С]
 * @param  to_ip   [Ŀ�Ķ�ip]
 * @param  to_port [Ŀ�Ķ�port]
 * @return         [description]
 */
int SWIOT_Send_To(int skt,char* buf,int len,unsigned int to_ip,unsigned short to_port);


#endif //__SWIOT_PLATFORM_H__
