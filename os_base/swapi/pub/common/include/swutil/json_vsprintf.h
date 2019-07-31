/** 
 * @file json_vsprintf.h
 * @brief �����γ�json����snprintf����
 * @author sunniwell
 * @date 2014-10-06
 */

#ifndef __JSON_VSPRINTF_H__
#define __JSON_VSPRINTF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
 @brief:�����γ�json��������ͨvsnprintf�������ǣ���Ὣjson���еĵ����š�˫���š���б�򡢻س���������5���ַ�����ת��
 */
int json_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/*
 @brief:�����γ�json��������ͨsnprintf�������ǣ���Ὣjson���еĵ����š�˫���š���б�򡢻س���������5���ַ�����ת��
 */
int json_snprintf(char * buf, size_t size, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __JSON_VSPRINTF_H__ */

