/** 
 * @file crc32.h
 * @brief 32λCRCУ��
 * @author huanghuaming
 * @date 2005-11-07
 */

#ifndef __CRC32_H__
#define __CRC32_H__

#ifdef __cplusplus
extern "C"
{
#endif
////////////////////////////////////////////////////////////////////////////////////

/** 
 * @brief ����CRCУ��ֵ
 * 
 * @param buf ָ��Ļ�����
 * @param size �������ĳ���
 * 
 * @return �ɹ�,���ؼ�����У��ֵ;����,����-1
 */
unsigned long sw_crc32_get( unsigned char *buf, int size );

/** 
 * @brief ��CRCУ�� 
 * 
 * @param buf ָ��Ļ�����
 * @param size �������ĳ���
 * 
 * @return ��У��ɹ�,����true;����,����false
 */
bool sw_crc32_check( unsigned char *buf, int size );

////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
};
#endif

#endif //__crc32_H__
