/** 
 * @file swlog.h
 * @brief The file defines the interfaces to manipulate logging information.
 *	level ������־�������
 *	target ������־�����Ŀ�꣬���ڣ��ļ���
 *	mods ������־�����ģ�飬��ģ������ת����32λint�����֣�Ȼ��Աȿ��������־��ģ���int�����飬������������������־�����������־ 	
 *
 * Copyright (c) 2006 Beijing Sunniwell Broadband Digital Technologies, Inc. All Rights Reserved.
 * @author Chen Guang / Hongchen Dou / huanghuaming
 * @date 2007-09-06
 * @history:
 *		2010-06-18 chenkai add log modules interface
 */

#ifndef __SWLOG_LINUX_H__
#define __SWLOG_LINUX_H__

/* logging level */
#define	LOG_LEVEL_ALL		0x00	//���������־
#define	LOG_LEVEL_DEBUG		0x01	//ָ��ϸ������Ϣ�¼��Ե���Ӧ�ó����Ƿǳ��а�����
#define	LOG_LEVEL_INFO		0x02	//��Ϣ�ڴ����ȼ�����ͻ��ǿ��Ӧ�ó�������й���
#define	LOG_LEVEL_WARN		0x03	//���������Ǳ�ڴ��������
#define	LOG_LEVEL_ERROR		0x04	//ָ����Ȼ���������¼�������Ȼ��Ӱ��ϵͳ�ļ�������
#define	LOG_LEVEL_FATAL		0x05	//ָ��ÿ�����صĴ����¼����ᵼ��Ӧ�ó�����˳�
#define	LOG_LEVEL_OFF		0x06	//�ر���־���

#define	SYSLOG_LEVEL_ALL		0x00	//���������־
#define	SYSLOG_LEVEL_ERROR		0x03	//���Error�������־
#define	SYSLOG_LEVEL_INFO		0x06	//���Infomation�������־
#define	SYSLOG_LEVEL_DEBUG		0x07	//���Debug�������־

#define LOG_TYPE_ALL 0				//all log type
#define LOG_TYPE_OPERATION 16		//������־
#define LOG_TYPE_RUN 17				//������־
#define LOG_TYPE_SECURITY 19		//��ȫ��־
#define LOG_TYPE_USER 20			//�û���־


#ifdef __cplusplus
extern "C"
{
#endif
	
/** 
 * @brief Initialize the logging API, prepare for logging.
 * 
 * @param level int,  The original level to record the logging infomation. You can use sw_log_set_level() to change it.
 * @param target char*,  The target of all logging infomation.
 * @param mods char*,The modules which allow to ouput log.
 * 
 * @return int , The status of this operation.  0 on success, else on failure.
 */
int sw_log_init( int level, char* targets,char* mods);

/** 
 * @brief Release some resources allocated by sw_log_init().
 */
void sw_log_exit();

/** 
 * @brief set the logging level
 * 
 * @param level int, the logging level to set.
 * @return 0 is success,1 is failed.
 */
int sw_log_set_level( int level );

/** 
 * @brief To get the logging level.
 * 
 * @return int , the logging level has been set previously.
 */
int sw_log_get_level();

/** 
 * @brief set the logging type
 * 
 * @param type int, the logging type to set.
 * @return 0 is success,1 is failed.
 */
int sw_log_set_type( int type );

/** 
 * @brief To get the logging type.
 * 
 * @return int , the logging type has been set previously.
 */
int sw_log_get_type();

/** 
 * @brief set the target of the logging info. It will override the original target list.
 * The new target list has only one target.
 * @param target,char*,the target to set,for example console,file:///tmp/disk1/log.log&max_size=5M
 */
int sw_log_set_targets( char* targets );

/** 
 * @brief Add a target to the target list.
 * 
 * @param target,char*,the target to add.for example console,file:///tmp/disk1/log.log&max_size=5M
 */
int sw_log_add_target( char* target );

/** 
 * @brief Remove a target from the target list.
 * 
 * @param target,char*, the target to del.
 */
int sw_log_del_target( char* target );

/** 
 * @brief Remove all targets from the target list.
 */
void sw_log_clear_alltarget();

/** 
 * @brief return the target list.
 *
 * @return the target list
 */
char* sw_log_get_targets();


/** 
 * @brief set some modules to output log,it will override thr original log modules
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int sw_log_set_mods( char* mods );

/** 
 * @brief Add a log modules  to the log modules list.
 * 
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int sw_log_add_mod( char* mod );

/** 
 * @brief Remove a log modules from the log modules list.
 * 
 * @param mods,char*, the modules to del.
 */
int sw_log_del_mod( char* mod );

/** 
 * @brief Remove all log modules from the log modules list.
 */
int sw_log_clear_allmods();

/** 
 * @brief set to output log of all moduls 
 */
int sw_log_add_allmods();

/** 
 * @brief return the log modules list.
 *
 * @return the log modules list
 */
char* sw_log_get_mods();

/**
 * @brief enable log modules
 */
int sw_log_enable_mod(char *mod, int enable);

/** 
 * @brief Output the logging info.
 * 
 * @param level int, the level of the logging info
 * @param mod  char*,the modduls name
 * @param file  char*,the file name
 * @param line  int,the line in the file
 * @param format char*, format string.
 * @param ... 
 * 
 * @return int , the logging level has been set previously.
 */
int sw_log( int level,const char* mod,const char* file,int line,const char *format, ... ) __attribute__ ((__format__ (__printf__, 5, 6)));

/** 
 * @brief Output the logging info.
 * 
 * @param level int, the level of the logging info
 * @param type int, the type of the logging info
 * @param mod  char*,the modduls name
 * @param file  char*,the file name
 * @param line  int,the line in the file
 * @param format char*, format string.
 * @param ... 
 * 
 * @return int , the logging level has been set previously.
 */
int sw_log_syslog( int level, int type, const char* mod,const char* file,int line,const char *format, ... ) __attribute__ ((__format__ (__printf__, 6, 7)));

/**
 * @brief print current log output info
 */
void sw_log_print_state();

#ifdef __cplusplus
}
#endif

#endif /* __SWLOG_LINUX_H__ */


