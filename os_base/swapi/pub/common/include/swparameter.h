/**
 * @file swparameter.h
 * @brief �����������ӿ�
 * @author Dou Hongchen / huanghuaming
 * @history 2007-02-14 created
 *			 2011-02-28 chenkai add idepot ����
 */

#ifndef __SW_PARAMETER_H__
#define __SW_PARAMETER_H__

#include "swidepot.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*��������۲���*/
typedef void (*on_para_modified)(char* name,char* value,void* handle);

/** 
 * @brief ��ʼ����������
 *	
 * @param max_buf_size ����ģ�黺�����Ĵ�С���Ƽ�ֵ��128*1024
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_init(int max_buf_size);

/** 
 * @brief �Ƿ��Ѿ���ʼ��
 * 
 * @return true,��ʼ��; false, δ��ʼ��
 */
bool sw_parameter_is_init();

/** 
 * @brief �ͷ���Դ
 */
void sw_parameter_exit();

/**
 *	@brief ע������ֿ�,ע��֮������ֿ���ĺ���ͨ�������ӿڼ��ص�parameter��
 */
bool sw_parameter_register_depot( sw_idepot_t* depot,bool isdefault);

/**
 * @breif �������ƻ�ȡ�����ֿ�
 */
sw_idepot_t* sw_parameter_get_depot(char* name);


/**
 * @brief ж�ز����ֿ⣬ɾ��paraemeter����ڸòֿ���Ĳ���
 */
bool sw_parameter_unregister_depot( char* name );

/*
 * @brief ��ָ����depot���ݷַ���swparameter��
 * @param sw_paradepot_t* p_depot, ��������Ҫ��depot
 * @return �ɹ�����true,ʧ�ܷ���false
 */
bool sw_parameter_updatefrom_depot( sw_idepot_t* depot );

/** 
 * @brief ��������в���, ����Ŀ����load��ʽ����
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_save();


/**
 * @brief ���ò���ֵ�ı�Ļص�����
 */
void sw_parameter_set_observer(on_para_modified on_modified,void* handle);

/** 
 * @brief ��ȡ�ı�����Ĳ���
 * 
 * @param name 
 * @param value 
 * @param size 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_get( char* name, char* value, int size );

/** 
 * @brief �����ı�����Ĳ���
 * 
 * @param name 
 * @param value 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_set( char* name, char* value );

/** 
 * @brief ����һ�����
 * 
 * @param buf 
 * @param size 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_set_group( char *buf, int size );

/** 
 * @brief ��ȡ�ı�����Ĳ���,���ز�������ֵ
 * 
 * @param name 
 * 
 * @return ��������ֵ
 */
int sw_parameter_get_int( char* name );

/** 
 * @brief ����ֵ�����ı�����Ĳ���
 * 
 * @param name 
 * @param value ��������ֵ
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_set_int( char* name, int value );


/** 
 * @brief ���ò�����ȱʡֵ,��������������ӵ��������У�����������ڣ��Ҳ����ֿ�û�иı������κδ���
 *	��������ֿ⼶����ߣ�����Ĳ����ֿ�	
 * 
 * @param name ��������
 * @param value ����ֵ
 * @param depot ָ���Ĳ����ֿ�
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_set_default( char* name, char* value,sw_idepot_t* depot);

/**
 * @brief ���ڴ������һ��Ĭ�ϵĲ�����Ĭ�ϵĲ����ֿ��У����������������Ա����,��������������ӵ�Ĭ�ϵĲֿ���
 *
 * @param buf,��������
 * @param size,�����С
 *
 * return true,���³ɹ�; false,����ʧ��
 */
bool sw_parameter_set_group_default(char *buf, int size);

/** 
 * @brief ���Ӳ�����ָ���Ĳ����ֿ�
 * 
 * @param name ��������
 * @param value ����ֵ
 * @param depotname ָ���Ĳ����ֿ�
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_add( char* name, char* value,char* depotname );

/** 
 * @brief ɾ���ı�����Ĳ���
 * 
 * @param name 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_delete( char* name );

/** 
 * @brief ������в���
 */
void sw_parameter_delete_all();

/** 
 * @brief ȡ�õ�һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_first( unsigned long *pos );

/** 
 * @brief ȡ����һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_next( unsigned long *pos );

/** 
 * @brief ȡ����һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_prev( unsigned long *pos );

/** 
 * @brief ȡ�����һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_last( unsigned long *pos );

/** 
 * @brief ȡ�ò�����POS����,����NULL��ʾû���ҵ�
 * 
 * @param name 
 * @param pos 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_get_pos( char *name, unsigned long *pos );

/** 
 * @brief ����POS��ȡ����,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * @param *value 
 * 
 * @return 
 */
char* sw_parameter_get_by_pos( unsigned long pos, char **value );

/** 
 * @brief ȡ�ò�������
 * 
 * @return 
 */
int sw_parameter_get_num();

/**
 * @brief ����ĳ������ֻ��
 *
 * @param name ��������
 * @param readonly �Ƿ�ֻ�� 
 *
 * return �ɹ�����true��ʧ�ܷ���false
 */
bool sw_parameter_set_readonly(char* name,bool readonly);


/**
 * @brief ȡ��ĳ��������ֻ��״̬
 *
 * @param name ��������
 *
 * return ����ֻ��״̬
 */
bool sw_parameter_get_readonly(char* name);

/**
 * @brief ���ò����Ƿ�ʵʱ��ȡ:Ŀǰֻ��java����,Ĭ�ϵ�java���ݿ��������ʵʱ��ȡ��
 *
 * @param name Ҫʵʱ��д�Ĳ�������
 *
 * return �ɹ�����true��ʧ�ܷ���false
 */
bool sw_parameter_set_realtime(char* name,bool realtime);

/**
 * @brief ǿ���޸Ķ�Ӧ�Ĳ����ֿ�λ��,��������ֻ������,java������ʵʱ��ȡ����,�����java�����Ļ���Ҫ��APK���ݿ���²���ֵ,
 					���������λ�ø���,ԭʼ��ֵ�Ļ����޸Ĳ���ֵ��������ΪĬ��ֵ,
 					�˽ӿ�ֻ������app��ʼ��ʱ����
 *
 * @param name,������
 * @param defaultvalue,����Ĭ��ֵ
 * @param readonly,����ֻ��
 * @param realtime,ʵʱ��ȡ
 * @param depot,ʵʱ��ȡ
 *
 * return true,������λ�øı�󷵻�true
 */
bool sw_parameter_update_with_depot(char *name, char *defaultvalue, bool readonly, bool realtime, sw_idepot_t* depot);

/**
 * @brief ˢ��java�������е�ֻ���ͷ�ʵʱ��ȡ�Ĳ���
 */
void sw_parameter_refresh(void);

#ifdef __cplusplus
}
#endif

#endif /*__SW_PARAMETER_H__*/
