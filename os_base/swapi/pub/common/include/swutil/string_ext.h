/** 
 * @file string_ext.h
 * @brief ����string����չ����
 * @author sunniwell
 * @date 2006-01-15
 */

#ifndef __STRING_EXT_H__
#define __STRING_EXT_H__

#ifdef __cplusplus
extern "C"
{
#endif
/** 
 * @brief �����ִ�Сд�Ƚ��ַ�����ǰn���ַ�
 * 
 * @param s1 ָ��ĵ�һ���ַ���
 * @param s2 ָ��ĵڶ����ַ���
 * @param n �μӱȽϵ��ַ���
 * 
 * @return ������s1��s2�ַ�����ͬ�򷵻�0��s1������s2�򷵻ش���0��ֵ��s1��С��s2�򷵻�С��0��ֵ
 */
int xstrncasecmp(const char *s1, const char *s2, size_t n);

/** 
 * @brief �����ִ�С�Ƚ��ַ���
 * 
 * @param s1 ָ��ĵ�һ���ַ���
 * @param s2 ָ��ĵڶ����ַ���
 * 
 * @return ������s1��s2�ַ�����ͬ�򷵻�0��s1������s2�򷵻ش���0��ֵ��s1��С��s2�򷵻�С��0 ��ֵ
 */
int xstrcasecmp(const char *s1, const char *s2);

/** 
 * @brief ���ַ�����delim������ַ��ָ�����طָ��ĵ�һ����
 * 
 * @param stringp ָ�����ָ���ַ���
 * @param delim �ַ����ָ��
 * 
 * @return ������һ���ָ����ַ���ָ�룬������޴ӷָ��򷵻�NULL
 */
char* xstrsep(char** stringp, const char *delim);

/** 
 * @brief �����size���ַ����뵽�ַ���
 * 
 * @param str ָ����ַ�������
 * @param size ���Ƶ��ֽڸ���
 * @param format ��ָ�ַ�����ת������ʽ������
 * @param ... ����һ�����͵Ĳ���
 * 
 * @return �ɹ��򷵻ز���str�ַ������ȣ�ʧ���򷵻�-1
 */
int xsnprintf(char *str, size_t size, const char *format, ...);
/** 
 * @brief �����size���ַ����뵽�ַ���
 * 
 * @param s ָ����ַ�������
 * @param buf_size ���Ƶ��ֽڸ���
 * @param format ��ָ�ַ�����ת������ʽ������
 * @param ap �ɱ�����б�
 * 
 * @return �ɹ��򷵻ز���s�ַ������ȣ�ʧ���򷵻�-1
 */
int xvsnprintf(char *s, size_t buf_size, const char* format, va_list ap);


/** 
 * @brief ��ȡ�ֶ�ֵ
 * 
 * @param buf ָ���Դ�ַ���
 * @param name ��ָ�Ĳ�����
 * @param value ��ָ�Ĳ���ֵ
 * @param valuelen ��ָ����ֵ�ĳ���
 * 
 * @return �ɹ�,����ָ��Ĳ���ֵ; ����,����NULL
 */
char *xstrgetval( char *buf, char *name, char *value, int valuelen );

/*
 @brief:�ж�������ַ����Ƿ�������
 */
int xstrisdigit(char* str);

#ifdef __cplusplus
}
#endif

#endif /* __STRING_EXT_H__ */

