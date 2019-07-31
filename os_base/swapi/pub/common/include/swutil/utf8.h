#ifndef __UTF8_H__
#define __UTF8_H__

/**
 * @brief ��ȡUTF8�ĵ����ֽ���ռ����
 * @return ��utf8���س��ȣ�����0
 */
int UTF8_CharLen(unsigned char* utf8_text);
/**
 * @brief ��������UTF8����,�����ؿ������ڴ泤��
 *
 * @param utf8_dest Ŀ���ַ
 *		utf8_srcԴutf8�Ӵ���ʼ��ַ
 *
 * @return ����ʵ�ʿ�����utf8_src�ĸ���.ʧ�ܷ���0
 */
int UTF8_charcpy(char *utf8_dest, char* utf8_src);

/** 
 * @brief UTF-8תUnicode�������unicode_text��
 * 
 * @param utf8_text ָ���Դutf8�ַ���
 * @param unicode_text ָ��ת�����unicode�ַ���
 * 
 * @return ����ʵ��ʹ�õ�utf8_text�ĸ���.�������0,Ҫע���ų�����
 */
int UTF8_Unicode( unsigned char* utf8_text, unsigned short* unicode_text );
/** 
 * @brief ��utf8�ַ����ĳ���
 * 
 * @param utf8_text ָ���Դutf8�ַ���
 * 
 * @return ����utf8�ַ����ĳ���
 */
int UTF8_strlen(  unsigned char* utf8_text );
/** 
 * @brief ��utf8�ַ����в���һ��unicode��
 * 
 * @param utf8_text ָ���Դutf8�ַ���
 * @param unicode ���ҵ�unicode��
 * 
 * @return ���ҳɹ�,����unicode����ֵ�λ��;����,����NULL
 */
char* UTF8_strchr( unsigned char* utf8_text, unsigned short unicode );
/** 
 * @brief ��utf8�ַ����в�����һ���ַ���
 * 
 * @param utf8_text ָ���Դutf8�ַ���
 * @param utf8_child_str ���ҵ�utf8�ַ���
 * 
 * @return ���ҳɹ�,����utf8_child_str��utf8_text��һ�γ��ֵ�λ��;����,����NULL
 */
char* UTF8_strstr( unsigned char* utf8_text, unsigned char* utf8_child_str );
/** 
 * @brief �Ƚ�����utf8�ַ���
 * 
 * @param utf8_text1 ָ��ĵ�һ���ַ���
 * @param utf8_text2 ָ��ĵڶ����ַ���
 * 
 * @return �����ַ�����ͬ,����0;��utf8_text1����utf8_text2,�򷵻ش���0��ֵ;��utf8_text1С��utf8_text2,�򷵻�С��0��ֵ
 */
int UTF8_strcmp( unsigned char* utf8_text1, unsigned char* utf8_text2 );
/** 
 * @brief �����ַ���
 * 
 * @param utf8_dest ָ���Ŀ�Ļ�����
 * @param utf8_src ָ���Դ�ַ���
 * 
 * @return �ɹ�,����Ŀ�Ļ������ĵ�ַ;����,����NULL
 */
char* UTF8_strcpy( char* utf8_dest, char* utf8_src );
/** 
 * @brief �ַ�������
 * 
 * @param utf8_dest ָ���Ŀ�Ļ�����
 * @param utf8_src ָ���Դ�ַ���
 * 
 * @return �ɹ�����,����Ŀ�Ļ������ĵ�ַ;����,����NULL
 */
char* UTF8_strcat( char* utf8_dest, char* utf8_src );
/** 
 * @brief �����ַ�����N���ֽ�
 * 
 * @param utf8_dest ָ���Ŀ�Ļ�����
 * @param utf8_src ָ���Դ�ַ���
 * @param len ���Ƶ��ֽ���Ŀ
 * 
 * @return �ɹ�,����Ŀ�Ļ������ĵ�ַ;����,����NULL
 */
char* UTF8_strncpy( char* utf8_dest, char* utf8_src, int len );


/** 
 * @brief ��һ���ַ����в���unicode�ַ�,ֻҪ�ҵ�һ���ͷ����ҵ���λ��,��UTF8_strchr������
 * 
 * @param utf8_text ָ���Դutf8�ַ���
 * @param unicode ���ҵ�unicode�ַ�
 * @param count 
 * 
 * @return ���ҳɹ�,�����ҵ���λ��;����,����NULL
 */
char* UTF8_FindMultiChar( unsigned char* utf8_text, unsigned short* unicode, int count );

/** 
 * @brief ��UTF8_FindMultiChar��������,��Ҫָ���ַ����ĳ���
 * 
 * @param utf8_text ָ���Դutf8�ַ���
 * @param maxlen Դutf8�ַ����ĳ���
 * @param unicode ���ҵ�unicode�ַ�
 * @param count 
 * 
 * @return ���ҳɹ�,�����ҵ���λ��;����,����NULL
 */
char* UTF8_FindMultiCharEx( unsigned char* utf8_text, int maxlen, unsigned short* unicode, int count );

/** 
 * @brief ȥ����ĩָ�����ַ�,�����ַ���ʼλ��
 * 
 * @param utf8_text ָ���Դutf8�ַ���
 * @param len Դutf8�ַ����ĳ���
 * @param unicode ��Ҫȥ����unicode�ַ�
 * @param count 
 * 
 * @return �ɹ�,����ȥ������ַ���;����,����NULL
 */
char* UTF8_ExceptRChar( unsigned char* utf8_text, int len, unsigned short* unicode, int count );

#endif	/*__UTF8_H__*/

