/** 
 * @file swthrd.h
 * @brief �̺߳����ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16 created
 */

#ifndef __SWTHRD_H__
#define __SWTHRD_H__


#ifdef __cplusplus
extern "C"
{
#endif
#include "swapi.h"

/**
 * �̵߳��Ȳ���
 * 
 */
typedef enum
{
	SW_SCHED_NORMAL = 0,		/* һ����� */
	SW_SCHED_FIFO,				/*�Ƚ��ȳ�  */
	SW_SCHED_RR				/* ѭ����   */
}sw_policy_t;


/* �Ƽ��̶߳�ջ��С(�ֽ�)*/
#define DEFAULT_STACK_SIZE	64*1024
/* �Ƽ��߳����ȼ� */
#define DEFAULT_PRIORIOTY	100

/**   
 * @brief  �̻߳ص�����,�ᱻ��������,����0��ʾ�˳��߳�,������ʾ�������� 
 * 
 * @param *threadhandler_t	ָ��ص�������ָ��
 * @param wparam 			�ص������ĵ�һ������
 * @param lparam 			�ص������ĵڶ�������
 * 
 * @return ��(��0),�̱߳���������; ��(0),�߳��˳�
 */
typedef bool (*threadhandler_t)(uint32_t wparam, uint32_t lparam );

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
				   threadhandler_t handler, uint32_t wparam, uint32_t lparam );
/** 
 * @brief �ر��߳�
 * 
 * @param thrd �߳̾��
 * @param ms �ȴ���ʱ��(��λΪ����)
 */
void sw_thrd_close( HANDLE thrd, int ms );
/** 
 * @brief �߳��Ƿ񱻴�,  
 * 
 * @param thrd �߳̾��
 * 
 * @return ����򿪷��� ��(1); ���򷵻ؼ�(0) 
 */
unsigned int sw_thrd_is_openned( HANDLE thrd );
/** 
 * @brief �߳��Ƿ�running,  
 * 
 * @param thrd �߳̾��
 * 
 * @return ���running���� ��(1); ���򷵻ؼ�(0) 
 */
bool sw_thrd_is_running( HANDLE thrd );
/** 
 * @brief �����߳����ȼ�, priority�ķ�ΧΪ[0,255],0��ʾ���ȼ����,255���
 * 
 * @param thrd �߳̾��
 * @param priority �߳����ȼ��Ĵ�С
 */
void sw_thrd_set_priority( HANDLE thrd, unsigned char priority );
/** 
 * @brief ȡ���߳����ȼ�,priority�ķ�ΧΪ[0,255],0��ʾ���ȼ����,255���
 * 
 * @param thrd �߳̾�� 
 * 
 * @return ��ǰ�̵߳����ȼ�
 */
unsigned char sw_thrd_get_priority( HANDLE thrd );

/** 
 * @brief �߳���ͣ
 * 
 * @param thrd �߳̾�� 
 */
void sw_thrd_pause( HANDLE thrd );
/** 
 * @brief �̼߳���
 * 
 * @param thrd �߳̾�� 
 */
void sw_thrd_resume( HANDLE thrd );
/** 
 * @brief �߳��Ƿ���ͣ
 * 
 * @param thrd �߳̾��
 * 
 * @return �����ͣ������(true);����,���ؼ�(false)
 */
bool sw_thrd_is_paused(HANDLE thrd);
	
/** 
 * ����ȫ�ֵ��Ȳ���
 * 
 * @param policy sw_policy_t
 */	
void sw_thrd_set_global_policy(sw_policy_t policy);
	

/** 
 * @brief �߳��ӳ�
 * 
 * @param timeout ��ʱʱ��(��λΪ����) 
 */
void sw_thrd_delay( int timeout );
/** 
 * @brief ȡ��ϵͳ����ʱ��(ms)
 * 
 * @return ��ǰ������ʱ��
 */
unsigned int sw_thrd_get_tick();

/** 
 * @brief ��ӡ�����߳� 
 */
void sw_thrd_print();

/** 
 * @brief ��ȡ�����߳���Ϣ
 * 
 * @param buf 
 * @param size 
 * 
 * @return �������buf�Ĵ�С
 */
int sw_thrd_getinfo( char* buf, int size );
	
/** 
 * @brief �����ֲ����߳�
 * 
 * @param name ����
 * 
 * @return �߳̾��
 */
HANDLE sw_thrd_find_byname( char* name );
	

#ifdef __cplusplus
}
#endif

#endif  /* __SWTHRD_H__ */
