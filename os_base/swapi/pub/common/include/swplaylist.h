/**
*  �����в����б�ӿ�
* 
* <p>
* Copyright (c) 2006 Beijing Sunniwell Broadband Digital Technologies, Inc. All  Rights Reserved.
*
* @author  Dou Hongchen /huanghuaming
* @version 2.0
* @date 2007.02.14
*/

#ifndef __SWPLAYLIST_H__
#define __SWPLAYLIST_H__

/* ��������:service_type���μ�service_descriptor, 0x80-0xfe���Զ����ֶΣ�
   Ϊ����ֱ������DVB��׼�й���service_type�ı����ֶ�������IP��Ŀ */
#define	SERVICE_TYPE_NULL				0x0	/* δ֪ */
#define	SERVICE_TYPE_DIGITAL_TV			0x01/* ���ֵ��� */
#define	SERVICE_TYPE_DIGITAL_RADIO		0x02/* ������Ƶ�㲥 */
#define	SERVICE_TYPE_TELETEXT			0x03/* ͼ�ĵ��� */
#define	SERVICE_TYPE_NVOD_REFRENCE		0x04/* NVOD���� */
#define	SERVICE_TYPE_NVOD_TIME_SHIFT	0x05/* NVODʱ�� */
#define	SERVICE_TYPE_MOSAIC				0x06/* ������ */
#define	SERVICE_TYPE_PAL_SIGNAL			0x07/* PAL�ź� */
#define	SERVICE_TYPE_SECAM_SIGNAL		0x08/* SECAM�ź� */
#define	SERVICE_TYPE_D_MAC				0x09
#define	SERVICE_TYPE_FM_RADIO			0x0a/* ��Ƶ�㲥 */
#define	SERVICE_TYPE_NTSC_SIGNAL		0x0b/* NTSC�ź� */
#define	SERVICE_TYPE_DATA_BROADCAST		0x0c/* ���ݹ㲥 */

/* �������Զ������ͣ�ע�ⲻҪ����Ӫ�̶����ͻ */
#define	SERVICE_TYPE_USER				0x80/* �Զ�������: 0x80-0xfe */
#define SERVICE_TYPE_IP_AV				(SERVICE_TYPE_USER+1)/* IP����ƵƵ�� */
#define SERVICE_TYPE_IP_PAGE			(SERVICE_TYPE_USER+2)/* IP-WEBҳ��Ƶ�� */

#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @brief ��ʼ��PLAYLIST
 * 
 * @param size��ʼ�������ڴ��С
 *
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_init(int max_buf_size, int max_chnnl_num);

/** 
 * @brief �ͷ���Դ
 */
void sw_playlist_exit();

/** 
 * @brief �Ƿ��Ѿ���ʼ��
 * 
 * @return true,��ʼ��; false, δ��ʼ��
 */
bool sw_playlist_is_init();

/** 
 * @brief ���ļ������벥���б�
 * 
 * @param file 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_load_from_file( char *file );

/** 
 * @brief �����ڴ��е�PLAYLIST��file
 * 
 * @param file 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_save_to_file( char *file );

/** 
 * @brief ��FLASH�м��ز����б�
 * 
 * @param addr 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_load_from_flash( int addr );

/** 
 * @brief ���沥���б�FLASH
 * 
 * @param addr 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_save_to_flash( int addr );

/** 
 * @brief ���򲥷��б�
 * 
 * @param b_descend 
 */
void sw_playlist_sort( bool b_descend );

/** 
 * @brief ��ȡƵ����URL
 * 
 * @param chnl 
 * @param url 
 * @param size 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_get( int chnl, char *url, int size );


/** 
 * @brief ��ȡƵ����URL
 * 
 * 
 * @return ����Ƶ���� 
 */
int sw_playlist_get_max_chnnl();

/** 
 * @brief ����Ƶ����URL
 * 
 * @param chnl 
 * @param url 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_set( int chnl, char *url );

/** 
 * @brief ����Ƶ����URL
 * 
 * @param chnl 
 * @param url 
 * 
 * @return chnl num
 */
int sw_playlist_set_chnl( int chnl, char *url );

/** 
 * @brief ����һ��Ƶ��URL
 * 
 * @param buf 
 * @param size 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_set_group( char *buf, int size );

/** 
 * @brief ɾ��Ƶ����ֻ�ڻ������в���������sw_playlist_Save()����
 * 
 * @param chnl 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_delete( int chnl );

/** 
 * @brief �����ǰ�Ĳ����б�ֻ�ڻ������в���������sw_playlist_Save()����
 */
void sw_playlist_delete_all();

/** 
 * @brief ȡ�õ�һ��Ƶ����,����-1��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_first( unsigned long *pos );

/** 
 * @brief ȡ����һ��Ƶ����,����-1��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_next( unsigned long *pos );

/** 
 * @brief ȡ����һ��Ƶ����,����-1��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_prev( unsigned long *pos );

/** 
 * @brief ȡ�����һ��Ƶ����
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_last( unsigned long *pos );

/** 
 * @brief ȡ��Ƶ���ŵ�POS����
 * 
 * @param chnl 
 * @param pos 
 * 
 * @return true,�ɹ�; false,ʧ��
 */
bool sw_playlist_get_pos( int chnl, unsigned long *pos );

/** 
 * @brief ����POS��ȡƵ��
 * 
 * @param pos 
 * @param *url 
 * 
 * @return 
 */
int sw_playlist_get_by_pos( unsigned long pos, char **url );

/* 
 ����Ƶ���Ż�ȡurl
 */
int sw_playlist_get_by_chnl( int chnl,char **url );

/*�����кŻ�ȡƵ��url
 *return Ƶ����Ӧ��chnl
 */
int sw_playlist_get_by_line_num( int line_num,char **url);

 /** 
 * @brief ȡ��Ƶ������
 * 
 * @return 
 */
int sw_playlist_get_num();
//����url���Ҷ�Ӧ��Ƶ����
int sw_playlist_find_url(char *url);
//����tsid����ȡ�б�����ͬtsid����Ŀ
int sw_playlist_get_url_number(int tsid);
//�ú�����������ƥ���url��ȡ�������жԱ�ˢ��
bool sw_playlist_cmp_channel_url(char *url,int k);

bool sw_playlist_swap(int now,int new);

bool sw_playlist_move(int now,int new);


void sw_playlist_sort_by_name(bool b_descend);


#ifdef __cplusplus
}
#endif

#endif /* __SWPLAYLIST_H__ */
