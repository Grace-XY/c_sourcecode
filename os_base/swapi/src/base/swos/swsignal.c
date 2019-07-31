/** 
 * @file swsignal.h
 * @brief �ź��������ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */
#include "swapi.h"
#include "swos_priv.h"
#include "swsignal.h"

typedef struct sw_signal_t
{
  sem_t sem;
  int max_val;
}sw_signal_t;

#define swos_malloc malloc
#define swos_free free

/** 
 * @brief �����ź���
 * 
 * @param defval ȡֵΪ0
 * @param maxval ȡֵΪ1
 * 
 * @return �ɹ�,�����ź������; ����,���ؿ�ֵ
 */
HANDLE  sw_signal_create( int defval, int maxval )
{
	sw_signal_t* h = (sw_signal_t*)swos_malloc( sizeof( sw_signal_t ) );

	if( h == NULL )
	{	
		return NULL;
	}
	if ( sem_init( &(h->sem), 0, defval ) == -1)
	{/* ��ʼ���ź���ʧ��,ʧ��ԭ���error */
		perror("sem_init: ");
		swos_free( h );
		h = NULL;
	}
	else
		h->max_val = maxval;
	return h;
}

/** 
 * @brief �����ź���
 * 
 * @param signal �ź������
 */
void sw_signal_destroy( HANDLE sem )
{
	if( sem == NULL )
	{
		return;
	}
	
	sem_destroy( (sem_t*)sem );
	swos_free( sem );
}

/** 
 * @brief �ȴ��ź���
 * 
 * @param signal �ź������
 * @param timeout -1��ʾ���޵ȴ�;����ֵ��ʾ�ȴ���ʱ��(ms)
 * 
 * @return �ɹ�,����0; ����,����-1
 */
int sw_signal_wait( HANDLE sem, int timeout )
{
	struct timespec tv;
     	
	if( sem == NULL )
	{	
		return 0;
	}
	
	if( timeout < 0 )
	{/* ��Ҫ�жϷ���ֵ,��Ϊ����ź�����destroy��Ҳ�᷵�ص� */
		return sem_wait( (sem_t*)sem ) == 0 ? 0 : -1;
	}
	else
	{
#ifdef WIN32
        gettimeofday(&tv, NULL);
#else
	 	clock_gettime( CLOCK_REALTIME, &tv );
#endif

		tv.tv_nsec += (timeout%1000)*1000000; 
		if( tv.tv_nsec>= 1000000000 )
		{
			tv.tv_sec +=1;
			tv.tv_nsec-=1000000000;
		}
		tv.tv_sec += timeout/1000;
		if( sem_timedwait( (sem_t*)sem, (const struct timespec *)&tv ) )
		{
			return -1;
		}
		return 0;
	}
}

/** 
 * @brief �ͷ��ź���
 * 
 * @param signal �ź������
 */
void sw_signal_give( HANDLE sem )
{
	int val;

	if( sem == NULL )
	{
		return;
	}

	sem_getvalue( (sem_t*)sem, &val );

	if( val < ((sw_signal_t*)sem)->max_val )
	{
		sem_post( (sem_t*)sem );
	}
}

/** 
 * @brief �����ź���
 * 
 * @param signal �ź������
 */
void sw_signal_reset( HANDLE sem )
{
	sw_signal_give( sem );
}
