/** 
 * @file swqueue.h
 * @brief ���к����ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-9-16 created
 * 		 2011-03-09 ʵ�ֶ���ȼ��Ķ���
 */


#ifndef __SWQUEUE_H__
#define __SWQUEUE_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @brief ��������
 * 
 * @param length ���г���
 * @param element_size ������ÿ��Ԫ�صĳ���
 * @param full_cover ��������ʱ���Ƿ�����ɾ����Ԫ��
 *
 * @return �������
 */
HANDLE sw_queue_create(int length,uint32_t element_size,bool full_cover );

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
HANDLE sw_queue_create_with_priority(uint32_t element_size,int* qsize,bool* full_cover,int level_num);


/** 
 * @brief �ͷŶ�����Դ
 * 
 * @param handle �������
 */
void sw_queue_destroy( HANDLE handle );

/** 
 * @brief �����������Ԫ��
 * 
 * @param handle �������
 * @param e Ҫ���ӵ�Ԫ�صĵ�ַ
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post( HANDLE handle, void* e);

/** 
 * @brief ��ָ���ȼ��Ķ���������Ԫ��
 * 
 * @param handle �������
 * @param e Ҫ���ӵ�Ԫ�صĵ�ַ
 * @param level ���ȼ��ȼ�
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post_with_level(HANDLE handle,void* e,int level);


/** 
 * @brief �Ӷ�������ȡԪ��,�������Ϊ�ջ�һֱ�ȴ�
 * 
 * @param handle �������
 * @param e �洢��ȡ��Ԫ�����ݵĵ�ַ
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read( HANDLE handle,void* e);

/** 
 * @brief �Ӷ�������ȡԪ��
 * 
 * @param handle �������
 * @param e �洢��ȡ��Ԫ�����ݵĵ�ַ
 * @param timeout ��ȡԪ�صĳ�ʱʱ�䣬��λms
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_timeout( HANDLE handle,void* e,int timeout);

/** 
 * @brief �Ӷ�������ȡԪ�أ��������Ϊ�����̷���
 * 
 * @param handle �������
 * @param e �洢��ȡ��Ԫ�����ݵĵ�ַ
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_nowait( HANDLE handle,void* e);


/** 
 * @brief �����Ϣ����
 * 
 * @param handle �������
 */
void sw_queue_clear( HANDLE handle );

/** 
 * @brief ȡ��ϵͳ��Ϣ������Ϣ��Ŀ
 * 
 * @param handle �������
 * 
 * @return 
 */
int sw_queue_get_num( HANDLE handle );

/** 
 * @brief ȡ����Ϣ������󳤶�
 * 
 * @param handle �������
 * 
 * @return 
 */
int sw_queue_get_max( HANDLE handle );


#ifdef __cplusplus
}
#endif

#endif /* __SWQUEUE_H__ */
