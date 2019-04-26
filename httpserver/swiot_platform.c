//#include <stdio.h>
//#include <sys/socket.h>
//#include <pthread.h>
#include "swiot_platform.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/portmacro.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"

#if 1//by peter

xSemaphoreHandle xMutex = NULL;

void* SWIOT_Mutex_Create()
{
    int ret = 0;

    /* �ź���ʹ��ǰ�����ȴ���������������һ�����������͵��ź����� */
    xMutex = xSemaphoreCreateMutex();
    if( xMutex != NULL )
    {

    }

    return xMutex;
}

void SWIOT_Mutex_Destroy( void* mutex )
{
    vSemaphoreDelete( mutex );
}

void SWIOT_Mutex_Lock( void* mutex )
{
    //return pthread_mutex_lock( (pthread_mutex_t *)mutex );
    xSemaphoreTake(xMutex, portMAX_DELAY);
}

void SWIOT_Mutex_Unlock( void* mutex )
{
    //pthread_mutex_unlock( (pthread_mutex_t *)mutex );
    xSemaphoreGive(xMutex);
}

/**
 * [�����׽���]
 * @param  domain   [Э���壬����Э������AF_INET��Ҫʹ��ipv4��ַ����AF_INET6��AF_LOCAL ]
 * @param  type     [ָ��Socket����,���õ�SOCKET������SOCK_STREAM��SOCK_DGRAM��SOCK_RAW��SOCK_PACKET��SOCK_SEQPACKET]
 * @param  protocol [ָ��Э�飬����Э����IPPROTO_TCP,IPPROTO_UDP,IPPROTO_STCP,IPPROTO_TIPC����type�����������]
 * @return          [�ɹ������׽��֣����� < 0]
 */
int SWIOT_Creat_Socket(int domain,int type,int protocol)
{
    return socket(domain,type,protocol);
}

/**
 * [�����׽���]
 * @param  skt [�׽���]
 * @return     [���� < 0]
 */
int SWIOT_Close_Socket(int skt)
{
    return close(skt);
}

/**
 * [���׽���]
 * @param  skt  [��Ҫ�󶨵��׽���]
 * @param  ip   [�󶨵�Ip��ַ]
 * @param  port [�󶨵Ķ˿�]
 * @return      [�ɹ�����0]
 */
int SWIOT_Bind_Socket(int skt,unsigned int ip,unsigned short port)
{
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = SWIOT_htons(port);

    return bind(skt,(const struct sockaddr*)&addr,sizeof(addr));
}


/**
 * [��������]
 * @param  skt     [�׽���]
 * @param  backlog [�������г���]
 * @return         [�ɹ�����0 ʧ�� < 0]
 */
int SWIOT_Listen( int skt,int backlog )
{
    return listen( skt,backlog );
}

/**
 * [�����׽���ѡ��]
 * @param  skt        [�׽���]
 * @param  level      [����Э���]
 * @param  optname    [��Ҫ���ʵ�ѡ����]
 * @param  optval     [ѡ��ֵ�Ļ���]
 * @param  optval_len [ѡ��ĳ���]
 * @return            [ʧ��<0]
 */
int SWIOT_Setsocket_Opt(int skt,int level,int optname,void* optval,int optval_len)
{
    return setsockopt( skt,level,optname,optval,optval_len );
}

/**
 * [��������]
 * @param  skt  [�׽���]
 * @param  ip   [����ip]
 * @param  port [����port]
 * @return      [ʧ�� < 0 ���Ϊ������ ��errno == EINPROGRESS ��ʾ�������ڽ�����]
 */
int SWIOT_Connect( int skt,unsigned int ip,unsigned short port )
{
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port   = port;
    addr.sin_addr.s_addr = ip;

    return connect(skt,(const struct sockaddr*)&addr,sizeof( addr ));
}
int SWIOT_Fcntl(int s, int cmd, int val)
{
    return fcntl(s,cmd,val);
}


/**
 * [��������]
 * @param  skt  [�׽���]
 * @param  ip   [�Զ�ip]
 * @param  port [�Զ�port]
 * @return      [<0 ����ʧ��]
 */
int SWIOT_Accept( int skt,unsigned int *ip,unsigned short *port )
{
    int ret = -1;
    struct sockaddr_in addr = {0};
    socklen_t len = sizeof(addr);

    ret = accept( skt,(struct sockaddr*)&addr,&len );

    if( ret > 0 )
    {
        if( ip )
            *ip = addr.sin_addr.s_addr;

        if( port )
            *port = addr.sin_port;
    }

    return ret;
}


/**
 * [��������]
 * @param  skt [���ܵ��׽���]
 * @param  buf [���ܻ�����]
 * @param  len [��������С]
 * @return     [== 0 ��ʾ�Է������ж� <0 �� errno == EAGAIN(ջ�����) || errno == EINTR(��ʾϵͳ���ñ��ж�) ��ʾ���Լ����� ����Ϊ�쳣]
 */
int SWIOT_Recv( int skt,char *buf,int len )
{
    int ret = -1;

    ret = recv( skt,buf,len,0 );

    return ret;
}

/**
 * [��������]
 * @param  skt [���͵��׽���]
 * @param  buf [���ͻ�����]
 * @param  len [����������]
 * @return     [== 0 ��ʾ�Է������ж� <0 �� errno == EAGAIN(ջ�����) || errno == EINTR(��ʾϵͳ���ñ��ж�) ��ʾ���Լ���д ����Ϊ�쳣]
 */
int SWIOT_Send( int skt,char *buf,int len )
{
    int ret = -1;

    ret = send( skt,buf,len,0 );

    return ret;
}

/**
 * [����һ�����ݱ������������ݱ�Դ��ַ]
 * @param  skt       [�׽ӿ�]
 * @param  buf       [���ջ�����]
 * @param  len       [��������С]
 * @param  from_ip   [Դ��ַip]
 * @param  from_port [Դ��ַport]
 * @return           []
 */
int SWIOT_Recv_From(int skt,char* buf,int len,unsigned int* from_ip,unsigned short*from_port)
{
    struct sockaddr_in addr = {0};
    int ret = -1;
    socklen_t addr_len = sizeof( addr );

    ret = recvfrom( skt,buf,len,0,(struct sockaddr*)&addr,&addr_len );

    if( ret > 0 )
    {
        if( from_ip )
            *from_ip = addr.sin_addr.s_addr;

        if( from_port )
            *from_port = addr.sin_port;
    }

    return ret;
}

/**
 * [�������ݱ�]
 * @param  skt     [�׽ӿ�]
 * @param  buf     [���ͻ�����]
 * @param  len     [��������С]
 * @param  to_ip   [Ŀ�Ķ�ip]
 * @param  to_port [Ŀ�Ķ�port]
 * @return         [description]
 */
int SWIOT_Send_To(int skt,char* buf,int len,unsigned int to_ip,unsigned short to_port)
{
    struct sockaddr_in addr = {0};
    int ret = -1;

    addr.sin_family = AF_INET;
    addr.sin_port   = to_port;
    addr.sin_addr.s_addr = to_ip;

    ret = sendto( skt,buf,len,0,(const struct sockaddr*)&addr,sizeof(addr) );

    return ret;
}
#else

void* SWIOT_Mutex_Create()
{
    int ret = 0;

    pthread_mutex_t *mt;
    pthread_mutexattr_t pma;

    mt = (pthread_mutex_t*)malloc( sizeof(pthread_mutex_t) );
    if( mt )
    {
        memset( mt, 0, sizeof(pthread_mutex_t) );

        ret |= pthread_mutexattr_init( &pma );
        ret |= pthread_mutexattr_settype( &pma, PTHREAD_MUTEX_ERRORCHECK_NP );
        ret |= pthread_mutex_init( mt, &pma );
        ret |= pthread_mutexattr_destroy( &pma );

        //pthread_mutex_init( mt, NULL );
    }
    return mt;
}

void SWIOT_Mutex_Destroy( void* mutex )
{
    pthread_mutex_t *mt = (pthread_mutex_t *)mutex;

    pthread_mutex_destroy( mt );

    free( mt );
}

void SWIOT_Mutex_Lock( void* mutex )
{
    return pthread_mutex_lock( (pthread_mutex_t *)mutex );
}

void SWIOT_Mutex_Unlock( void* mutex )
{
    pthread_mutex_unlock( (pthread_mutex_t *)mutex );
}
/**
 * [�����׽���]
 * @param  domain   [Э���壬����Э������AF_INET��Ҫʹ��ipv4��ַ����AF_INET6��AF_LOCAL ]
 * @param  type     [ָ��Socket����,���õ�SOCKET������SOCK_STREAM��SOCK_DGRAM��SOCK_RAW��SOCK_PACKET��SOCK_SEQPACKET]
 * @param  protocol [ָ��Э�飬����Э����IPPROTO_TCP,IPPROTO_UDP,IPPROTO_STCP,IPPROTO_TIPC����type�����������]
 * @return          [�ɹ������׽��֣����� < 0]
 */
int SWIOT_Creat_Socket(int domain,int type,int protocol)
{
    return socket(domain,type,protocol);
}

/**
 * [�����׽���]
 * @param  skt [�׽���]
 * @return     [���� < 0]
 */
int SWIOT_Close_Socket(int skt)
{
    return close(skt);
}

/**
 * [���׽���]
 * @param  skt  [��Ҫ�󶨵��׽���]
 * @param  ip   [�󶨵�Ip��ַ]
 * @param  port [�󶨵Ķ˿�]
 * @return      [�ɹ�����0]
 */
int SWIOT_Bind_Socket(int skt,unsigned int ip,unsigned short port)
{
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = SWIOT_htons(port);

    return bind(skt,(const struct sockaddr*)&addr,sizeof(addr));
}


/**
 * [��������]
 * @param  skt     [�׽���]
 * @param  backlog [�������г���]
 * @return         [�ɹ�����0 ʧ�� < 0]
 */
int SWIOT_Listen( int skt,int backlog )
{
    return listen( skt,backlog );
}

/**
 * [�����׽���ѡ��]
 * @param  skt        [�׽���]
 * @param  level      [����Э���]
 * @param  optname    [��Ҫ���ʵ�ѡ����]
 * @param  optval     [ѡ��ֵ�Ļ���]
 * @param  optval_len [ѡ��ĳ���]
 * @return            [ʧ��<0]
 */
int SWIOT_Setsocket_Opt(int skt,int level,int optname,void* optval,int optval_len)
{
    return setsockopt( skt,level,optname,optval,optval_len );
}

/**
 * [��������]
 * @param  skt  [�׽���]
 * @param  ip   [����ip]
 * @param  port [����port]
 * @return      [ʧ�� < 0 ���Ϊ������ ��errno == EINPROGRESS ��ʾ�������ڽ�����]
 */
int SWIOT_Connect( int skt,unsigned int ip,unsigned short port )
{
    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port   = port;
    addr.sin_addr.s_addr = ip;

    return connect(skt,(const struct sockaddr*)&addr,sizeof( addr ));
}
int SWIOT_Fcntl(int s, int cmd, int val)
{
    return fcntl(s,cmd,val);
}


/**
 * [��������]
 * @param  skt  [�׽���]
 * @param  ip   [�Զ�ip]
 * @param  port [�Զ�port]
 * @return      [<0 ����ʧ��]
 */
int SWIOT_Accept( int skt,unsigned int *ip,unsigned short *port )
{
    int ret = -1;
    struct sockaddr_in addr = {0};
    socklen_t len = sizeof(addr);

    ret = accept( skt,(struct sockaddr*)&addr,&len );

    if( ret > 0 )
    {
        if( ip )
            *ip = addr.sin_addr.s_addr;

        if( port )
            *port = addr.sin_port;
    }

    return ret;
}


/**
 * [��������]
 * @param  skt [���ܵ��׽���]
 * @param  buf [���ܻ�����]
 * @param  len [��������С]
 * @return     [== 0 ��ʾ�Է������ж� <0 �� errno == EAGAIN(ջ�����) || errno == EINTR(��ʾϵͳ���ñ��ж�) ��ʾ���Լ����� ����Ϊ�쳣]
 */
int SWIOT_Recv( int skt,char *buf,int len )
{
    int ret = -1;

    ret = recv( skt,buf,len,0 );

    return ret;
}

/**
 * [��������]
 * @param  skt [���͵��׽���]
 * @param  buf [���ͻ�����]
 * @param  len [����������]
 * @return     [== 0 ��ʾ�Է������ж� <0 �� errno == EAGAIN(ջ�����) || errno == EINTR(��ʾϵͳ���ñ��ж�) ��ʾ���Լ���д ����Ϊ�쳣]
 */
int SWIOT_Send( int skt,char *buf,int len )
{
    int ret = -1;

    ret = send( skt,buf,len,0 );

    return ret;
}

/**
 * [����һ�����ݱ������������ݱ�Դ��ַ]
 * @param  skt       [�׽ӿ�]
 * @param  buf       [���ջ�����]
 * @param  len       [��������С]
 * @param  from_ip   [Դ��ַip]
 * @param  from_port [Դ��ַport]
 * @return           []
 */
int SWIOT_Recv_From(int skt,char* buf,int len,unsigned int* from_ip,unsigned short*from_port)
{
    struct sockaddr_in addr = {0};
    int ret = -1;
    socklen_t addr_len = sizeof( addr );

    ret = recvfrom( skt,buf,len,0,(struct sockaddr*)&addr,&addr_len );

    if( ret > 0 )
    {
        if( from_ip )
            *from_ip = addr.sin_addr.s_addr;

        if( from_port )
            *from_port = addr.sin_port;
    }

    return ret;
}

/**
 * [�������ݱ�]
 * @param  skt     [�׽ӿ�]
 * @param  buf     [���ͻ�����]
 * @param  len     [��������С]
 * @param  to_ip   [Ŀ�Ķ�ip]
 * @param  to_port [Ŀ�Ķ�port]
 * @return         [description]
 */
int SWIOT_Send_To(int skt,char* buf,int len,unsigned int to_ip,unsigned short to_port)
{
    struct sockaddr_in addr = {0};
    int ret = -1;

    addr.sin_family = AF_INET;
    addr.sin_port   = to_port;
    addr.sin_addr.s_addr = to_ip;

    ret = sendto( skt,buf,len,0,(const struct sockaddr*)&addr,sizeof(addr) );

    return ret;
}

#endif
