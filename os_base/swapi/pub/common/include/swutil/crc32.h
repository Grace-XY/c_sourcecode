/** 
 * @file crc32.h
 * @brief 32位CRC校验
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
 * @brief 计算CRC校验值
 * 
 * @param buf 指向的缓冲区
 * @param size 缓冲区的长度
 * 
 * @return 成功,返回计算后的校验值;否则,返回-1
 */
unsigned long sw_crc32_get( unsigned char *buf, int size );

/** 
 * @brief 做CRC校验 
 * 
 * @param buf 指向的缓冲区
 * @param size 缓冲区的长度
 * 
 * @return 若校验成功,返回true;否则,返回false
 */
bool sw_crc32_check( unsigned char *buf, int size );

////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
};
#endif

#endif //__crc32_H__
