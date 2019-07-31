/** 
 * @file swurl_vsprintf.h
 * @brief ��������url����snprintf����
 * @author sunniwell
 * @date 2014-10-06
 */

#ifndef __SWURL_VSPRINTF_H__
#define __SWURL_VSPRINTF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
 @brief:��������url������ͨvsnprintf�������ǣ���Ὣurl�еķǴ�ӡ�ַ����� '%'��'?'��'&'��'@'��':'��'/'��';' ���߸������ַ�ת���� %XX ����ʽ������XXΪʮ����������Ҳ���Ǹ������ַ���ASCII��
 */
int sw_url_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/*
 @brief:��������url������ͨsnprintf�������ǣ���Ὣurl�еķǴ�ӡ�ַ����� '%'��'?'��'&'��'@'��':'��'/'��';' ���߸������ַ�ת���� %XX ����ʽ������XXΪʮ����������Ҳ���Ǹ������ַ���ASCII��
 */
int sw_url_snprintf(char * buf, size_t size, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __SWURL_VSPRINTF_H__ */

