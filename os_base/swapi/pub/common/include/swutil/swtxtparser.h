/** 
 * @file txtparser.h
 * @brief ���ı���ʽ���з���
 * @author chenkai
 * @date 2005-10-18 created
 */

#ifndef __TXTPARSER_H__
#define __TXTPARSER_H__


/* �ҵ���һ����Ϊ' '���ַ� */
#define ADV_SPACE(a) {while (isspace(*(a)) && (*(a) != '\0'))(a)++;}

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief �����ַ����ǲ���ip��ַ,�����������ж��Ƿ�ʱ�Ϸ���IP��ַ��,��0.0.0.1����Ϊʱ�Ϸ���ip��ַ
 * 
 * @param buf ָ����ַ���
 * 
 * @return ����������ip��ַ,����true; ����,����false
 */
 bool sw_txtparser_is_address( char* buf );

/** 
 * @brief ��ȡ��ʽ���������ߵ��ַ���
 * 
 * @param equations ָ���Դ�ַ���
 * @param pleft ָ��ĵ�ʽ����ַ���
 * @param pright ָ��ĵ�ʽ�ұ��ַ���
 * 
 * @return 
 */
char* equation_parse(char* equations,char** pleft,char** pright);

/** 
 * @brief ��ȡ��ʽ���������ߵ��ַ���
 * 
 * @param equations ָ���Դ�ַ���
 * @param pleft ָ��ĵ�ʽ����ַ���
 * @param pright ָ��ĵ�ʽ�ұ��ַ���
 * 
 * @return 
 */
char* equation_parse_as_line(char* equations,char** pleft,char** pright);

/** 
 * @brief �������ַ���ת��Ϊ2��������
 * 
 * @param string ָ��������ַ���
 * @param length �����ַ����ĳ���
 * @param binary ָ���2��������
 * @param binsize 2��������ĳ���
 * 
 * @return �ɹ�,����0; ����,����-1
 */
int  txt2hex(const char* string, int length,uint8_t* binary,int binsize);

/** 
 * @brief �Ƿ���һ������
 * 
 * @param srcint ָ��һ����ֵ
 * 
 * @return ��������,����true; ����,����false
 */
bool sw_txtparser_is_int( char* srcint );

/** 
 * @brief ������Ч�Լ���Ƿ���Mac
 * 
 * @param srcMac ָ����ַ���
 * 
 * @return ����Mac��ַ,����true; ����,����false
 */
bool sw_txtparser_is_mac_address( char* src_mac );

/** 
 * @brief �Ƿ��Ǵ���
 * 
 * @param srcBand ָ����ַ���
 * 
 * @return ���Ǵ���,����true; ����,����false
 */
bool sw_txtparser_is_band( char* src_band );

/** 
 * @brief ������Ч�Լ�� �Ƿ���һ��port
 * 
 * @param srcport ָ����ַ���
 * 
 * @return ����port,����true; ����,����false
 */
bool sw_txtparser_is_port( char* src_port );

/** 
 * @brief ������Ч�Լ�� �Ƿ�������ֵ
 * 
 * @param srcVol ָ����ַ���
 * 
 * @return ��������ֵ,����true; ����,����false
 */
 bool sw_txtparser_is_vol( char* src_val );

/** 
 * @brief ������Ч�Լ�� �Ƿ���netmode
 * 
 * @param srcNetmode ָ����ַ���
 * 
 * @return ����netmode,����true; ����,����false
 */
bool sw_txtparser_is_net_mode( char* src_net_mode );

#ifdef __cplusplus
}
#endif


#endif //__TXTPARSER_H__
