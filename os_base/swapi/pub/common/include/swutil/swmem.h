/** 
 * @file swmem.h
 * @brief �ɷֿ�/�ɸ����ڴ�й©λ�õ��ڴ����ʽ
 * @author hujinshui / huanghuaming
 * @date 2005-09-14
 */

#ifndef __SWMEM_H__
#define __SWMEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief ���ѷ�����ڴ���г�ʼ�� 
 * 
 * @param buf 	�ѷ�����ڴ��ַ
 * @param size 	�ѷ����ڴ�Ĵ�С 
 * @param align �ֽڷ���ķ�ʽ
 * 
 * @return �ɹ�,�����ڴ���; ʧ��,���ؿ�ֵ
 */
HANDLE sw_mem_init( char *buf, int size, int align );

/** 
 * @brief �ͷ��ڴ���
 * 
 * @param h �ڴ���
 */
void sw_mem_exit( HANDLE h );

/** 
 * @brief �ͷ��ڴ���, �����ڴ�й¶���, ���ִ��Ч��
 * 
 * @param h �ڴ���
 */
void sw_mem_exit_nocheck( HANDLE h );

/** 
 * @brief �ڴ����
 * 
 * @param h �ڴ���
 */
void sw_mem_reset( HANDLE h );

/** 
 * @brief ����Ƿ���δ�ͷŵ��ڴ� 
 * 
 * @param h �ڴ���
 */
void sw_mem_check( HANDLE h );
/** 
 * @brief ��ȡ�ܵ��ڴ��С
 * 
 * @param h �ڴ���
 * 
 * @return �ܵ��ڴ��С
 */
int sw_mem_get_total_size( HANDLE h );
/** 
 * @brief ȡ��ʷ������ѷ���ߴ�
 * 
 * @param h �ڴ���
 * 
 * @return ����ѷ���ߴ�Ĵ�С
 */
int sw_mem_get_max_size( HANDLE h );
/** 
 * @brief ȡ����������ߴ�(�ڴ���Ƭ̫��ʱ�޷���ȡ׼ȷ���ڴ��С)
 * 
 * @param h �ڴ���
 * 
 * @return ���ڷ���ߴ�Ĵ�С
 */
int sw_mem_get_cur_size( HANDLE h );
/** 
 * @brief �����ڴ��Ƿ���ʹ����(�з������û�ͷŵ��ڴ�)
 * 
 * @param h �ڴ���
 * 
 * @return ���û���ѷ���Ľڵ�,�򷵻���(true);����,���ؼ�(false)
 */
bool sw_mem_is_empty( HANDLE h );

/** 
 * @brief ���ڴ�����ָ����ڴ��з���һ���ڴ�
 * 
 * @param h �ڴ���
 * @param size �����ڴ�Ĵ�С
 * @param filename ���ڵĵ�ǰ�ļ��� 
 * @param line ���ڵĵ�ǰ�к�
 * 
 * @return �ɹ����ط�����ڴ�ĵ�ַ; ����,���ؿ�ֵ
 */
void *sw_mem_alloc( HANDLE h, int size, const char *filename, int line );
/** 
 * @brief 			�ͷ��ڴ�����ָ���һ���ڴ�
 * 
 * @param h 		�ڴ���
 * @param p 		ָ����ڴ��ַ
 * @param filename 	���ڵĵ�ǰ�ļ���
 * @param line 		���ڵĵ�ǰ�к�
 */
void sw_mem_free( HANDLE h, void *p, const char *filename, int line );
/** 
 * @brief ���ڴ���ָ����ڴ��и����ַ���
 * 
 * @param h �ڴ���
 * @param s ָ����ַ���
 * @param filename ���ڵĵ�ǰ�ļ���
 * @param line ���ڵĵ�ǰ�к�
 * 
 * @return �ɹ�,���ظ��ƺ���ַ���ָ��; ����,���ؿ�ֵ
 */
char *sw_mem_strdup( HANDLE h, const char *s, const char *filename, int line );
/** 
 * @brief ��ԭ���ڴ�Ļ��������������ڴ�
 * 
 * @param h �ڴ���
 * @param p ָ��ԭ�е��ڴ�
 * @param size �����ڴ�Ĵ�С
 * @param filename ���ڵĵ�ǰ�ļ���
 * @param line ���ڵĵ�ǰ�к�
 * 
 * @return �ɹ�,����ʵ�ʷ������µ�ַ; ����,���ؿ�ֵ
 */
void *sw_mem_realloc( HANDLE h, void *p, int size, const char *filename, int line );

	
/**
 * @brief ��ʼ���ڴ���������use_mutex����
 */
HANDLE sw_mem_init_ex( char *buf, int size, int align, int use_mutex );

/**
 * @brief ����ڴ��ʹ�����������print_node����, ����˲���Ϊ0��������ӡ�ڴ���ʹ�����, ֻ����Ƿ����
 */
void sw_mem_check_ex( HANDLE h, int print_node );

/**
 * @brief ��ȡ�ڴ�������ڴ��С
 */
int sw_mem_get_size( HANDLE h, void* p );

int sw_mem_get_alloc_size( HANDLE h );

#ifdef __cplusplus
}
#endif

#endif /*__SWMEM_H__*/

