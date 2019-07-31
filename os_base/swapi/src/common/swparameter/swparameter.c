/**
 * @file swparameter.c
 * @brief ʵ�ֲ�������ӿ�
 * @author Dou Hongchen / huanghuaming
 * @history 2007-02-14 created
 *			2011-01-20 liuchunrui ʵ���µĲ�������
 *			2011-02-28 chenkai �Դ����������������
 */
#include "swapi.h"
#include "swlog.h"
#include "swmem.h"
#include "swmutex.h"
#include "swthrd.h"
#include "swsignal.h"
#include "swparameter.h"

/* ���������� */
#define MAX_PARA_NUM		2048
/*�������ֿ�ĸ���*/
#define MAX_DEPOT_NUM		8
/*�����۲��ߵ������Ŀ*/
#define MAX_OBSERVER_NUM    4

typedef struct
{
	char* name;
	char* value;
	bool  readonly;
	bool	realtime;	/* ʵʱ���� */
	sw_idepot_t* depot;
}para_desc_t;

/* �����ı�ص�����*/
typedef struct
{
	on_para_modified  on_modified;
	void* handle;
}para_observer_t;

typedef struct
{
	char* buf;
	HANDLE mem;
	HANDLE thrd; //���������߳�
	HANDLE signal;
	HANDLE mutex;
	uint32_t timeout;
	sw_idepot_t* depots[MAX_DEPOT_NUM]; //�����ֿ�����
	sw_idepot_t* defdepot;	//��û��ָ�������ֿ�ʱ�����ӵĲ�����������ֿ���
	int dcount;	//ʵ�ʲֿ����Ŀ
	para_observer_t observers[MAX_OBSERVER_NUM]; //ע��Ĳ���ֵ�ı�Ļص�����
	int ocount;	//�ص���������Ŀ
  para_desc_t plist[MAX_PARA_NUM]; //�����б�
	int pnum;	//�����б��в�������Ŀ		
	bool exit;	//�Ƿ��˳������߳�
	bool saveall;//�������б���Ĳ���
	bool saveauto;//�����Զ�����Ĳ���
}sw_parameter_t;

static sw_parameter_t  m_parameter;

/* ���Ҳ������ڵ���� */
static int sw_parameter_find( char *name );
/*ͨ���ֿ������Ҳ����ֿ�*/
static int depot_find_byname(char* name);
/*����һ������*/
static bool sw_parameter_set_with_depot(char* name,char* value,sw_idepot_t* depot,bool is_load,bool only_default);
//���������߳�
static bool param_save_proc(uint32_t wparam,uint32_t lparam);
static void insert_qsort();

/** 
 * @brief ��ʼ����������
 * 
 * @param max_buf_size ����ģ�黺�����Ĵ�С���Ƽ�ֵ��128*1024
 *
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_init(int max_buf_size)
{
	memset(&m_parameter,0,sizeof(sw_parameter_t));
	m_parameter.buf  = (char*)malloc(max_buf_size);

	if( m_parameter.buf  == NULL || max_buf_size <=0  )
		goto ERROR_EXIT;

	m_parameter.mem = sw_mem_init(m_parameter.buf,max_buf_size,4);
	if(m_parameter.mem == NULL)
		goto ERROR_EXIT;

	m_parameter.timeout = 300;			

	m_parameter.mutex = sw_mutex_create();
	m_parameter.signal = sw_signal_create(0,1);

	if( m_parameter.mutex == NULL  || m_parameter.signal == NULL)	
		goto ERROR_EXIT;

	m_parameter.thrd = sw_thrd_open( "tParaSaveProc", 80, 
                     	SW_SCHED_NORMAL,64 * 1024,param_save_proc,(uint32_t)(&m_parameter),0);

	if(m_parameter.thrd == NULL )
		goto ERROR_EXIT;
	
	sw_thrd_resume(m_parameter.thrd);

	return true;
ERROR_EXIT:
	if( m_parameter.thrd != NULL )
	{
		sw_thrd_close(m_parameter.thrd,300);
		m_parameter.thrd = NULL;
	}
	
	if(m_parameter.signal!= NULL )
	{
		sw_signal_destroy(m_parameter.signal);
		m_parameter.signal = NULL;
	}

	if(m_parameter.mutex != NULL )
	{
		sw_mutex_destroy(m_parameter.mutex);
		m_parameter.mutex = NULL;
	}

	if( m_parameter.mem)
	{
		sw_mem_exit(m_parameter.mem);
		m_parameter.mem = NULL;
	}

	if(m_parameter.buf )
	{
		free(m_parameter.buf);
		m_parameter.buf = NULL;
	}

	printf("sw_parameter_init failed ..\n");
	return false;
}

/** 
 * @brief �Ƿ��Ѿ���ʼ��
 * 
 * @return true,��ʼ��; false, δ��ʼ��
 */
bool sw_parameter_is_init()
{
	return ( m_parameter.mem != NULL );
}

/** 
 * @brief �ͷ���Դ
 */
void sw_parameter_exit()
{
	m_parameter.exit = true;
	if( m_parameter.signal != NULL )
		sw_signal_give(m_parameter.signal);
	
	sw_thrd_delay(50);
		
	if( m_parameter.thrd != NULL )
	{
		sw_thrd_close(m_parameter.thrd,400);
		m_parameter.thrd = NULL;
	}
	
	if(m_parameter.signal!= NULL )
	{
		sw_signal_destroy(m_parameter.signal);
		m_parameter.signal = NULL;
	}

	if(m_parameter.mutex != NULL )
	{
		sw_mutex_destroy(m_parameter.mutex);
		m_parameter.mutex = NULL;
	}

	if( m_parameter.mem)
	{
		sw_mem_exit_nocheck(m_parameter.mem);
		m_parameter.mem = NULL;
	}

	if(m_parameter.buf )
	{
		free(m_parameter.buf);
		m_parameter.buf = NULL;
	}
}


/**
 *	@brief ע������ֿ�,ע��֮������ֿ���ĺ���ͨ�������ӿڼ��ص�parameter��
 */
//�������صĴ�����
static int on_spread_para(sw_idepot_t *depot,char* name,char* value)
{
	sw_parameter_set_with_depot(name,value,depot,true,false);
	return 0;
}

bool sw_parameter_register_depot(sw_idepot_t* depot,bool isdefault)
{
	int  i = 0;

	if( depot == NULL )
		return false;
	
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	//���depotָ����ͬ���ظ�����
	for( i=0;i<m_parameter.dcount;i++)
	{
		if( m_parameter.depots[i] == depot )
			break;
	}
	if( i < m_parameter.dcount) 
	{
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		return true;
	}

	//�������򷵻�
	if(m_parameter.dcount >= MAX_DEPOT_NUM )
	{
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		return false;
	}
	
	i = m_parameter.dcount;
	m_parameter.depots[i] = depot;
	m_parameter.dcount++;
	if( isdefault || m_parameter.defdepot == NULL )
		m_parameter.defdepot = depot;
	
	if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);

	depot->load(depot,on_spread_para);
	return true;
}

/**
 * @breif �������ƻ�ȡ�����ֿ�
 */
sw_idepot_t* sw_parameter_get_depot(char* name)
{
	int t = -1;
	sw_idepot_t* depot = NULL;

	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	t =  depot_find_byname(name);
	if( t >= 0 && t< m_parameter.dcount )
		depot = m_parameter.depots[t];
	
	if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
	
	return depot;
}

/**
 * @brief ж�ز����ֿ⣬ɾ��paraemeter����ڸòֿ���Ĳ���
 */
bool sw_parameter_unregister_depot( char* name )
{
	int i,j,t, maxnum;
	sw_idepot_t* depot = NULL;
	
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	t =  depot_find_byname(name);
	if( t >= 0 && t< m_parameter.dcount )
	{
		depot = m_parameter.depots[t];
		if( t + 1< m_parameter.dcount) 
			memmove(m_parameter.depots+t,m_parameter.depots+t+1,sizeof(sw_idepot_t)*(m_parameter.dcount-1-t) );
		m_parameter.depots[m_parameter.dcount-1] = NULL;
		m_parameter.dcount--;
	}
	
	if( depot == NULL )
	{	
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		return true;
	}
	
	//ɾ�������ڸòֿ���Ĳ���
	maxnum = m_parameter.pnum;
	j=0;
	for( i=0; i < maxnum; i++ )
	{
		if(  m_parameter.plist[i].depot !=  depot )
		{
			if ( j != i )
			{
				memmove( m_parameter.plist+j, m_parameter.plist+i, sizeof(para_desc_t));
				memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
			}
			j++;
		}
		else
		{
			if( m_parameter.plist[i].name )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].name, __FILE__, __LINE__ );
			if( m_parameter.plist[i].value )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
	
			memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
		}
	}
	m_parameter.pnum = j;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return true;
}


/*
 * @brief ��ָ����depot���ݷַ���swparameter��
 * @param sw_paradepot_t* p_depot, ��������Ҫ��depot
 * @return �ɹ�����true,ʧ�ܷ���false
 */
//�������µĴ�����
static int on_update_para(sw_idepot_t *depot,char* name,char* value)
{
	sw_parameter_set_with_depot(name,value,NULL,false,false);
	return 0;
}
bool sw_parameter_updatefrom_depot( sw_idepot_t* depot )
{
	if( depot )
	{
		depot->load(depot,on_update_para);
		return true;
	}
	else
		return false;
}


/** 
 * @brief ��������в���, ����Ŀ����load��ʽ����
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_save()
{
	int i=0;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
	
	//����Ƿ��в����ı�
	for( i =0;i<m_parameter.dcount;i++)
	{
		if( m_parameter.depots[i] != NULL  &&  m_parameter.depots[i]->modified )
			break;
	}
	
	//����в����ı䣬�ͷ��ź�����������������߳�
	if( m_parameter.signal && i< m_parameter.dcount)
	{
		m_parameter.saveall = true;
		sw_signal_give(m_parameter.signal);
	}
	
	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	return true;
}


/**
 * @brief ���ò���ֵ�ı�Ļص�����
 */
void sw_parameter_set_observer(on_para_modified on_modified,void* handle)
{
	if( m_parameter.ocount < MAX_OBSERVER_NUM )
	{
		m_parameter.observers[m_parameter.ocount].on_modified = on_modified;
		m_parameter.observers[m_parameter.ocount].handle = handle;
		m_parameter.ocount++;
	}
}


/** 
 * @brief ��ȡ�ı�����Ĳ���
 * 
 * @param name 
 * @param value 
 * @param size 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_get( char* name, char* value, int size )
{
	int i;
	if ( name == NULL || m_parameter.mem == NULL)
		return false;

	if( m_parameter.mutex )
		sw_mutex_lock( m_parameter.mutex );
	
	/* Ѱ�Ҳ����������е�λ�� */
	i = sw_parameter_find( name );
	if( i < 0 )
	{
		/* Ĭ�ϵ������JAVA���ݿ�Ļ�,û�еĻ�ֱ�Ӵ�java�ж�ȡ */
		if ( m_parameter.defdepot->type == IDEPOT_JAVA && m_parameter.pnum < MAX_PARA_NUM)
		{
			i = m_parameter.pnum;
			m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
			m_parameter.plist[i].value = NULL;
			m_parameter.plist[i].depot = m_parameter.defdepot;
			m_parameter.plist[i].readonly = false;
			m_parameter.plist[i].realtime = true;
			m_parameter.pnum++;
			insert_qsort();
			i = sw_parameter_find( name );
			goto PARAMETER_GET;
		}
		if ( value != NULL && 0 < size )
			memset(value,0,size);
		if( m_parameter.mutex )
			sw_mutex_unlock( m_parameter.mutex );
		return false;
	}
PARAMETER_GET:
	/* ���� JAVA ʵʱ��ֻ������,ÿ�ζ���java��ȡ */
	if( m_parameter.plist[i].depot->type == IDEPOT_JAVA  
		&& m_parameter.plist[i].realtime == true
		&& m_parameter.plist[i].readonly == false )
	{
		if (m_parameter.plist[i].value != NULL)
		{
			sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
			m_parameter.plist[i].value = NULL;
		}
		if( m_parameter.mutex )
			sw_mutex_unlock( m_parameter.mutex );
		if ( m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,name,value,size) )
			return true;
		return false;
	}
	/* ����������еĲ���ֵΪ�գ��Ӳֿ��л�ȡһ��*/
	if( m_parameter.plist[i].value == NULL )
	{	/* ��������ϲ�ֻ���жϲ����Ƿ���ڵĻ��˴�����ɺ�����ȡ�������� */
		if ( value != NULL && 0 < size)
			memset(value, 0, size);
		if ( size < 1024 || value == NULL )
		{
			char tmpbuf[1024];
			if ( m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,name,tmpbuf,sizeof(tmpbuf)) )
			{
				if (m_parameter.plist[i].readonly == false || tmpbuf[0] != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem,tmpbuf, __FILE__, __LINE__ );
				if ( value != NULL && 0 < size)
					strlcpy(value, tmpbuf, size);
			}	
		}
		else
		{
			if ( m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,name,value,size) )
			{
				if (m_parameter.plist[i].readonly == false || value[0] != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
	}
	else
	{
		/* �Ƿ���Ҫ��memset??���ڲ������ִ�,�ϲ���ʹ����ҲҪ���ִ��ķ�ʽʹ��,��������memset */
		if ( value != NULL && 0 < size )
			strlcpy( value, m_parameter.plist[i].value, size);
	}
   
	if( m_parameter.mutex )
		sw_mutex_unlock( m_parameter.mutex );
		
	return true;
}

/** 
 * @brief �����ı�����Ĳ���
 * 
 * @param name 
 * @param value 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_set( char* name, char* value )
{
	if ( m_parameter.mem == NULL )
		return false;
	return sw_parameter_set_with_depot(name,value,NULL,false,false);
}
	
/*���ò�����ָ����depot*/
static bool sw_parameter_set_with_depot(char* name,char* value,sw_idepot_t* depot,bool is_load,bool only_default)
{
	int i,len1,len2;
	
	if( m_parameter.mutex )
		sw_mutex_lock( m_parameter.mutex );
	
	i = sw_parameter_find(name);
	
	if( i >= 0 )
	{
		if( value == NULL )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//�����д������,����false,JAVAֻ�������������ڴ��޸�,��java������Ϊ��ʱ�������޸�
		if( m_parameter.plist[i].readonly && (m_parameter.plist[i].depot->type == IDEPOT_JAVA || m_parameter.plist[i].value != NULL))
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//���ֻ������ȱʡ��������depotû�иı䣬��ֱ�ӷ���
		if( only_default && (depot == m_parameter.plist[i].depot || depot == NULL) )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//���ָ���Ĳ����ֿ�ͱ���Ĳ����ֿ��������Ҽ���û�����(typeԽ�󼶱��µף�
		//��ֱ�ӷ���
		if( depot != NULL && depot !=  m_parameter.plist[i].depot 
			&& depot->type >= m_parameter.plist[i].depot->type ) 
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}
		/* JAVAʵʱ����ֱ�ӱ��浽java���ݿ���,�����ر��� */
		if (false == is_load && m_parameter.plist[i].depot->type == IDEPOT_JAVA && m_parameter.plist[i].realtime)
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			m_parameter.plist[i].depot->set(m_parameter.plist[i].depot,name,value);
			return true;
	  }
		//����ֵ��ͬ����û�иı�����ֿ⣬��ֱ�ӷ���true
		if(m_parameter.plist[i].value && strcmp(m_parameter.plist[i].value, value) == 0
			&& (depot == NULL || depot == m_parameter.plist[i].depot) )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);

			return true;
		}

		if( m_parameter.plist[i].value == NULL )
		{
			m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
		}
		else if( strcmp(m_parameter.plist[i].value, value) != 0 )
		{
			len1 = strlen( m_parameter.plist[i].value );
			len2 = strlen( value );
			if( len2 <= len1 )
				strcpy( m_parameter.plist[i].value, value );//���жϿռ��С
			else
			{
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
		//��������ֿ�
		if( depot != NULL )
			m_parameter.plist[i].depot = depot;
		
		if( !is_load )
		{
			m_parameter.plist[i].depot->set(m_parameter.plist[i].depot,name,value);
			m_parameter.plist[i].depot->modified = true;
			if( m_parameter.plist[i].depot->autosave)
			{
				m_parameter.saveauto = true;
				sw_signal_give(m_parameter.signal);
			}
		}
		else /**/
		{
			if (m_parameter.plist[i].depot->type == IDEPOT_JAVA)
				m_parameter.plist[i].realtime = true;
		}

		if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
		
		if( !is_load )
		{
			for( i =0;i<m_parameter.ocount; i++ )
			{
				if(m_parameter.observers[i].handle != NULL )
					m_parameter.observers[i].on_modified(name,value,m_parameter.observers[i].handle);
			}
		}

		return true;
	}
	else if( m_parameter.pnum < MAX_PARA_NUM && strlen(name) < 256)
	{//���������̫����Ϊ�Ǳ�������
		i = m_parameter.pnum;
		m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
		if( value )
			m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
		else
			m_parameter.plist[i].value = NULL;
		if( depot == NULL )
			m_parameter.plist[i].depot = m_parameter.defdepot;
		else
			m_parameter.plist[i].depot = depot;

		if( !is_load )
		{
			m_parameter.plist[i].depot->set(m_parameter.plist[i].depot,name,value);
			m_parameter.plist[i].depot->modified = true;
			if( m_parameter.plist[i].depot->autosave)
			{
				m_parameter.saveauto  =  true;
				sw_signal_give(m_parameter.signal);
			}
		}
		
		if (m_parameter.plist[i].depot->type == IDEPOT_JAVA)
			m_parameter.plist[i].realtime = true;
		m_parameter.plist[i].readonly = false;
		m_parameter.pnum ++;
		/* ��������, ����ʱ������,�𽥼��� */
		insert_qsort();
		if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);

		return true;
	}

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	return false;

}

/* ���һ��Ĭ�ϵĲ���,���only_defaultΪtrue�Ļ���Ҫ�����ϵĲ���ֵ,���Ϊfalse�Ļ����ھͲ��ø����� */
static bool group_parameter_update(char *buf, int size, bool only_default)
{
	char *p, *e, *name_s, *name_e;
	char *name, *value;
	int strsize, len, num = 0;

	strsize = strlen( buf );
	if( size < strsize )
		strsize = size;

	/* �������� */
	p = buf;
	while( p < buf + strsize )
	{
		/* ȥ����ʼ�Ŀո� */
		while( *p == ' ' ||  *p == '\t' ||  *p == '\r' ||  *p == '\n' )
			p++;
		name_s = p;

		/* ���н��� */
		while( *p != '\r' && *p != '\n' && *p != '\0' )
			p++;
		e = p;

		/* ����: ע��,����,�ϰ汾������Ϣ */
		if( *name_s == '#' || e <= name_s )
			goto NEXT_ROW;

		/* �ҵ�name/value�ָ������õ��������� */
		p = name_s;
		while( *p != ' ' && *p != '\t' && *p != ':' && *p != '=' && p < e )
			p++;
		if( p <= name_s )
			goto NEXT_ROW;
		name_e = p;

		/* ����value֮ǰ�ķָ������õ�����ֵ */
		p++;
		while( *p == ' ' || *p == '\t' || *p == ':' || *p == '=' )
			p++;

		if( p <= e )
		{
			len = name_e - name_s;
			name = sw_mem_alloc( m_parameter.mem, len+1, __FILE__, __LINE__ );
			if( name )
			{
				memmove( name, name_s, len );
				name[len] = 0;
			}
			else
				break;
			len = e - p;
			value = sw_mem_alloc( m_parameter.mem, len+1, __FILE__, __LINE__ );
			if( value )
			{
				memmove( value, p, len );
				value[len] = 0;
			}
			else
				break;
			if ( !only_default )
			{/* ��Ҫˢ���ϵĲ���ֵ */
				sw_parameter_set( name, value );
			}
			else
			{/* �����������Ҫ��ӵ�Ĭ�ϵĲ����ֿ��� */
				sw_parameter_set_with_depot(name, value, m_parameter.defdepot, true, true);
			}
			sw_mem_free( m_parameter.mem, name, __FILE__, __LINE__ );
			sw_mem_free( m_parameter.mem, value, __FILE__, __LINE__ );
			num++;
		}
NEXT_ROW:
		p = e + 1;
	}

	return 0 < num;
}
/** 
 * @brief ����һ�����
 * 
 * @param buf 
 * @param size 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_set_group( char *buf, int size )
{
	return group_parameter_update(buf, size, false);
}

/**
 * @brief ���ڴ������һ��Ĭ�ϵĲ�����Ĭ�ϵĲ����ֿ��У����������������Ա����,��������������ӵ�Ĭ�ϵĲֿ���
 *
 * @param buf,��������
 * @param size,�����С
 *
 * return true,���³ɹ�; false,����ʧ��
 */
bool sw_parameter_set_group_default(char *buf, int size)
{
	return group_parameter_update(buf, size, true);
}

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
bool sw_parameter_update_with_depot(char *name, char *defaultvalue, bool readonly, bool realtime, sw_idepot_t* depot)
{
	if (name == NULL || *name == '\0' || defaultvalue == NULL || depot == NULL)
		return false;
	sw_mutex_lock( m_parameter.mutex );

	int i = sw_parameter_find(name);
	if (i >= 0)
	{
		m_parameter.plist[i].readonly = readonly;
		m_parameter.plist[i].realtime = realtime;
		if (depot == m_parameter.plist[i].depot)
		{
			if (depot->type == IDEPOT_FILE && 
					(m_parameter.plist[i].value == NULL || m_parameter.plist[i].value[0] == '\0') )
			{
				if (m_parameter.plist[i].value)
					sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = NULL;
				if (*defaultvalue != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
			}
			sw_mutex_unlock( m_parameter.mutex );
			return false;
		}
		if (depot->type == IDEPOT_JAVA)
		{
			char value[1024];
			depot->get(depot,name,value,sizeof(value));
			if (value[0] == '\0')
			{
				if (readonly == false && m_parameter.plist[i].value && m_parameter.plist[i].value[0] != '\0')
					depot->set(depot, name, m_parameter.plist[i].value);
			}
			else
			{
				if (m_parameter.plist[i].value)
					sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
		else if (depot->type == IDEPOT_FILE)
		{
			if (m_parameter.plist[i].value == NULL || m_parameter.plist[i].value[0] == '\0')
			{
				if (m_parameter.plist[i].value)
					sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = NULL;
				if (*defaultvalue != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
			}
		}
		m_parameter.plist[i].depot = depot;
		sw_mutex_unlock( m_parameter.mutex );
		return true;
	}
	else if( m_parameter.pnum < MAX_PARA_NUM && strlen(name) < 256)
	{
		i = m_parameter.pnum;
		m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
		m_parameter.plist[i].readonly = readonly;
		m_parameter.plist[i].realtime = realtime;
		m_parameter.plist[i].value = NULL;
		m_parameter.plist[i].depot = depot;
		if (depot->type == IDEPOT_JAVA)
		{
			char value[1024];
			depot->get(depot,name,value,sizeof(value));
			if (value[0] == '\0')
			{
				if (readonly == false && defaultvalue != '\0')
				{
					depot->set(depot, name, defaultvalue);
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
				}
			}
			else
			{
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
		else if (*defaultvalue != '\0')
		{
			if (depot->type == IDEPOT_FILE)
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
		}
		m_parameter.pnum ++;
		/* ��������, ����ʱ������,�𽥼��� */
		insert_qsort();
		sw_mutex_unlock(m_parameter.mutex);
		return false;
	}
	sw_mutex_unlock(m_parameter.mutex);
	return false;
}

/**
 * @brief ˢ��java�������е�ֻ���ͷ�ʵʱ��ȡ�Ĳ���
 */
void sw_parameter_refresh(void)
{
	sw_mutex_lock( m_parameter.mutex );
	int i = 0;
	for (i = 0; i < m_parameter.pnum; i++)
	{
		if (m_parameter.plist[i].depot->type == IDEPOT_JAVA 
				&& (m_parameter.plist[i].readonly || m_parameter.plist[i].realtime == false))
		{
			char value[1024];
			m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,m_parameter.plist[i].name,value,sizeof(value));
			if (value[0] == '\0')
			{
				if (m_parameter.plist[i].value)
					sw_mem_free(m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__);
				m_parameter.plist[i].value = NULL;
			}
			else
			{
				if (m_parameter.plist[i].value == NULL)
					m_parameter.plist[i].value = sw_mem_strdup(m_parameter.mem, value, __FILE__, __LINE__);
				else if (strcmp(value, m_parameter.plist[i].value) != 0)
				{
					sw_mem_free(m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__);
					m_parameter.plist[i].value = sw_mem_strdup(m_parameter.mem, value, __FILE__, __LINE__);
				}
			}
		}
	}
	sw_mutex_unlock(m_parameter.mutex);
}

/** 
 * @brief ��ȡ�ı�����Ĳ���,���ز�������ֵ
 * 
 * @param name 
 * 
 * @return ��������ֵ
 */
int sw_parameter_get_int( char* name )
{
	char text[16];

	if( sw_parameter_get( name, text, sizeof( text ) ) )
		return atoi( text );
	return -1;

}

/** 
 * @brief ����ֵ�����ı�����Ĳ���
 * 
 * @param name 
 * @param value ��������ֵ
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_set_int( char* name, int value )
{
	char text[16];
	sprintf( text, "%d", value );
	return sw_parameter_set( name, text );
}

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
bool sw_parameter_set_default( char* name, char* value,sw_idepot_t* depot)
{
	return sw_parameter_set_with_depot(name,value,depot,true,true);
}


/** 
 * @brief ���Ӳ�����ָ���Ĳ����ֿ�
 * 
 * @param name ��������
 * @param value ����ֵ
 * @param depotname ָ���Ĳ����ֿ�
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_add( char* name, char* value,char* depotname )
{
	int i;
	sw_idepot_t* depot = NULL;
	
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	i = depot_find_byname(depotname);

	if( i>=0 && i< m_parameter.dcount )
		depot = m_parameter.depots[i];

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	if( depot )
		sw_parameter_set_with_depot(name,value,depot,false,false);
	
	return (depot != NULL );
}

/** 
 * @brief ɾ���ı�����Ĳ���
 * 
 * @param name 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_delete( char* name )
{
	int i;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* Ѱ�Ҳ����������е�λ�� */
	i = sw_parameter_find( name );
	if( 0 <= i )
	{
		//�����д������,����false
		if( m_parameter.plist[i].readonly )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//������� java ����������ɾ��
		if( m_parameter.plist[i].depot->type == IDEPOT_JAVA )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		if( m_parameter.plist[i].name )
			sw_mem_free( m_parameter.mem, m_parameter.plist[i].name, __FILE__, __LINE__ );
		if( m_parameter.plist[i].value )
			sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
	
		/*���ò����иı�*/
		m_parameter.plist[i].depot->modified = true;

		/* �����ǰ�� */
		if( i+1 < m_parameter.pnum )
			memmove( m_parameter.plist+i, m_parameter.plist+i+1, sizeof(para_desc_t)*(m_parameter.pnum-i-1) );
		m_parameter.pnum--;
		
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		
		return true;
	}
	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
		
	return false;

}


/** 
 * @brief ���ȱʡ�ֿ��е����в���
 */
void sw_parameter_delete_all()
{
	int i,j,maxnum;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
	maxnum = m_parameter.pnum;
	for( i=0, j=0; i< maxnum; i++ )
	{
		if( m_parameter.plist[i].readonly ||  m_parameter.plist[i].depot !=  m_parameter.defdepot )
		{
			if ( j != i )
			{
				memmove( &m_parameter.plist[j], &m_parameter.plist[i], sizeof(para_desc_t));
				memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
			}
			j++;
		}
		else
		{
			if( m_parameter.plist[i].name )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].name, __FILE__, __LINE__ );
			if( m_parameter.plist[i].value )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
	
			memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
		}
	}
	m_parameter.pnum = j;
	m_parameter.defdepot->modified  = true;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
}

/** 
 * @brief ȡ�õ�һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_first( unsigned long *pos )
{
	if( 0 < m_parameter.pnum )
	{
		if( pos )
			*pos = 0;
		return m_parameter.plist[0].name;
	}
	return NULL;

}

/** 
 * @brief ȡ����һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_next( unsigned long *pos )
{
	if( ((int)*pos)+1 < m_parameter.pnum )
	{
		*pos = (*pos) + 1;
		return m_parameter.plist[*pos].name;
	}
	else
		return NULL;

}

/** 
 * @brief ȡ����һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_prev( unsigned long *pos )
{
	if( 0 < *pos )
	{
		*pos = (*pos) - 1;
		return m_parameter.plist[*pos].name;
	}
	else
		return NULL;
}

/** 
 * @brief ȡ�����һ������,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_last( unsigned long *pos )
{

	if( 0 < m_parameter.pnum )
	{
		*pos = m_parameter.pnum - 1;
		return m_parameter.plist[*pos].name;
	}
	else
		return NULL;

}

/** 
 * @brief ȡ�ò�����POS����,����NULL��ʾû���ҵ�
 * 
 * @param name 
 * @param pos 
 * 
 * @return true,�ɹ�; false, ʧ��
 */
bool sw_parameter_get_pos( char *name, unsigned long *pos )
{
	int i = sw_parameter_find( name );
	if( i < 0 )
		return false;
	else if( pos )
		*pos = i;
	return true;
}

/** 
 * @brief ����POS��ȡ����,����NULL��ʾû���ҵ�
 * 
 * @param pos 
 * @param *value 
 * 
 * @return 
 */
char* sw_parameter_get_by_pos( unsigned long pos, char **value )
{
	char* name = NULL;
	if( (int)pos >= m_parameter.pnum || value == NULL )
		return NULL;

	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	/* ���� JAVA ������ ÿ�ζ���java��ȡ */
	if( m_parameter.plist[pos].depot->type == IDEPOT_JAVA  
		&& m_parameter.plist[pos].realtime
		&& m_parameter.plist[pos].readonly == false
		)
	{
		if (m_parameter.plist[pos].value != NULL)
		{
			sw_mem_free( m_parameter.mem, m_parameter.plist[pos].value, __FILE__, __LINE__ );
			m_parameter.plist[pos].value = NULL;
		}
	}

	if( m_parameter.plist[pos].value == NULL )
	{
		char temp[1024];
		memset(temp,0,sizeof(temp));
		m_parameter.plist[pos].depot->get(m_parameter.plist[pos].depot,m_parameter.plist[pos].name,temp,sizeof(temp)-1);
		m_parameter.plist[pos].value = sw_mem_strdup( m_parameter.mem, temp, __FILE__, __LINE__ );
	}

	if( m_parameter.plist[pos].value )
	{
		*value = m_parameter.plist[pos].value;
		name = m_parameter.plist[pos].name;
	}

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	return name;
}

/** 
 * @brief ȡ�ò�������
 * 
 * @return 
 */
int sw_parameter_get_num()
{
	return m_parameter.pnum;
}


/**
 * @brief ����ĳ������д����
 *
 * @param name Ҫ�����Ĳ�������
 *
 * return �ɹ�����true��ʧ�ܷ���false
 */
bool sw_parameter_set_readonly(char* name,bool readonly)
{
	if ( name == NULL )
		return false;
	int i;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* Ѱ�Ҳ����������е�λ�� */
	i = sw_parameter_find( name );
	if( i >= 0 )
		m_parameter.plist[i].readonly  = readonly;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return (i >=0 );
}	

/**
 * @brief ȥ��ĳ��������д����
 *
 * @param name Ҫд�����Ĳ�������
 *
 * return �ɹ�����true��ʧ�ܷ���false
 */
bool sw_parameter_get_readonly(char* name)
{
	if ( name == NULL )
		return false;
	int i;
	bool readonly =false;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* Ѱ�Ҳ����������е�λ�� */
	i = sw_parameter_find( name );
	if( i >= 0 )
		readonly = m_parameter.plist[i].readonly;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return readonly;
}

/**
 * @brief ���ò����Ƿ�ʵʱ��ȡ:Ŀǰֻ��java����,Ĭ�ϵ�java���ݿ��������ʵʱ��ȡ��
 *
 * @param name Ҫʵʱ��д�Ĳ�������
 *
 * return �ɹ�����true��ʧ�ܷ���false
 */
bool sw_parameter_set_realtime(char* name,bool realtime)
{
	if ( name == NULL )
		return false;
	int i;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* Ѱ�Ҳ����������е�λ�� */
	i = sw_parameter_find( name );
	if( i >= 0 )
	{
		m_parameter.plist[i].realtime  = realtime;
	}
	else if (m_parameter.pnum < MAX_PARA_NUM && m_parameter.defdepot->type == IDEPOT_JAVA)
	{
			i = m_parameter.pnum;
			m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
			m_parameter.plist[i].value = NULL;
			m_parameter.plist[i].depot = m_parameter.defdepot;
			m_parameter.plist[i].readonly = false;
			m_parameter.plist[i].realtime = realtime;
			m_parameter.pnum ++;
			insert_qsort();
	}
	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return (i >=0 );
}	

/* ���Ҳ������ڵ����,���ڼ���Ĳ����Ѿ��ź����ˣ�ʹ���۰���� */
static int sw_parameter_find( char *name )
{
	if(name == NULL )
		return -1;
#if 1
	int big = 0, end = m_parameter.pnum-1;
	int i, mid;
	if ( m_parameter.pnum == 0 )
		return -1;
	//����
	while ( big <= end )
	{
		mid = (big+end)>>1;
		i=strcasecmp(name, m_parameter.plist[mid].name);
		if ( 0 < i )
		{//mid��nameС,����ʱ,������ʼλ��
			big = mid+1;
		}
		else if ( i < 0 )
		{//���Ľ���λ��
			end = mid-1;
		}
		else
			return mid;
	}
#else
	int i = 0;
	int maxnum = m_parameter.pnum;
	for ( ; i < maxnum ; i++)
	{
		if ( strcasecmp(m_parameter.plist[i].name, name) == 0 )
			return i;
	}
#endif
	return -1;
}

/* ����һ����������õ�����ӿ�,��������ŵ������ */
static void insert_qsort(void)
{/**/
	if ( m_parameter.pnum <= 1 )
		return ;
	int big = 0, end = m_parameter.pnum-2;
	int i, mid;
	char *name = m_parameter.plist[end+1].name;
	para_desc_t tmp;
	memcpy(&tmp, m_parameter.plist+m_parameter.pnum-1, sizeof(para_desc_t));
	/* ���ٲ��ҵ�����λ�� */
	while ( big <= end )
	{
		mid = (big+end)/2;
		i = strcasecmp(name, m_parameter.plist[mid].name);
		if ( 0 < i )
		{//mid��nameС,����ʱ,������ʼλ��
			big = mid + 1;
		}
		else if ( i < 0 )
		{
			end = mid - 1;
		}
		else
		{
			return;
		}
	}

	if ( 0 < i )
	{//mid��nameС,����ʱ,��mid������ұ�name���λ��
		int maxpos = m_parameter.pnum - 1;
		while ( mid < maxpos )
		{
			if ( 0 < strcasecmp(name, m_parameter.plist[mid].name) )
			{
				mid++;
			}
			else
			{
				memmove(m_parameter.plist+mid+1, m_parameter.plist+mid, sizeof(para_desc_t)*(m_parameter.pnum-1-mid));
				memcpy(m_parameter.plist+mid, &tmp, sizeof(para_desc_t));
				return;	
			}
		}
	}
	else
	{//mid��name����ǰ���ҵ����һ����name���λ��
		while ( 0 <= mid )
		{
			if ( strcasecmp(name, m_parameter.plist[mid].name) < 0 )
				mid--;
			else
			{
				mid++;
				break;
			}
		}
		if ( mid < 0 )
			mid = 0;
		memmove(m_parameter.plist+mid+1, m_parameter.plist+mid, sizeof(para_desc_t)*(m_parameter.pnum-1-mid));
		memcpy(m_parameter.plist+mid, &tmp, sizeof(para_desc_t));
	}
}

/*ͨ���ֿ������Ҳ����ֿ�*/
static int depot_find_byname(char* name)
{
	int i;
	
	if(name == NULL )
		return -1;

	for( i=0; i<m_parameter.dcount; i++ )
	{
		if( strcasecmp(m_parameter.depots[i]->name,name) == 0 )
			return i;
	}
	return -1;
}


//�����ֿ��ռ������Ļص�����,����-1��ʾû�к����Ĳ���
static int on_gather_para(sw_idepot_t *depot,int pos,char** name,char** value)
{	
	int i;
	
	if( pos < 0 )
		pos = 0;
	
	for( i=pos; i<m_parameter.pnum; i++)
	{
		if( m_parameter.plist[i].depot == depot )
		{
			*name =  m_parameter.plist[i].name;
			*value  =  m_parameter.plist[i].value;
			break;
		}
	}
	
	if( i <  m_parameter.pnum )	
		return (i+1);
	else
		return -1;
}

//���������߳�
static bool param_save_proc(uint32_t wparam,uint32_t lparam)
{
	int i;
	sw_parameter_t* parameter = (sw_parameter_t*)wparam;

	sw_signal_wait( parameter->signal,-1);
	
	//����ڳ�ʱ֮ǰ�����²���Ҫ���棬������ȴ�
	while( !parameter->exit  && (sw_signal_wait(parameter->signal,parameter->timeout)==0) );

	//��ʱ�ˣ����������
	if(parameter->mutex)
		sw_mutex_lock(parameter->mutex);
	
	//����Ƿ��в����ı�
	if( parameter->saveall)
	{
		for( i =0;i<parameter->dcount;i++)
		{
			if( parameter->depots[i] != NULL  &&  parameter->depots[i]->modified )
			{
				parameter->depots[i]->modified = false;
			 	parameter->depots[i]->save(parameter->depots[i],on_gather_para);
			}	
		}
 		parameter->saveall = false;
		parameter->saveauto =false;
	}
	else if( parameter->saveauto)
	{
		for( i =0;i<parameter->dcount;i++)
		{
			if( parameter->depots[i] != NULL  &&  parameter->depots[i]->modified &&  parameter->depots[i]->autosave )
			{
				parameter->depots[i]->modified = false;
			 	parameter->depots[i]->save(parameter->depots[i],on_gather_para);
			}	
		}
 		parameter->saveauto = false;
	}	

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	//�ж��Ƿ��˳�
	if( parameter->exit )
	{
		parameter->thrd = NULL;
		return false;
	}
	return true;
}
