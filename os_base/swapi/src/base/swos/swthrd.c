/**
* @file swthrd.c
* @brief �̺߳����ӿ�
* @author lijian / huanghuaming / chenkai
* @history
*         2005-09-16 created  
*         2010-07-08 qushihui modified
*/

#include "swapi.h"
#include "swos_priv.h"
#include "swthrd.h"
#include "swmutex.h"

#if (defined(ANDROID) && defined(SUPPORT_SWAPI30))
#include "swjni.h"
#endif

#ifndef WIN32
#include <sys/times.h>
#include <sys/syscall.h>
#endif

#define MAX_THRD_NUM	64           //����߳���
#define swos_malloc malloc
#define swos_free free

#ifdef ANDROID
#define __sched_priority sched_priority
#endif
#ifdef WIN32
#define __sched_priority sched_priority
#endif

#ifdef ANDROID
#include <sys/prctl.h>
#endif

typedef struct sthrdinfo_t
{
	/* �߳�ID */
	pthread_t tid;
	/* �߳����Զ��� */
	pthread_attr_t attr;
	/* ���߳�PID */
	pid_t ppid;
	/* �߳�PID */
	pid_t pid;
	/*�̵߳��Ȳ���*/
	int policy;
	/* �̵߳����ȼ� */
	int priority;
	/* �źŵ� */
	sem_t sem;
	/* �̻߳ص����� */
	threadhandler_t handler;
	/* �ص��������� */
	unsigned long wparam;
	unsigned long lparam;	
	
	/* �Ƿ���ͣ */
	int is_pause;
	/* ��ʾ��ǰ�߳��Ƿ��˳� */
	int is_exit;
	/* �������� */
	char name[32];
}sthrdinfo_t;

static sthrdinfo_t *m_all[MAX_THRD_NUM];			//�߳���Ϣ
static int m_ref = -1;								//�߳���
static HANDLE m_mutex = NULL; 						//������
static int m_global_policy = -1;					//���Ȳ���

static void* sema_thread_handler( void* lp_param );	//�̻߳ص�����

/** 
 * @brief �����߳�,�򿪺�����ͣ״̬,priority=0(���),priority=255(���)
 * 
 * @param name 		�������߳���
 * @param priority 	�߳����ȼ�
 * @param policy 	���Ȳ��� sw_sched_t
 * @param stack_size ��ջ�Ĵ�С
 * @param handler 	ָ��ص�������ָ��
 * @param wparam 	�ص������ĵ�һ������
 * @param lparam 	�ص������ĵڶ�������
 * 
 * @return �ɹ�,�����߳̾��;ʧ��,���ؿ�ֵ
 */
HANDLE sw_thrd_open( char *name, unsigned char priority, sw_policy_t policy, int stack_size, 
					threadhandler_t handler, uint32_t wparam, uint32_t lparam )
{
	int i, rc;
	pthread_attr_t *pattr = NULL;
	sthrdinfo_t *info = NULL;
	struct sched_param param;
	int max_priority;
	int min_priority;

	if( m_ref < 0 )
	{
		memset( m_all, 0, sizeof(m_all) );
		m_ref = 0;
		m_mutex = sw_mutex_create();
	}

	if( m_mutex )
	{
		sw_mutex_lock( m_mutex );
	}
	/* ����Լ��˳����߳�, �ͷ���Դ */
	if( m_ref > MAX_THRD_NUM - 2 )
	{	
		for( i=0; i < MAX_THRD_NUM; i++ )
		{
#ifdef WIN32
            //ATTENTION m_all[i]->tid != 0��mingw�±��벻��, mingw��pthread_t����Ϊ�ṹ��
			if( m_all[i] && *((int *)&m_all[i]->tid) != 0 && m_all[i]->handler == NULL )
#else
			if( m_all[i] &&  m_all[i]->tid  != 0 && m_all[i]->handler == NULL )
#endif
			{
				OS_LOG_DEBUG( "Release self-exit thread<%s:%x>\n", m_all[i]->name, (unsigned int)&m_all[i] );			
				pthread_attr_destroy( &m_all[i]->attr );
				sem_destroy( &m_all[i]->sem );
				memset( m_all[i], 0, sizeof(sthrdinfo_t) );
				swos_free( m_all[i] );
				m_all[i] = NULL;
				m_ref--;
			}
		}
	}
	
	if( m_ref >= MAX_THRD_NUM )
	{
		OS_LOG_ERROR( "Thread num reach max(%d)!!\n", MAX_THRD_NUM );
		sw_mutex_unlock( m_mutex );

		return NULL;
	}
	info = (sthrdinfo_t*)swos_malloc( sizeof(sthrdinfo_t) );
	
	if( info == NULL )
	{
		OS_LOG_DEBUG( "error: sw_thrd_open() failed\n" );
		if( m_mutex )
		{
			sw_mutex_unlock( m_mutex );
		}
		
		return NULL;
	}
	
	memset( info, 0, sizeof(sthrdinfo_t) );
	info->handler = handler;
	info->wparam = wparam;
	info->lparam = lparam;
	info->policy = policy;
	info->priority = (int)priority;
	info->is_pause = 1;

	if( m_global_policy != -1 )
	{
		info->policy = m_global_policy;
	}
	
	strlcpy( info->name, name, sizeof(info->name) );
	
	if( info->policy < SW_SCHED_NORMAL || info->policy > SW_SCHED_RR )
	{
		info->policy = SW_SCHED_NORMAL;
	}
	/* ��ʼ���߳����Զ��� */
	pattr = &( info->attr );
	pthread_attr_init( pattr );

	//�����̶߳�ջ��С
	if( stack_size < 65536 )
	{
		stack_size = 65536;
	}

	rc = pthread_attr_setstacksize( pattr, stack_size );
	if( rc != 0 )
	{
		unsigned int defsize = 0;

		pthread_attr_getstacksize( pattr, &defsize );
		OS_LOG_ERROR( "sw_thrd_open [%s]: set stack size failed! %d, using default size %d\n", name, stack_size, defsize );
	}
	
	pthread_attr_setdetachstate( pattr, PTHREAD_CREATE_DETACHED );
	
	/*���õ��Ȳ���*/
	pthread_attr_setschedpolicy( pattr, info->policy /* SCHED_RR */ );

	/*��linuxϵͳ��, �߳����ȼ�ȡֵ��ΧΪ1~99, 1���, 99���*/
	param.__sched_priority = 1;
	max_priority = sched_get_priority_max( info->policy );
	min_priority = sched_get_priority_min( info->policy );

	priority = max_priority - ( priority * ( max_priority - min_priority ) ) / 255;
	param.__sched_priority = priority;

	/* �����źŵ� */
	if( sem_init( &( info->sem ), 0, 0 ) == -1 )
	{
		if( pattr )
		{
			pthread_attr_destroy( pattr );
		}
		goto ERROR_EXIT;
	}

	/* ��ʼ�����߳� */
	if( pthread_create( &(info->tid), pattr, sema_thread_handler, (void*)info ) != 0 )
	{
		if( pattr )
		{
			pthread_attr_destroy( pattr );
		}
		sem_destroy( &(info->sem) );
		goto ERROR_EXIT;
	}

	//�߳�����ʱ�������ȼ�
	pthread_setschedparam( info->tid, info->policy, &param );

	/* �����߳̾�� */
	for( i=0; i < MAX_THRD_NUM; i++ )
	{
		if( m_all[i] == NULL )
		{
			m_all[i] = info;
			m_ref++;
			break;
		}
	}
	OS_LOG_DEBUG( "Thread-<%s : %d : %x> opened\n", info->name,i,(unsigned int)info);

	if( m_mutex )
	{
		sw_mutex_unlock( m_mutex );
	}

	return info;

ERROR_EXIT:
	info->handler = NULL;
    memset(&info->tid, 0, sizeof(info->tid));
	if( m_mutex )
	{
		sw_mutex_unlock( m_mutex );
	}
	OS_LOG_DEBUG( "error: sw_thrd_open() failed !\n");
	
	return NULL;
}

/** 
 * @brief �ر��߳�
 * 
 * @param thrd �߳̾��
 * @param ms �ȴ���ʱ��(��λΪ����)
 */
void sw_thrd_close( HANDLE thrd, int ms )
{
	int i = 0;
	int status = 0;
	void *result;
	sthrdinfo_t *info = (sthrdinfo_t *)thrd;

	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!\n" );
		return;
	}

	if( info == NULL )
	{
		return;
	}

	info->is_exit = 1;
	sem_post( &(info->sem) );

	OS_LOG_DEBUG( "Thread-<%s : %d : %x> closing [%d]...\n", info->name,i,(unsigned int)info, ms );

	/* �ȴ��߳��Լ��˳� */
	while( info->handler && ms > 0 )
	{
		usleep( 10*1000 );
		ms -= 10;
	}

	if( m_mutex )
	{
		sw_mutex_lock( m_mutex );
	}

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( info == m_all[i] )
		{
			break;
		}
	}

	if( MAX_THRD_NUM <= i )
	{
		if( m_mutex )
		{
			sw_mutex_unlock( m_mutex );	
		}

		return;
	}

	if( info->handler )
	{
#ifdef ANDROID
		/* ǿ���˳� */
		if( !pthread_kill(info->tid,SIGUSR1) )
		{
			OS_LOG_DEBUG( "Thread<%s:%x> is kill!!!\n", info->name, (unsigned int)info );
			usleep( 10*1000 );
		}
#else
		/* ǿ���˳� */
		if( !pthread_cancel(info->tid) )
		{
			OS_LOG_DEBUG( "Thread<%s:%x> is canneled!!!\n", info->name, (unsigned int)info );
		}
		else
		{
			status = pthread_join( info->tid, &result );
			if( status == 0 )
			{
				if (result == PTHREAD_CANCELED )
				{
					OS_LOG_DEBUG( "Thread<%s:%x> canceled!!!\n", info->name, (unsigned int)info);//����߳��Ǳ�ȡ���ģ��ش�ӡ��
				}
				else
				{
					OS_LOG_DEBUG( "result:%d\n", (int)result);
				}
			}
		}
#endif
	}
	
	pthread_attr_destroy( &info->attr );
	sem_destroy( &info->sem );
	memset( info, 0, sizeof(*info) );
	m_all[i] = NULL;
	swos_free( info );
	
	m_ref--;

	if( m_mutex )
	{
		sw_mutex_unlock( m_mutex );
	}

	return;
}

/** 
 * @brief �߳��Ƿ񱻴�,  
 * 
 * @param thrd �߳̾��
 * 
 * @return ����򿪷��� ��(1); ���򷵻ؼ�(0) 
 */
unsigned int sw_thrd_is_openned( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return false;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;

	if( info == NULL )
	{
		return false;
	}

#ifdef WIN32
	return ( *((int *)&info->tid) != 0 );
#else
	return ( info->tid != 0 );
#endif
}

/** 
 * @brief �߳��Ƿ�running
 * 
 * @param thrd �߳̾��
 * 
 * @return ���running������(true);����,���ؼ�(false)
 */
bool sw_thrd_is_running( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return false;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if(info->handler == NULL)	
		return false;
	if( info->is_pause == 1 )
		return false;
	return true;
}

/** 
 * @brief �����߳����ȼ�, priority�ķ�ΧΪ[0,255],0��ʾ���ȼ����,255���
 * 
 * @param thrd �߳̾��
 * @param priority �߳����ȼ��Ĵ�С
 */
void sw_thrd_set_priority( HANDLE thrd, unsigned char priority )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return;
	}

	struct sched_param param;
	sthrdinfo_t *info = (sthrdinfo_t *)thrd;

	if( info == NULL )
	{
		return;
	}
	info->priority = (int)priority;
	priority = sched_get_priority_max( info->policy ) - 
					( priority * ( sched_get_priority_max( info->policy ) - sched_get_priority_min( info->policy ) ) ) / 255;
	param.__sched_priority = priority;
	pthread_setschedparam( info->tid, SCHED_RR, &param );

	return;
}

/** 
 * @brief ȡ���߳����ȼ�,priority�ķ�ΧΪ[0,255],0��ʾ���ȼ����,255���
 * 
 * @param thrd �߳̾�� 
 * 
 * @return ��ǰ�̵߳����ȼ�
 */
unsigned char sw_thrd_get_priority( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return 0;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	return info->priority;	
}

/** 
 * @brief �߳���ͣ
 * 
 * @param thrd �߳̾�� 
 */
void sw_thrd_pause( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if( info )
	{
		info->is_pause = 1;
	}

	return;
}

/** 
 * @brief �߳��Ƿ���ͣ
 * 
 * @param thrd �߳̾��
 * 
 * @return �����ͣ������(true);����,���ؼ�(false)
 */
bool sw_thrd_is_paused( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return false;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if( info )
	{
		return info->is_pause;
	}
	
	return false;	
}

/** 
 * @brief �̼߳���
 * 
 * @param thrd �߳̾�� 
 */
void sw_thrd_resume( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if( info )
	{
		info->is_pause = 0;
		sem_post( &(info->sem) );
	}

	return;
}

/** 
 * @brief �߳��ӳ�
 * 
 * @param timeout ��ʱʱ��(��λΪ����) 
 */
void sw_thrd_delay( int timeout )
{
	usleep( timeout*1000 );

	return;
}

/** 
 * @brief ȡ��ϵͳ����ʱ��(ms)
 * 
 * @return ��ǰ������ʱ��
 */
unsigned int sw_thrd_get_tick()
{
#ifdef WIN32
    return timeGetTime();
#else
	static uint32_t timeorigin;
	static bool firsttimehere = true;

	uint32_t now = times(NULL);
	
	//fix hi3560 platform bug
	if(now == (uint32_t)-1)
	{
		now = -errno;
	}

	if( firsttimehere )
	{
		timeorigin = now;
		firsttimehere = false;
	}

	return ( now - timeorigin )*10;
#endif
}


#ifdef ANDROID

/** 
 * @brief ���̺߳Ų����߳�
 * 
 * @param name ����
 * 
 * @return �߳̾��
 */
static HANDLE sw_thrd_find_pidname(pid_t pid)
{
	int i;

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler && pid == m_all[i]->pid )
		{
			return m_all[i];
		}
	}

	return NULL;
}


static void thread_exit_handler(int sig)  
{   
	HANDLE thrd = sw_thrd_find_pidname(getpid());
	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	if (info)
		OS_LOG_DEBUG( "On Live:Thread<%s:%x> is exit!!!\n", info->name, (unsigned int)info );
	pthread_exit(0);  
}  
#endif
/** 
 * @brief �߳̿�ʼ
 * 
 * @param lp_param 
 */
static void* sema_thread_handler( void* lp_param )
{
#ifdef ANDROID
	struct sigaction actions;  
	memset(&actions, 0, sizeof(actions));   
	sigemptyset(&actions.sa_mask);  
	actions.sa_flags = 0;   
	actions.sa_handler = thread_exit_handler;  
	if(sigaction(SIGUSR1,&actions,NULL)<0)
	{
		OS_LOG_DEBUG("sigaction failed !\n");
	}  
#endif
	sthrdinfo_t *info = (sthrdinfo_t *)lp_param;
	pid_t tid = 0;

#if defined(WIN32)
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL); //�����˳��߳�
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL ); //��������ȡ��
#elif defined(ANDROID)
	info->ppid = getpid();
	info->pid = gettid();
	tid = info->pid;
	int err = prctl(PR_SET_NAME, info->name); //�Ѳ���arg2��Ϊ���ý��̵ľ�������, Since Linux 2.6.11
	if(err != 0)
		OS_LOG_ERROR("prctl thread name fail\n");
	OS_LOG_DEBUG( "Thread-<%p, pid:%d, tid:%d, name:%s> handler\n",info, info->pid, tid, info->name);
#ifdef SUPPORT_SWAPI30
	sw_jni_get_env();
#endif
#else
	info->ppid = getppid();
	info->pid = syscall( SYS_gettid );/*getpid()*/;
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL); //�����˳��߳�
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL ); //��������ȡ��
#endif

	while( !info->is_exit )
	{
		
		if( info->is_pause )
		{
			sem_wait( &(info->sem) );
		}
		/* ���Ӳ��Ե� */
#ifndef ANDROID
		pthread_testcancel();
#endif

		if( info->handler == NULL || info->is_exit )
		{
			break;
		}

		if( !info->handler( info->wparam, info->lparam ) )
		{
			break;
		}
	}

	info->handler = NULL;

#if (defined(ANDROID) && defined(SUPPORT_SWAPI30))
	sw_jni_del_env();
#endif

	OS_LOG_DEBUG( "Thread-<%s:%x,pid:%d,tid:%d> exit self\n", info->name, (unsigned int)info, info->pid, tid );

	return NULL;
}

/** 
 * @brief ����ȫ�ֵ��Ȳ���
 * 
 * @param policy sw_policy_t
 */	
void sw_thrd_set_global_policy( sw_policy_t policy )
{
	m_global_policy = policy;
}

/** 
 * @brief ��ӡ�����߳� 
 */
void sw_thrd_print()
{
	int i;

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if(m_all[i] && m_all[i]->handler )
		{
			OS_LOG_INFO( "%-2d 0x%08x %-14s ppid=%d pid=%d proc=0x%-8x policy=%d priority=%d pause=%d \n", 
				   			i, (int)(m_all[i]), m_all[i]->name,m_all[i]->ppid, m_all[i]->pid, (int)m_all[i]->handler, 
				   			m_all[i]->policy, m_all[i]->priority, m_all[i]->is_pause);
		}
	}
	OS_LOG_INFO( "\nref = %d\n", m_ref );
	
	return;
}

/** 
 * @brief ��ȡ�����߳���Ϣ
 * 
 * @param buf 
 * @param size 
 * 
 * @return �������buf�Ĵ�С
 */
int sw_thrd_getinfo( char* buf, int size )
{
	if (buf == NULL || size <= 0)
		return 0;
	char sz_buf[512];
	int i, n, len;

	*buf = 0;
	len = 0;
	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler )
		{
			n = snprintf( sz_buf, sizeof(sz_buf), "%-2d 0x%08x %-14s ppid=%d pid=%d proc=0x%-8x policy=%d priority=%-3d pause=%d \n", 
				   i, (int)(m_all[i]), m_all[i]->name,m_all[i]->ppid, m_all[i]->pid,(int)m_all[i]->handler,
				   m_all[i]->policy, m_all[i]->priority, m_all[i]->is_pause);
			if( n < (int)sizeof(sz_buf) && len + n < size )
			{
				strlcpy( buf+len, sz_buf, len < size ? size-len : 0);
				len += n;
			}
		}
	}
	n = snprintf( sz_buf, sizeof(sz_buf), "\nref = %d\n", m_ref );
	if( len + n < size )
	{
		strlcpy( buf+len, sz_buf, len < size ? size-len : 0);
	}

	return len;
}

/** 
 * @brief �����ֲ����߳�
 * 
 * @param name ����
 * 
 * @return �߳̾��
 */
HANDLE sw_thrd_find_byname( char* name )
{
	int i;

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler && !strcmp( name, m_all[i]->name ) )
		{
			return m_all[i];
		}
	}

	return NULL;
}

/** 
 * @brief ���̺߳Ų����߳���,������ص��ַ������̰߳�ȫ,���߳�����ʹ��
 * 
 * @param tid �̺߳�
 * 
 * @return �߳���
 */
const char *sw_thrd_getname_bypid(pid_t tid)
{
	static char name[32];
	int i;
	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler && m_all[i]->pid == tid)
		{
			strlcpy(name, m_all[i]->name, sizeof(name));
			return &name[0];//����static��Ϊ��ֹʹ��m_all[i]->nameʱ�߳��˳������ڴ��ͷ����������
		}
	}
	return "";
}
