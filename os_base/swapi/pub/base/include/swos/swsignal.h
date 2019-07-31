/** 
 * @file swsignal.h
 * @brief �ź��������ӿ�
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */

#ifndef __SWSIGNAL_H__
#define __SWSIGNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief �����ź���
 * 
 * @param defval ȡֵΪ0
 * @param maxval ȡֵΪ1
 * 
 * @return �ɹ�,�����ź������; ����,���ؿ�ֵ
 */
HANDLE sw_signal_create( int defval, int maxval );

/** 
 * @brief �����ź���
 * 
 * @param signal �ź������
 */
void sw_signal_destroy( HANDLE signal );

/** 
 * @brief �ȴ��ź���
 * 
 * @param signal �ź������
 * @param timeout -1��ʾ���޵ȴ�;����ֵ��ʾ�ȴ���ʱ��(ms)
 * 
 * @return �ɹ�,����0; ����,����-1
 */
int sw_signal_wait( HANDLE signal, int timeout );

/** 
 * @brief �ͷ��ź���
 * 
 * @param signal �ź������
 */
void sw_signal_give( HANDLE signal );

/** 
 * @brief �����ź���
 * 
 * @param signal �ź������
 */
void sw_signal_reset( HANDLE signal );

#ifdef __cplusplus
}
#endif

#endif  /* __SWSIGNAL_H__ */
