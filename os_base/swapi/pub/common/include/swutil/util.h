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

/** 
 * @brief �������ַ�ת����Сд
 * 
 * @param buf ָ���Դ�ַ���
 * 
 * @return �ɹ�,����Сд�ַ���;����,����NULL
 */
char *strlower( char * buf );

/** 
 * @brief �������ַ�ת���ɴ�д
 * 
 * @param buf ָ���Դ�ַ���
 * 
 * @return �ɹ�,���ش�д�ַ���;����,����NULL
 */
char *strupper(char * buf);

/** 
 * @brief ��ȡָ����ʽ��ʱ��
 * 
 * @param fmt ָ���ת����ʽ������
 * 
 * @return �ɹ�,����ת����ʱ��;����,����NULL
 */
char *getfmtdatetime(char *fmt);
/*  */
/** 
 * @brief ȥ���ַ����е�ָ���ַ�
 * 
 * @param str ָ���Դ�ַ���
 * @param trim ָ��ķָ��
 *
 * @return �ɹ�,���طָ����ַ���;����,����NULL
 */
char *strtrim(char *str, const char *trim);
char *strrtrim(char *str, const char *trim);
char *strltrim(char *str, const char *trim);

/** 
 * @brief �ַ����Ƚ�(�����ֵĴ�Сд)
 * 
 * @param cs ָ��ĵ�һ���ַ���
 * @param ct ָ��ĵڶ����ַ���
 * 
 * @return ������cs��ct�ַ�����ͬ,����0;��cs����ct,�򷵻ش���0��ֵ;��csС��ct,�򷵻�С��0��ֵ
 */
int stricmp(const char * cs,const char * ct);
char *get_last_path_component(char *path);
int sw_crc32(register int fd, uint32_t *main_val, unsigned long *main_len);


/* ���ַ�����ʽ��ʱ��ת���ɶ�������ʽ����λ���� 8:30 -> 8*60+30*/
int TimezoneToMinute( char* szTimezone );
	
/* ��������ʱ��(��λ����)ת�����ַ��� */
void MinuteToTimezone( int minute, char* buf );
	
#ifdef __cplusplus
}
#endif

#endif /* __SWUTIL_H__ */
