/** 
 * @file util.h
 * @brief util�⣬ʹ��swthrd/swmutex/swsem/swqueue�����
 * @author tanzongren
 * @date 2006-01-05
 */

#ifndef __SWUTIL_H__
#define __SWUTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* ���ַ�����ʽ��ʱ��ת���ɶ�������ʽ����λ���� 8:30 -> 8*60+30*/
int sw_timezone2minute( char* p_timezone );
	
/* ��������ʱ��(��λ����)ת�����ַ��� */
void sw_minute2timezone( int minute, char* buf );
	
#ifdef __cplusplus
}
#endif

#endif /* __SWUTIL_H__ */
