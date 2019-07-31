/************************************************************
* AUTHOR: lijian / huanghuaming / chenkai
* CONTENT: ��Ϣ���к����ӿ�
* NOTE:	
* HISTORY:
* [2005-9-16] created
* [2011-03-09] ����֧�����ȼ����У����֧��5�����ȼ������ȼ�	
*	��ֵԽС�����ȼ�Խ��
***********************************************************/
#include "swapi.h"
#include "swutil_priv.h"
#include "swmutex.h"
#include "swqueue.h"
#include "swsignal.h"

#define MAX_PRIORITY_NUM 5  //������ȼ���Ŀ

typedef struct
{
	/*���ȼ��ȼ���Ŀ*/
	int level_num;
	/*ÿ��Ԫ�صĴ�С*/
	uint32_t esize;
	/* �������� */
	void* elements[MAX_PRIORITY_NUM];
	/*�����ܿռ��С*/
	uint32_t tsize[MAX_PRIORITY_NUM];
	/* ��Ϣ�����еĶ�λ�� */
	void* read[MAX_PRIORITY_NUM];
	/* ��Ϣ�����е�дλ�� */
	void* write[MAX_PRIORITY_NUM];	
	/*������Ϣ���н����ź���*/
	HANDLE write_signal[MAX_PRIORITY_NUM];
	/*�Ƿ�����֮�󸲸��ϵ�Ԫ��*/
	bool full_cover[MAX_PRIORITY_NUM];
	/*������Ϣ���г����ź���*/
	HANDLE read_signal;
	/* ������Ϣ���н��ͳ����ź���*/
	HANDLE mutex;
}sw_queue_t;


/** 
 * @brief ��������
 * 
 * @param length ���г���
 * @param element_size ������ÿ��Ԫ�صĳ���
 * @param full_cover ��������ʱ���Ƿ�����ɾ����Ԫ��
 *
 * @return �������
 */
HANDLE sw_queue_create( int length,uint32_t element_size,bool full_cover )
{
	return sw_queue_create_with_priority(element_size,&length,&full_cover,1);
}

/** 
 * @brief ��������ȼ��Ķ���
 * 
 * @param element_size ������ÿ��Ԫ�صĳ���
 * @param qsize,ÿ���ȼ����еĳ���
 * @param full_cover ÿ���ȼ����е�������ʱ���Ƿ�����ɾ����Ԫ��
 * @param level_num  �ȼ�����Ŀ,���ȼ���Ŀ������5
 *
 * @return ���о��
 */
HANDLE sw_queue_create_with_priority(uint32_t element_size,int* qsize,bool* full_cover,int level_num)
{
	sw_queue_t *q=NULL;
	int i =  0;
	if( qsize == NULL || full_cover==NULL || element_size <=0 || level_num > MAX_PRIORITY_NUM )
		return NULL;

	q= malloc(sizeof(sw_queue_t));
	if( q == NULL )
	{
		UTIL_LOG_FATAL("No enough memory to create a queue!\n");
		return NULL;
	}
	memset(q,0,sizeof(sw_queue_t));
	
	q->esize = element_size;
	q->level_num = level_num;
	for( i=0; i<level_num; i++ )
	{
		q->elements[i] = (void*)malloc(element_size *qsize[i]);
		if( q->elements[i] == NULL )
		{
			UTIL_LOG_FATAL("No enough memory to malloc a queue elements!\n");
			goto ERROR_EXIT;
		}
	
 		q->tsize[i] = element_size *qsize[i];
		q->full_cover[i] = full_cover[i];
		q->read[i]=q->write[i]=q->elements[i];

		q->write_signal[i] = sw_signal_create( 0, 1 );
		if( q->write_signal[i] == NULL )
		{
			UTIL_LOG_FATAL("Fail to ceate a signal!\n");
			goto ERROR_EXIT;
		}
	}

	q->mutex = sw_mutex_create();
	if( q->mutex == NULL )
	{	
		UTIL_LOG_FATAL("Fail to ceate a mutex!\n");
		goto ERROR_EXIT;
	}
	q->read_signal = sw_signal_create( 0, 1 );
	if( q->read_signal == NULL )
	{
		UTIL_LOG_FATAL("Fail to ceate a signal!\n");
		goto ERROR_EXIT;
	}	
	return q;
ERROR_EXIT:
	if(q)
		sw_queue_destroy(q);
	return NULL;

	
}

/** 
 * @brief �ͷŶ�����Դ
 * 
 * @param handle �������
 */
void sw_queue_destroy( HANDLE handle )
{
	sw_queue_t *q = (sw_queue_t *)handle;
	int i;

	if(q == NULL )
		return;
	if( q->mutex)
		sw_mutex_lock( q->mutex );	
	for(i=0;i<q->level_num; i++)
	{
		if( q->write_signal[i] )
		{
			sw_signal_destroy(q->write_signal[i]);
			q->write_signal[i] = NULL;
		}

		if(q->elements[i])
		{
			free(q->elements[i]);
			q->elements[i] = NULL;
		}
	}
	q->level_num = 0;
	if( q->mutex)
		sw_mutex_unlock( q->mutex );	

	if( q->read_signal )
	{
		HANDLE rsignal = q->read_signal;
		q->read_signal = NULL;
		sw_signal_destroy(rsignal);
	}

	if( q->mutex)
	{
		HANDLE mutex = q->mutex;
		q->mutex = NULL;
		sw_mutex_destroy(mutex);
	}
	
	free(q);
}

/** 
 * @brief �����������Ԫ��
 * 
 * @param handle �������
 * @param e Ҫ���ӵ�Ԫ�صĵ�ַ
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post( HANDLE handle,void* e)
{	
	return sw_queue_post_with_level(handle,e,0);
}

/** 
 * @brief ��ָ���ȼ��Ķ���������Ԫ��
 * 
 * @param handle �������
 * @param e Ҫ���ӵ�Ԫ�صĵ�ַ
 * @param level ���ȼ��ȼ�
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post_with_level(HANDLE handle,void* e,int level)
{
	void* next = NULL;
	bool  is_full = false;
	sw_queue_t *q = (sw_queue_t *)handle;

	if(q == NULL || level >= q->level_num )
		return -1;
	

	sw_signal_wait(q->write_signal[level],0);

	sw_mutex_lock( q->mutex );	
	//������һ��elementλ��
	next = q->write[level]<(q->elements[level]+q->tsize[level]-q->esize) ? (q->write[level]+q->esize) : q->elements[level];
	//�ж϶����Ƿ��Ѿ�����
	is_full = (next == q->read[level]);

	sw_mutex_unlock(q->mutex);

	//���������ɾ���ϵ�Ԫ�أ�����һ��element��readָ�����
	//������Ҫ�ȴ��ճ�һ��Ԫ�������������Ԫ��
	if( !q->full_cover[level] && is_full)
	{
		sw_signal_wait(q->write_signal[level],WAIT_FOREVER);
	}

	sw_mutex_lock( q->mutex );

	memcpy(q->write[level],e,q->esize);

	q->write[level] = q->write[level]<(q->elements[level]+q->tsize[level]-q->esize) ? (q->write[level]+q->esize) : q->elements[level];
	/* ����������ˣ�ɾ�����ϵ���Ϣ */
	if(q->full_cover[level] && q->write[level] == q->read[level] )
		q->read[level] =q->read[level]<(q->elements[level]+q->tsize[level]-q->esize) ? q->read[level]+q->esize : q->elements[level];

	sw_mutex_unlock( q->mutex );
	sw_signal_give( q->read_signal);

	return 0;
}

/** 
 * @brief �Ӷ�������ȡԪ��,�������Ϊ�ջ�һֱ�ȴ�
 * 
 * @param handle �������
 * @param e �洢��ȡ��Ԫ�����ݵĵ�ַ
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read( HANDLE handle,void* e)
{
	return sw_queue_read_timeout(handle,e,-1);
}

/** 
 * @brief �Ӷ�������ȡԪ��
 * 
 * @param handle �������
 * @param e �洢��ȡ��Ԫ�����ݵĵ�ַ
 * @param timeout ��ȡԪ�صĳ�ʱʱ�䣬��λms
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_timeout(HANDLE handle,void* e,int timeout)
{
	int  idx_got = -1;
	bool is_empty = false;
	int i=0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || e==NULL || q->level_num <=0 )
		return -1;	
	
	if( sw_signal_wait(q->read_signal, timeout) != 0 )
		return -1;

	sw_mutex_lock( q->mutex);
	
	for( i = 0;i<q->level_num; i++ )
	{
		if( q->read[i] != q->write[i] &&  idx_got < 0 )
		{
			memcpy(e,q->read[i],q->esize);
			q->read[i] =q->read[i]<(q->elements[i]+q->tsize[i]-q->esize) ? q->read[i]+q->esize : q->elements[i];
			idx_got = i;
		}
		is_empty = (q->read[i] == q->write[i]);
		if( !is_empty )
			break;		
	}
	
	sw_mutex_unlock( q->mutex );

	if( !is_empty )
		sw_signal_give( q->read_signal );

	if( idx_got >=0 )
	{
		sw_signal_give(q->write_signal[idx_got]);
		return 0;
	}
	else
	{
		return -1;
	}
}

/** 
 * @brief �Ӷ�������ȡԪ�أ��������Ϊ�����̷���
 * 
 * @param handle �������
 * @param e �洢��ȡ��Ԫ�����ݵĵ�ַ
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_nowait( HANDLE handle,void* e)
{
	return sw_queue_read_timeout(handle,e,0);
}

/** 
 * @brief ȡ��ϵͳ��Ϣ������Ϣ��Ŀ
 * 
 * @param handle �������
 * 
 * @return 
 */
int sw_queue_get_num( HANDLE handle )
{
	int i=0;
	int num = 0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <=0 )
		return 0;
	
	sw_mutex_lock( q->mutex);

	for( i=0;i<q->level_num;i++)
	{
		if( q->write[i] > q->read[i] )
		{
			num += (q->write[i]-q->read[i])/q->esize;
		}
		else if( q->write[i] == q->read[i] )
		{
			num += 0;
		}
		else
		{
			num += (q->write[i]+q->tsize[i]-q->read[i])/q->esize;
		}
	}
	sw_mutex_unlock(q->mutex);

	return num;
}

/** 
 * @brief ȡ����Ϣ������󳤶�
 * 
 * @param handle �������
 * 
 * @return 
 */
int sw_queue_get_max( HANDLE handle )
{
	int i=0;
	int num = 0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <= 0 )
		return 0;

	for(i=0;i<q->level_num; i++)
		num += q->tsize[i]/q->esize - 1;
	
	return num;
}

/** 
 * @brief �����Ϣ����
 * 
 * @param handle �������
 */
void sw_queue_clear( HANDLE handle )
{
	int i =0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <=0 )
		return;
	
	sw_mutex_lock( q->mutex );
	for( i=0 ;i < q->level_num; i++)
	{
		q->write[i] = q->read[i] = q->elements[i];
	}
	sw_mutex_unlock( q->mutex );

	sw_signal_give(q->write_signal);
}

/** 
 * @brief ����ض����ȼ�����Ϣ����
 * 
 * @param handle �������
 * @param level_num ��Ϣ���ȼ�
 */
void sw_queue_clear_with_level( HANDLE handle, int level)
{
	int i =0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <=0 )
		return;
	
	sw_mutex_lock( q->mutex );
	
	if (0 <= level &&  level < q->level_num )
		q->write[level] = q->read[level] = q->elements[level];
	
	sw_mutex_unlock( q->mutex );

	sw_signal_give(q->write_signal);
}
