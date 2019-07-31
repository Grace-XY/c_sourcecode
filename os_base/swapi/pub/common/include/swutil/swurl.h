/** 
 * @file swurl.h
 * @brief ����һ��URL�������ܽӿ�
 * @author sunniwell
 * @date 2005-01-05 created 
 */

#ifndef __SWURL_H__
#define __SWURL_H__


#ifdef __cplusplus
extern "C"
{
#endif
	
typedef struct _sw_url
{
    /** 
     * @brief URLͷ '// :'֮ǰ��һ����
     */
	char head[32];	
	char user[32];
	char pswd[32];
	/** 
	 * @brief URL ������Ϣ(������IP)
	 */
	char hostname[256];
	/** 
	 * @brief has ipv4 IP
	 */
	bool has_ipv4;
	/** 
	 * @brief has ipv6 IP
	 */
	bool has_ipv6;
	/** 
	 * @brief URLָ���IP�������ֽ���
	 */
	uint32_t ip;
	/** 
	 * @brief URLָ���IP��ipv6
	 */
	struct in6_addr ipv6;
	/** 
	 * @brief URL�˿ڣ������ֽ���
	 */
	uint16_t port;
	/** 
	 * @brief URL path
	 */
	char path[1024];		
	/** 
	 * @brief URL���һ����
	 */
	char tail[256];
	/** 
	 * @brief URL���һ���ʵĺ�׺
	 */
	char suffix[32];
    /**
     * @brief hostname[:port]
     */
	char host[256];
}sw_url_t;

/** 
 * @brief ����URL,
 * 
 * @param dst ָ�������Ľ��
 * @param url ָ���ԴURL
 * 
 * @return 0��ʾ�����ɹ�,-1����ʧ��
 */
int sw_url_parse(sw_url_t* dst, const char* url);

/** 
 * @brief ȡ��URL�еĲ���
 * 
 * @param url ָ���ԴURL
 * @param name ָ��Ĳ�����
 * @param value ָ��Ĳ���ֵ
 * @param valuesize ָ�����ֵ�ĳ���
 * 
 * @return �ɹ�,����ָ��Ĳ���ֵ; ����,����NULL
 */
char* sw_url_get_param_value( const char* url, const char* name, char *value, int valuesize );

/** 
 * @brief ��URL����ȡ����
 * 
 * @param url ָ���ԴURL
 * @param name ָ��Ĳ�����
 * 
 * @return �ɹ�,����ָ��Ĳ���ֵ; ����,����NULL
 */
char* sw_url_get_param(char* url,char* name);

/** 
 * @brief ��URL����ȡ����
 * 
 * @param url ָ���ԴURL
 * @param name ָ��Ĳ�����
 * 
 * @return �ɹ�,���ز���������ֵ; ����,����-1
 */
int sw_url_get_param_int(const char* url, const char* name);

/** 
 * @brief ��URL����ȡpath
 * 
 * @param url ָ���ԴURL
 * @param path ������
 * @param size ����������
 * 
 * @return ���ػ�������ַ
 */
char *sw_url_get_path(const char *url, char *path, int size);

/** 
 * @brief ��URL����ȡheader
 * 
 * @param url ָ���ԴURL
 * @param header ������
 * @param size ����������
 * 
 * @return ���ػ�������ַ
 */
char *sw_url_get_header(const char *url, char *header, int size);

/** 
 * @brief ��URL���б���
 * 
 * @param in ָ��������ַ���
 * @param out ָ�������ַ���
 * 
 * @return �ɹ�,����0; ����,����-1
 */
int sw_url_encode(const char* in,char* out);

/** 
 * @brief ��URL���б���
 * 
 * @param in ָ��������ַ���
 * @param out ָ�������ַ���
 * @param outsize ָ�������ַ�����󳤶�
 * 
 * @return �ɹ�,����0; ����,����-1
 */
int sw_url_encode_ex(const char* in, char* out, int outsize);

#ifdef __cplusplus
}
#endif

#endif /* __SWURL_H__ */
