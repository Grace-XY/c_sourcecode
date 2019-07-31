/** 
 * @file swmutex.h
 * @brief �����������ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */

#ifndef __SWMUTEX_H__
#define __SWMUTEX_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief ����������
 * 
 * @return �ɹ�,���ػ��������; ����,���ؿ�ֵ
 */
HANDLE sw_mutex_create();

/** 
 * @brief ���ٻ�����
 * 
 * @param mutex ���������
 */
void sw_mutex_destroy( HANDLE mutex );

/** 
 * @brief ����
 * 
 * @param mutex ���������
 */
void sw_mutex_lock( HANDLE mutex );

/** 
 * @brief ����
 * 
 * @param mutex ���������
 */
void sw_mutex_unlock( HANDLE mutex );

#ifdef __cplusplus
}
#endif

#endif  /* __SWMUTEX_H__  */
