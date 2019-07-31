/** 
 * @file swmutex.h
 * @brief �����������ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */
#include "swapi.h"
#include "swos_priv.h"
#include "swmutex.h"

#define swos_malloc malloc
#define swos_free free

/** 
 * @brief ����������
 * 
 * @return �ɹ�,���ػ��������; ����,���ؿ�ֵ
 */
HANDLE sw_mutex_create()
{

	pthread_mutex_t* h = (pthread_mutex_t*)swos_malloc(sizeof(pthread_mutex_t));
	
	if(h)
	{
		if ( pthread_mutex_init(h, NULL) != 0)
		{/* ��ʼ��������ʧ��,����ԭ���error */
			perror("pthread_mutex_init: ");
			swos_free(h);
			h = NULL;
		}
	}
	
	return h;
	
}

/** 
 * @brief ���ٻ�����
 * 
 * @param mutex ���������
 */
void sw_mutex_destroy( HANDLE mutex )
{
	pthread_mutex_t* h = (pthread_mutex_t*)mutex;

	if(h==NULL)
	{	
		return;
	}
	
	pthread_mutex_destroy(h);
	swos_free(h);
	
}

/** 
 * @brief ����
 * 
 * @param mutex ���������
 */
void sw_mutex_lock( HANDLE mutex )
{
	if( mutex == NULL )
	{	
		return;
	}
	pthread_mutex_lock((pthread_mutex_t*)mutex );

}

/** 
 * @brief ����
 * 
 * @param mutex ���������
 */
void sw_mutex_unlock( HANDLE mutex )
{
	if( mutex == NULL )
	{	
		return;
	}
	pthread_mutex_unlock((pthread_mutex_t*)mutex );

}
