/** 
 * @file swtimer.c
 * @brief ���嶨ʱ�������ӿ�,ע����timer�Ļص�������ֻ�ʺϴ����ʱ�Ƚ�С�����飬
 *		�����Ӱ��������ʱ��ϵͳ��׼ȷ��
 *
 * @author hujinshui/chenkai
 * @date 2010-07-6 created
 */
  
#include "swapi.h"
#include "swutil_priv.h"
#include "swthrd.h"
#include "swmutex.h"
#include "swmem.h"
#include "swtimer.h"
#include "swsignal.h"

#define MAX_TIMERNAME_NUM	64
#define MAX_MEM_SIZE      64*1024

#define timer_alloc( size ) sw_mem_alloc( m_timer_des.hmem ,size, __FILE__, __LINE__ )
#define timer_free( p ) sw_mem_free( m_timer_des.hmem , p ,__FILE__, __LINE__ )

/**   
 * @brief  ��ʱ�����ݽṹ
 * 
 * @param name[MAX_TIMERNAME_NUM]				��ʱ������
 * @param interval		��ʱ�������ļ��ʱ��
 * @param times				��ʱ�������¼��Ĵ���
 * @param handler			��ʱ��������
 * @param wparam			��һ������
 * @param wparam			�ڶ�������
 * @param type				��ʱ������
 * @param status			��ʱ��״̬
 * 
 */
typedef struct timer
{
	char name[MAX_TIMERNAME_NUM];
	HANDLE handle; 					//timernode�ľ��
	unsigned long interval; 		//���õ�ֵ,�����������೤ʱ�䴦��һ���¼�
	long times;    		//���¼�ִ�ж��ٴ�
	long original_times; 	//��¼ԭʼ��times
	unsigned long start_time;
	unsigned long trigger_time;  	//�¼�����Ĵ���ʱ��  
	unsigned long store_time;   	//��¼�ϴ�����ʣ�µ�ʱ��
	ontimehandler_t handler;    	//������
	uint32_t wparam;				//�������ĵ�һ������
	uint32_t lparam;				//�������ĵڶ�������
	timer_status_t  status;			//��ʱ����״̬
	struct timer *next;				
	struct timer *prev;				

}timer_nod_t;

typedef struct timer_des
{
	HANDLE 			htimer_thrd;
	HANDLE 			htimer_mutex;
	HANDLE			htimer_signal;
	HANDLE 			hmem;
	timer_nod_t *ptimer_head;
	timer_nod_t *ptimer_tail;
	char		*ptimer_buf;
	unsigned long timeout;    
	bool	exit;
}timer_des_t;

static timer_des_t m_timer_des;    //ȫ��timer�����ṹ


/**
 * @brief �Ƚ�timeout��ֵ,ȡ��ǰ��������С�ģ�������
 */
static void upgrade_timeout( timer_nod_t *ptimer )
{
	if( NULL == ptimer )
	{
		return ;
	}

	int now;
	now = sw_thrd_get_tick( );
	if( m_timer_des.timeout > ( ptimer->trigger_time - now ) )
	{
		m_timer_des.timeout = ptimer->trigger_time - now;	
	}
}

/**
 * @brief 	���һ����ʱ�����,ʹ���붨ʱ������
 *
 * @param	ptimer ��ʱ�����ָ�� 	
 *
 * @return 0��ʾ��ʱ�����ɹ����������� -1��ʾ����ʧ��
 */
static int add_timer( timer_nod_t *ptimer )
{
//	timer_nod_t *tmp_timer;

	if ( NULL == ptimer )
	{
		return -1;
	}

	sw_mutex_lock(m_timer_des.htimer_mutex); 

	upgrade_timeout( ptimer );


	if ( m_timer_des.ptimer_head == NULL && m_timer_des.ptimer_tail == NULL )   //��һ�β��������
	{
		ptimer->prev = NULL;
		ptimer->next = NULL;
		m_timer_des.ptimer_head = ptimer;
		m_timer_des.ptimer_tail = ptimer;

		sw_mutex_unlock( m_timer_des.htimer_mutex );

		sw_signal_give( m_timer_des.htimer_signal );

		return 0;
	}
	else	 //�ڱ�β����
	{
		ptimer->prev = m_timer_des.ptimer_tail;
		ptimer->next = NULL;
		m_timer_des.ptimer_tail->next = ptimer;
		m_timer_des.ptimer_tail = ptimer;

		sw_mutex_unlock( m_timer_des.htimer_mutex );

		sw_signal_give( m_timer_des.htimer_signal );

		return 0;
	}

	return -1;		
}




/**
 * @brief ��������ɾ��һ����ʱ�����
 * param  ��ʱ�����
 *
 * @return 0��ʾɾ���ɹ�  -1��ʾɾ��ʧ��
 */
static int delete_timer( timer_nod_t *ptimer )
{
	if ( NULL == ptimer )
	{	
		return -1;
	}

	if ( m_timer_des.ptimer_head == NULL )   //���ж���Ϊ�գ����ش���
	{
		UTIL_LOG_INFO( "ɾ��һ���ն���!" );

		return -1;
	}
	else
	{
		//ɾ����㣬��ʱ������ֻ��һ�����
		if( ptimer == m_timer_des.ptimer_head && ptimer == m_timer_des.ptimer_tail  )
		{
			m_timer_des.ptimer_head = NULL;
			m_timer_des.ptimer_tail = NULL;
			timer_free( ptimer );   	
		}

		//ɾ�����ڱ�ͷ�Ľ��
		else if( ptimer == m_timer_des.ptimer_head && ptimer != m_timer_des.ptimer_tail  )
		{
			m_timer_des.ptimer_head = ptimer->next;
			ptimer->next->prev = NULL;
			timer_free( ptimer );   	
		}

		//ɾ�����ڱ�β�Ľ��
		else if( ptimer != m_timer_des.ptimer_head && ptimer == m_timer_des.ptimer_tail )
		{
			m_timer_des.ptimer_tail = ptimer->prev;
			ptimer->prev->next = NULL;
			timer_free( ptimer );
		}

		//ɾ�����ڱ��еĽ��
		else if( ptimer != m_timer_des.ptimer_head && ptimer != m_timer_des.ptimer_tail )
		{
			ptimer->prev->next = ptimer->next;
			ptimer->next->prev = ptimer->prev;
			timer_free( ptimer );   
		}

		return 0;
	}

}

/** 
 * @brief ��ʱʱ�䵽�ˣ�����������������¼�
 * 
 * @param ptimer  ��ʱ����� 
 *
 * return  ��
 */
static void on_tick( timer_nod_t *ptimer )
{
	unsigned long now;

	if ( NULL == ptimer )
	{
		return;
	}
	if ( ptimer->status != TIMER_STATUS_ENABLE )
	{
		return;
	}
	if ( ptimer->times == 0 )    //��ʾN���¼������˽���
	{
		return;
	}

	if( ptimer->times > 0 && ptimer->status == TIMER_STATUS_ENABLE )
	{
		now = sw_thrd_get_tick();

		if( ptimer->trigger_time <= now )
		{
			ptimer->handler( ptimer->wparam, ptimer->lparam );
			ptimer->times--;
			UTIL_LOG_INFO( "[%s]the rest times is %ld,now is %u \n ",__FUNCTION__,ptimer->times,sw_thrd_get_tick() );

			if( ptimer->times > 0 )   //���times����0����ʾ�����´δ����¼������Ϊ0��ʾû���´δ����¼�
			{
				ptimer->trigger_time = ptimer->trigger_time + ptimer->interval;
			}
			else if(ptimer->times == 0 ) 
			{
				UTIL_LOG_INFO( "return to the thread_pro \n" );
			
				return ;
			}
			
		}
	}
	else if( ptimer->times == -1 && ptimer->status == TIMER_STATUS_ENABLE )
	{
		now = sw_thrd_get_tick();
		if( ptimer->trigger_time <= now )
		{
			ptimer->handler( ptimer->wparam, ptimer->lparam );
			UTIL_LOG_INFO( "[%s]the rest times is %ld,now is %u \n ",__FUNCTION__,ptimer->times,sw_thrd_get_tick() );

			ptimer->trigger_time = ptimer->trigger_time + ptimer->interval;
		}
	}

}


/** 
 * @brief ��ѯ�����У���С��timeoutֵ
 *	
 * return ���ص�ǰ������timeoutֵ��С�Ľ��		
 */
static unsigned long find_min_timeout() //����һ����С��timeout
{
	timer_nod_t *tmp_timer;
	unsigned long now;
	unsigned long min_timeout;
	min_timeout = m_timer_des.timeout;

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( tmp_timer = m_timer_des.ptimer_head; tmp_timer != NULL ; tmp_timer = tmp_timer->next )		
	{
		if( tmp_timer->status == TIMER_STATUS_ENABLE )
		{
			now = sw_thrd_get_tick( );

			if( min_timeout > ( tmp_timer->trigger_time - now ) )
			{
				min_timeout = tmp_timer->trigger_time - now;

				sw_mutex_unlock( m_timer_des.htimer_mutex );
			}
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return min_timeout;	
}

/** 
 * @brief ��ʱ���߳�
 * 
 * @param wparam  �̵߳ĵ�һ������
 * @param wparam  �̵߳ĵڶ�������
 *
 * return 
 */

static bool thread_pro( unsigned long wparam, unsigned long lparam )
{
	timer_des_t *time_des = (timer_des_t*)wparam;
	timer_nod_t *ptimer;

	time_des->timeout = -1;
	time_des->timeout = find_min_timeout(); 		

	sw_signal_wait( time_des->htimer_signal, time_des->timeout );   //�ź������ȴ�ʱ����timeout
	if (time_des->exit)
	{
		time_des->htimer_thrd = NULL;
		return false;
	}

	sw_mutex_lock( time_des->htimer_mutex );    //���̼߳�����ԭ�Ӳ���,׼����������

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )	//�������������˾ʹ����¼�
	{
		on_tick(ptimer);
	}

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer->times == 0 )
		{
			delete_timer( ptimer );
		}
	}

	sw_mutex_unlock( time_des->htimer_mutex );

	return true ;
}


/** 
 * @brief �ͷŶ�����Դ
 * 
 * @return �� 
 */
static int release_queue()
{
	timer_nod_t *ptimer = NULL;	
	timer_des_t *time_des = &m_timer_des; 

	sw_mutex_lock( time_des->htimer_mutex );

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )	
	{
		ptimer->status =  TIMER_STATUS_DISABLE;
	}

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )	
	{
		if( -1 == delete_timer( ptimer ) )
		{
			UTIL_LOG_INFO("delete timer failed \n");
			sw_mutex_unlock( time_des->htimer_mutex );

			return -1;
		}
	}

	sw_mutex_unlock( time_des->htimer_mutex );

	return 0;
}

/** 
 * @brief ��ʼ����ʱ��
 * 
 * @param priority  ��ʱ���̵߳����ȼ� 
 * @param stacksize �������ʱ���̶߳�ջ�Ĵ�С
 *
 * @return 0,��ʼ���ɹ�; �����, ʧ��
 */
int sw_timer_init( int priority, int stacksize )
{
	m_timer_des.ptimer_buf = (char *) malloc( MAX_MEM_SIZE );

	if( NULL == m_timer_des.ptimer_buf )
	{
		UTIL_LOG_ERROR( "[sw_timer_init] no memory %d\n",MAX_MEM_SIZE );

		return -1;
	}

	m_timer_des.hmem = sw_mem_init( m_timer_des.ptimer_buf, MAX_MEM_SIZE, 4 );

	if( NULL == m_timer_des.hmem )
	{
		UTIL_LOG_ERROR( "[sw_timer_init] sw_mem_init error \n" );

		return -1;
	}

	//���г�ʼ��
	m_timer_des.ptimer_head = NULL;
	m_timer_des.ptimer_tail = NULL;
	UTIL_LOG_INFO( "timerinit  : queue is inited!\n" );

	//��ʼ��������
	m_timer_des.htimer_mutex = sw_mutex_create();
	UTIL_LOG_INFO( "timerinit  : mutex is inited!:%p\n", m_timer_des.htimer_mutex );

	//��ʼ���ź���
	m_timer_des.htimer_signal = sw_signal_create( 0, 1 );
	UTIL_LOG_INFO( "timerinit  : signal is inited\n" );

	//���ó�ʼ�ȴ�ʱ��Ϊ-1
	m_timer_des.timeout = -1;
	m_timer_des.exit = false;

	//���߳�thread_pro
	m_timer_des.htimer_thrd = sw_thrd_open( "swtimer", priority, 0, stacksize, 
								(threadhandler_t)thread_pro, (uint32_t)(&m_timer_des), 0 );
	UTIL_LOG_INFO( "timerinit  :  thread is inited!\n" );

	if( m_timer_des.htimer_thrd )
	{	
		sw_thrd_resume( m_timer_des.htimer_thrd );
	}
	UTIL_LOG_INFO( "timerinit  :  thread is resumed!\n" );

	return 0;	

}


/**
 * @brief �˳�timer ģ�飬�ͷ���Ӧ����Դ
 * 
 * @return ��
 */
void sw_timer_exit( )
{
	//�ͷŶ�����ռ�õ���Դ
	if( -1 == release_queue() )
	{
		UTIL_LOG_ERROR("queue release failed\n");
	}
	m_timer_des.exit = true;
	sw_signal_give( m_timer_des.htimer_signal );
	//�ر��߳�
	if( m_timer_des.htimer_thrd )
	{
		sw_thrd_close( m_timer_des.htimer_thrd , 2000 );
		m_timer_des.htimer_thrd = NULL;
	}

	//ɾ���ź���
	if( m_timer_des.htimer_signal )
	{	
		sw_signal_destroy( m_timer_des.htimer_signal );
		m_timer_des.htimer_signal = NULL;
	}

	//ɾ��������
	if( m_timer_des.htimer_mutex )
	{
		sw_mutex_destroy( m_timer_des.htimer_mutex );
		m_timer_des.htimer_mutex = NULL;
	}

	//���ɾ����ʱ��ģ�����Դ
	if( m_timer_des.hmem )
	{
		sw_mem_exit( m_timer_des.hmem );
		m_timer_des.hmem = NULL;
	}
}

/**
 * @brief ����һ����ʱ��
 *
 * @param name,��ʱ��������
 * @param interval, ��ʱ��ʱ����
 * @param times,��ʱ���ظ�������0��ʾ���޴�
 * @param handler,��ʱ���ص�����
 * @param wparam,��ʱ���ص�����1
 * @param lparam,��ʱ���ص�����2
 *
 * @return ��� ; NULL,ʧ��
 */
HANDLE sw_timer_create( char* name,int interval,int times,
		ontimehandler_t	handler,uint32_t wparam,uint32_t lparam )     //�൱��һ��newtimer ,����һ��node,���Ҹ�node��ֵ
{
	timer_nod_t *ptimer;
	if( NULL == name )
	{
		UTIL_LOG_ERROR( "param interval is invalid value" );

		return NULL;
	}
	//�ж�interval��times����ĺϷ���
	if( interval < 0 )
	{
		UTIL_LOG_ERROR( "param interval is invalid value" );

		return NULL;
	}	
	if( times < -1 )
	{
		UTIL_LOG_ERROR( "param times is invalid value" );

		return NULL;
	}

	//�ж�handler�����Ƿ�Ϸ�
	if( NULL == (ontimehandler_t)handler )
	{
		UTIL_LOG_ERROR( "param handler is invalid value(NULL)" );

		return NULL;
	}
    
	//�ж�ptimer�Ƿ�����ɹ�
	ptimer = ( timer_nod_t* )timer_alloc( sizeof( timer_nod_t ) );
	if ( ptimer == NULL )
	{
		UTIL_LOG_ERROR("timercreate  :  alloc memory failed \n ");

		return NULL;   
	}
	
	UTIL_LOG_INFO("[%s] : has alloc memory for %s \n",__FUNCTION__, name );

	//�Դ�����timer_node ��ʼ����ֵ��
	strlcpy( ptimer->name, name, sizeof( ptimer->name ) );
	ptimer->status = TIMER_STATUS_ENABLE; //����
	ptimer->interval = interval; //ʱ�Ӽ���	
	ptimer->times = times;
	ptimer->original_times = times;
	ptimer->handler = handler;
	ptimer->start_time = sw_thrd_get_tick();
	ptimer->trigger_time = ptimer->start_time + ptimer->interval;

	if ( add_timer( ptimer ) != 0 )  //��add_timer()Ҳ��sw_timer_create()��ʵ��
	{
		UTIL_LOG_ERROR("timercreate  :   timer node add to the list is failed\n");
		timer_free( ptimer );

		return NULL;
	}

	UTIL_LOG_INFO("[%s]  :%s has add to the list \n ",__FUNCTION__,name );

	return (HANDLE)ptimer ;
}

/**
 * @brief ����ָ����ʱ��
 * @param handle,��ʱ�����
 *
 * @return ��
 */
void sw_timer_destroy( HANDLE handle )
{
	if( NULL == (timer_nod_t *)handle )
	{
		return ;
	}
	//����handleֵ��ɾ����ʱ�������е�node
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			delete_timer( ptimer );	
			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return ;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return ;
}


/**
 * @brief ���ö�ʱ��,״̬�л����տ�ʼ������״̬
 * @param handle,��ʱ�����
 *
 * @return �Ƿ����óɹ���0 �ɹ���-1ʧ��
 */
int sw_timer_reset( HANDLE handle )
{
	//����handle����ָ��node��times��Ϊ0��	
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{
		return -1 ;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == handle )
		{
			ptimer->times = ptimer->original_times;
			ptimer->start_time = sw_thrd_get_tick();  //����start_timeΪϵͳ��ǰʱ��
			ptimer->trigger_time = ptimer->start_time + ptimer->interval; 
			ptimer->status = TIMER_STATUS_ENABLE;

			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return 0;	
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return -1;
}

/** 
 * @brief ȡ�ö�ʱ��״̬
 * @param handle,��ʱ�����
 *
 * @return �ɹ������ض�ʱ��״̬��ʧ�ܷ���-1
 */
timer_status_t sw_timer_get_status( HANDLE handle )
{

	//����handle,��ȡnode������ָ��node�Ķ�ʱ��״̬��Ҳ��Ҫ������ʱ������	
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1 ;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			sw_mutex_unlock( m_timer_des.htimer_mutex );	

			return ptimer->status;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex ); 

	return -1;
}

/** 
 * @brief ���ö�ʱ��״̬
 * @param handle,��ʱ�����
 *
 * @return ����0��ʾ���óɹ���-1��ʾ����ʧ��
 */
int sw_timer_set_status( HANDLE handle, timer_status_t status )
{

	//����handle,��ȡnode������ָ��node�Ķ�ʱ��״̬��Ҳ��Ҫ������ʱ������	
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;
	int now = 0;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1;
	}

	//�ж�status������ֵ�Ƿ��д���
	if( !( status == 0 || status == 1 ) )
	{
		UTIL_LOG_ERROR( "param status is invalid value" );

		return -1;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			UTIL_LOG_INFO("find the handle1");
			if( ptimer->status == TIMER_STATUS_ENABLE && status == TIMER_STATUS_DISABLE  )
			{
				now = sw_thrd_get_tick( ); 
				ptimer->store_time = ptimer->trigger_time -now;

				ptimer->status = status;
		//		UTIL_LOG_INFO( "###%d\n,$$%lu,&&&%lu ",ptimer->status, ptimer->trigger_time ,ptimer->store_time );
				sw_mutex_unlock( m_timer_des.htimer_mutex );

				return 0;
			}

			else if( ptimer->status == TIMER_STATUS_DISABLE && status ==TIMER_STATUS_ENABLE )
			{
				now = sw_thrd_get_tick( );
				ptimer->trigger_time = ptimer->store_time + now;
				ptimer->status = status;

			//	UTIL_LOG_INFO( "###%d\n,$$$%lu,&&&%lu ",ptimer->status, ptimer->trigger_time ,ptimer->store_time );
				sw_mutex_unlock( m_timer_des.htimer_mutex );	

				sw_signal_give( m_timer_des.htimer_signal );

				return 0;

			}

			else if( ( ptimer->status == TIMER_STATUS_ENABLE && status ==TIMER_STATUS_ENABLE )||
						( ptimer->status == TIMER_STATUS_DISABLE && status ==TIMER_STATUS_DISABLE ) )
			{
				ptimer->status = status;

			//	UTIL_LOG_INFO( "##%d\n,$$%lu ",ptimer->status, ptimer->trigger_time );

				sw_mutex_unlock( m_timer_des.htimer_mutex );	

				return 0;
			}

	/*		else if( ptimer->status == TIMER_STATUS_DISABLE && status ==TIMER_STATUS_DISABLE )
			{
				ptimer->status = status;

				UTIL_LOG_INFO( "###%d\n,$$$%lu",ptimer->status, ptimer->trigger_time );

				sw_mutex_unlock( m_timer_des.htimer_mutex );	

				return 0;
			}
			*/
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex ); 

	return -1;
}

/**
 * @brief ��������ȡ�ö�ʱ�����
 * @param name, timer����
 *
 *	@return ָ�����ƵĶ�ʱ�����,�ɹ�; NULL��ʧ��
 */
HANDLE sw_timer_get_byname( char* name )
{

	timer_nod_t* ptimer = NULL;
	
	if( NULL == name )
	{
		return NULL;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( strcmp( ptimer->name , name ) == 0 )
		{
			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return (HANDLE)ptimer;	
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return NULL;
}

/**
 * @brief ��ʱ������
 * @param handle,��ʱ�����
 * @param handler,��ʱ���ص�����
 * @param wparam,��ʱ���ص�����1
 * @param lparam,��ʱ���ص�����2
 *  
 * @return �Ƿ����óɹ���0 �ɹ���-1ʧ��
 */
int sw_timer_set_ontimehandler( HANDLE handle,ontimehandler_t handler,uint32_t wparam,uint32_t lparam )   //����SET �ص�������ʹhandlerָ����������
{
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1;
	}
	
	//�ж�handler�Ƿ�Ϸ�
	if( NULL == (ontimehandler_t)handler )
	{
		UTIL_LOG_ERROR( "param handler is invalid value(NULL)" );

		return -1;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );  

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			ptimer->handler = handler;
			ptimer->wparam = wparam;
			ptimer->lparam = lparam;
			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return 0;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return -1;
}

/**
 * @brief ���ö�ʱ����ʱʱ��,�ú����ᵼ�¶�ʱ������
 * @param handle,��ʱ�����
 * @param interval, ��ʱ��ʱ����  
 * @param times,��ʱ���ظ�������-1��ʾ���޴�
 *
 * @return �Ƿ����óɹ���0 �ɹ���-1ʧ��
 */
int sw_timer_set_interval( HANDLE handle,int interval,int times )
{
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1;
	}
	//�ж�interval��times����ĺϷ���
	if( interval < 0 )
	{
		UTIL_LOG_ERROR( "param interval is invalid value" );

		return -1;
	}	
	if( times < -1 )
	{
		UTIL_LOG_ERROR( "param times is invalid value" );

		return -1;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );  

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer  == timer_h )
		{
			ptimer->times = times;
			ptimer->original_times = times;
			ptimer->start_time = sw_thrd_get_tick( );  //����start_timeΪϵͳ��ǰʱ��
			ptimer->interval=interval;
			ptimer->trigger_time = ptimer->start_time + ptimer->interval; 
			ptimer->status = TIMER_STATUS_ENABLE;

			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return 0;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return -1;	
}

/**
  ��brief ��ӡ���ж�ʱ����Ϣ
 * @return ��
 */
void sw_timer_print()
{
	timer_nod_t* ptimer = NULL;

	sw_mutex_lock( m_timer_des.htimer_mutex );  

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( NULL == ptimer->next )
		{
			UTIL_LOG_INFO( "name is %s, interval=%lu, original_times=%ld,  start_time=%lu, status=%d \n ",
					ptimer->name, ptimer->interval, ptimer->original_times, ptimer->start_time, ptimer->status );

			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return ;
		}
		else
		{
			UTIL_LOG_INFO( "name is %s, interval=%lu, original_times=%ld,  start_time=%lu, status=%d \n ",
					ptimer->name, ptimer->interval, ptimer->original_times, ptimer->start_time, ptimer->status );
		}

	}
	UTIL_LOG_INFO("the timer list is empty \n ");		

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return ;
}




