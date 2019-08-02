/*************************************************************************
* AUTHOR��hujinshui / huanghuaming
* DATE��2005/09/14
* CONTEXT���ɷֿ�/�ɸ����ڴ�й©λ�õ��ڴ����ʽ
* REMARK��
* HISTORY: 
*************************************************************************/
#include "swapi.h"
#include "swmem.h"
#include "swthrd.h"
#include "swlog.h"
#include "swmutex.h"
#include "swutil_priv.h"

#define _DEBUG


//��������
#define ALIGN_SIZE(size)	((size+align) & ~align)		//�����align��xm->align-1
//ָ��((char*)p+size)����
#define ALIGN_PTR(p,size)	((size+align+(unsigned long)p) & ~align)

#define MEM_CHECK_SIZE 8
static const char m_memdbg_flag[] = "\xa5\xc1\xd8\x8e\xf1\xbd\x97\xec\xa6\xc2\xd9\x8f\xf2\xbe\x96\xed";

/*
static const char *GetPathFile( const char *path )
{
	int n = strlen(path)-1;
	while( 0 < n )
	{
		if( path[n] == '/' || path[n] == '\\' )
			return path+n+1;
		n--;
	}
	return path;
}
*/

/////////////////////////////////////////////////////////////////////////
typedef struct _MemHeader  
{
#ifdef _DEBUG
	//char filename[16];
	const char* filename;
	int line;
#endif
	int size;
	struct _MemHeader *prev, *next;
}smemnode_t;

typedef struct
{
	//��֧��Ļ�����
	char *buf;
	//��֧��Ļ�������С
	int size;
	//�����ֽ���
	int align;
	//��ǰ�Ѿ�������ڴ��С
	int alloc_size;
#ifdef _DEBUG
	//������λ��
	int maxpos;
#endif

	smemnode_t *first, *tail;	//����п��п�Ľ�㣬first����ǰ�Ŀ��п��㣬tail�����Ŀ��п���
	HANDLE mutex;
	
	//��������
	smemnode_t *list;
}sxmem_t;


//////////////////////////////////////////////////////////////////////////////////

/***
 * @brief ���ѷ�����ڴ���г�ʼ��
 * 
 * @param buf 	�ѷ�����ڴ��ַ
 * @param size 	�ѷ����ڴ�Ĵ�С 
 * @param align �ֽڷ���ķ�ʽ
 * @use_mutex   ʹ�û�����������˾�����ɵ��̷߳��ʣ���ֵ����0����������1
 * @return �ɹ�,�����ڴ���; ʧ��,���ؿ�ֵ
 */
HANDLE sw_mem_init_ex( char *buf, int size, int align, int use_mutex )
{
	if (buf == NULL || size < (int)sizeof(sxmem_t))
		return NULL;
	sxmem_t *xm = (sxmem_t *)(((align-1+(unsigned long)buf)/align)*align);

	memset( xm, 0, sizeof(sxmem_t) );
	xm->align = align;
	align--;
	xm->buf = (char *)ALIGN_PTR( xm, sizeof(sxmem_t) );		//�ɷ����ڴ�����
	xm->size = buf + size - xm->buf;						//�ɷ����ڴ�Ĵ�С
	if( use_mutex )
		xm->mutex = sw_mutex_create();
	
	return xm;
}


/***
 * @brief ���ѷ�����ڴ���г�ʼ�� 
 * 
 * @param buf 	�ѷ�����ڴ��ַ
 * @param size 	�ѷ����ڴ�Ĵ�С 
 * @param align �ֽڷ���ķ�ʽ
 * 
 * @return �ɹ�,�����ڴ���; ʧ��,���ؿ�ֵ
 */
HANDLE sw_mem_init( char *buf, int size, int align )
{
	return sw_mem_init_ex( buf, size, align, 1 );
}


/***
 * @brief �ͷ��ڴ���
 * 
 * @param h �ڴ���
 */
void sw_mem_exit( HANDLE h )
{
	if( h==NULL )
		return;
#ifdef _DEBUG
	sw_mem_check( h );
#endif
	sw_mutex_destroy( ((sxmem_t *)h)->mutex );
}

/***
 * @brief �ͷ��ڴ���, �����ڴ�й¶���, ���ִ��Ч��
 * 
 * @param h �ڴ���
 */
void sw_mem_exit_nocheck( HANDLE h )
{
	if( h==NULL )
		return;
#ifdef _DEBUG
	sw_mem_check_ex( h, 0 );
#endif
	sw_mutex_destroy( ((sxmem_t *)h)->mutex );
}

/***
 * @brief ��ȡ�ܵ��ڴ��С
 *
 * @param h �ڴ���
 * @return �ܵ��ڴ��С
 */
int sw_mem_get_total_size( HANDLE h )
{
	return h ? ((sxmem_t *)h)->size : 0;
}

/***
 * @brief �ڴ���գ�ǿ���ͷŵ������ѷ�����ڴ�
 * @param h �ڴ���
 */
void sw_mem_reset( HANDLE h )
{
	if (h == NULL)
		return;
	((sxmem_t *)h)->list = ((sxmem_t *)h)->first = ((sxmem_t *)h)->tail = NULL;
}


/***
 *   �������ڴ��е�����ȫ������Ϊ0
 * @param h �ڴ���
 */
void sw_mem_zero( HANDLE h )
{
	sxmem_t* xm = (sxmem_t*)h;
	if( xm==NULL )
		return;

	sw_mutex_lock( xm->mutex );
	if( xm->list==NULL )
	{
		memset( xm->buf, 0, xm->size );
	}
	else
	{
		smemnode_t *tail;
		unsigned int align = xm->align-1;
		char* p;
		tail = xm->tail;
		p = (char*)ALIGN_PTR( tail, sizeof(smemnode_t) );	//����Ǹ�����buf
		p = (char *)ALIGN_PTR( p, tail->size );		//����Ǹ�����buf��β��������Ϊ��һ����ַ
		memset( p, 0, xm->buf + xm->size - p );
	}
	sw_mutex_unlock( xm->mutex );
}

/***
 * @brief ȡ��ʷ������ѷ���ߴ�
 * @param h �ڴ���
 * 
 * @return ����ѷ���ߴ�Ĵ�С
 */
int sw_mem_get_max_size( HANDLE h )
{
#ifdef _DEBUG
	return ((sxmem_t *)h)->maxpos;
#else
	return sw_mem_get_cur_size( h );
#endif
}

/***
 * @brief ȡ����������ߴ�
 * @param h �ڴ���
 * @return ���ڷ���ߴ�Ĵ�С
 */
int sw_mem_get_cur_size( HANDLE h )
{
	if (h == NULL)
		return 0;
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node;
	char *p;
	unsigned int align = xm->align-1;
	int size = 0;
	if( xm->tail )
	{
		node = xm->tail;
		p = (char*)ALIGN_PTR(node, sizeof(smemnode_t));
		size = p-xm->buf + node->size;
	}
	
	return size;
}


static inline int crash_len( char* buf )
{
	int i;
	for( i=MEM_CHECK_SIZE; i>0 && buf[i-1] == m_memdbg_flag[i-1]; i-- );
	return i;
}


/** 
 * @brief ����Ƿ���δ�ͷŵ��ڴ棬�Լ��ڴ�����Ƿ��д��� 
 * 
 * @param h �ڴ���
 * @param print_node: �Ƿ��ӡ�����ڴ���
 */
void sw_mem_check_ex( HANDLE h, int print_node )
{
	if (h == NULL)
		return ;
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node = xm->list;
	int node_count, used_size, available_size, max_available;
	char *p, *end;
	int available;
	unsigned int align = xm->align-1;

	node_count = 0;
	used_size = 0;
	available_size = 0;
	max_available = 0;

	if( xm->list )
	{
		available = (unsigned long)xm->list - ALIGN_PTR(xm->buf,sizeof(smemnode_t));
		if( available > 0 )
		{
			if( max_available < available )
				max_available = available;
			available_size += available;
		}
		for( node = xm->list; node; node = node->next )
		{
			//����ڴ�ָ���Ƿ���Խ��
			if( (unsigned long)node < (unsigned long)xm
				|| (unsigned long)node >= (unsigned long)(xm->buf + xm->size)
				|| (unsigned long)ALIGN_PTR(node,sizeof(smemnode_t)) + (unsigned long)node->size > (unsigned long)(xm->buf + xm->size)
				|| node->size < 0
				|| (node != xm->list && (node->prev < xm->list || node->prev >= node || node->prev->next != node))
				|| (node->next && (node->next <= node || (char*)node->next > xm->buf + xm->size || node->next->prev != node)) )
			{
				if( (unsigned long)node < (unsigned long)xm
					|| (unsigned long)node >= (unsigned long)(xm->buf + xm->size) )
				{
					UTIL_LOG_DEBUG( "sw_mem_check error: node=%p, xm=%p, end=%p, xm->size=0x%x, node(size=%d, file=%s, line=%d)\n",
						node, xm, xm->buf+xm->size, xm->size, 0, "", 0 );
				}
				else
				{
					UTIL_LOG_DEBUG( "sw_mem_check error: node=%p, xm=%p, end=%p, xm->size=0x%x, node(size=%d, file=%s, line=%d)\n",
						node, xm, xm->buf+xm->size, xm->size, node->size-MEM_CHECK_SIZE, node->filename, node->line );
				}
			}

			used_size += node->size;
			if( node->next )
			{
				end = (char*)ALIGN_PTR(node,sizeof(smemnode_t)) + ALIGN_SIZE(node->size);
				available = ((char*)node->next) - (char*)ALIGN_PTR(end, sizeof(smemnode_t));
			}
			else
			{
				p = (char*)ALIGN_PTR( node, sizeof(smemnode_t) );
				end = (char *)ALIGN_PTR( p, node->size );
				p = (char*)ALIGN_PTR( end, sizeof(smemnode_t) );
				available = ALIGN_SIZE( xm->buf+xm->size - p );
			}
			if( available > 0 )
			{
				if( max_available < available )
					max_available = available;
				available_size += available;
			}
			node_count ++;
		}
	}
	else
	{
		p = (char*)ALIGN_PTR( xm->buf, sizeof(smemnode_t) );
		available_size = ALIGN_SIZE( xm->buf+xm->size - p );
		max_available = available_size;
	}

#if MEM_CHECK_SIZE > 0
	for( node = xm->list; node; node = node->next )
	{
		p = (char*)ALIGN_PTR(node,sizeof(smemnode_t));
		if( memcmp((char*)p + node->size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE)!=0 )
		{
			UTIL_LOG_ERROR( "%s(%p, size=%d-->%d, from %s:%d) memory is crashed.\n", __FUNCTION__, p, node->size-MEM_CHECK_SIZE, crash_len((char*)p + node->size-MEM_CHECK_SIZE), node->filename, node->line );
		}
	}
#endif
	if( print_node )
	{
#ifdef _DEBUG
		for( node = xm->list; node; node = node->next )
		{
			UTIL_LOG_DEBUG( "===>MEMORY LEAK: %s, line %d, %d bytes\n", node->filename, node->line, node->size-MEM_CHECK_SIZE );
		}
		UTIL_LOG_DEBUG( "MAX_POS=%d\n", xm->maxpos );
#endif

		UTIL_LOG_DEBUG( "%d nodes, %d bytes allocated.\tmemory %p, %d total bytes, %d bytes available, %d bytes max block\n", node_count, used_size, h, xm->size, available_size, max_available );
	}
}

/** 
 * @brief ����Ƿ���δ�ͷŵ��ڴ棬�Լ��ڴ�����Ƿ��д��� 
 * 
 * @param h �ڴ���
 */
void sw_mem_check( HANDLE h )
{
	sw_mem_check_ex( h, 1 );
}


#define INSERT_BEFORE(node, nex) {node->next=nex;if(nex){node->prev=nex->prev;nex->prev=node; if(node->prev)node->prev->next=node;}else{node->prev=NULL;}}
#define INSERT_AFTER(node, pre) {node->prev=pre;if(pre){node->next=pre->next;pre->next=node; if(node->next)node->next->prev=node;}else{node->next=NULL;}}
#define REMOVE_NODE(node) {if(node->prev)node->prev->next=node->next; if(node->next)node->next->prev=node->prev;}


/** 
 * @brief	ɨ�������Ƿ���free����ڴ���������Ҫ�� 
 * 
 * @param h	�ڴ���
 * @param size	�ѷ����ڴ�Ĵ�С
 * @param filename	���ڵĵ�ǰ�ļ���
 * @param line	���ڵĵ�ǰ�к�
 * 
 * @return �ɹ����ط�����ڴ����ʼ��ַ; ����,���ؿ�ֵ������������ȷ����
 */
static void* malloc_old( HANDLE h, int size ,const char *filename, int line)
{
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node, *new_node;
	unsigned int align = xm->align-1;
	char *end;
	int len;
	int firstlen;	//��һ�����п�Ĵ�С

	//���ͷ��buf֮��Ŀ�϶;
	if( xm->first==NULL )
	{
		if( xm->list == NULL )
			return NULL;
		
		node = xm->list;
		len = ((unsigned long)node) - ALIGN_PTR(xm->buf,sizeof(smemnode_t));
		if( size <= len )
		{
			//���NODE
			new_node = (smemnode_t *)xm->buf;
			memset( new_node, 0, sizeof(smemnode_t) );
#ifdef _DEBUG
			//strncpy( new_node->filename, GetPathFile(filename), sizeof(new_node->filename)-1 );
			new_node->filename = filename;
			new_node->line = line;
#endif
			new_node->size = size;
			
			//���뵽����ͷ;
			xm->list = new_node;
			//new_node->next = node;
			INSERT_BEFORE( new_node, node );
			xm->first = new_node;
			return (char*)ALIGN_PTR(new_node, sizeof(smemnode_t)); 
		}
		firstlen = len;
	}
	else
	{
		node = xm->first;
		firstlen = 0;
	}
	
	//�����������֮��ļ�϶�Ƿ������һ��ĳ���;
	while( node->next )
	{
		end = (char*)ALIGN_PTR(node,sizeof(smemnode_t)) + ALIGN_SIZE(node->size);
		len = ((char*)node->next) - (char*)ALIGN_PTR(end, sizeof(smemnode_t));
		
#if MEM_CHECK_SIZE > 0
		if( firstlen <= (int)align + ((MEM_CHECK_SIZE+3)&~3) )	//�����һ���п�̫С��������ͷ����ֽ��������޸����ָ�룬
#else
		if( firstlen <= (int)align )	//�����һ���п�̫С��������ͷ����ֽ��������޸����ָ�룬
#endif
		{
			xm->first = node;
			firstlen = len;
		}
		
		//�ҵ�����Ҫ��Ŀ�;
		if( size <= len )
		{
			//���NODE
			new_node = (smemnode_t *)end;
			memset( new_node, 0, sizeof(smemnode_t) );
#ifdef _DEBUG
			//strncpy( new_node->filename, GetPathFile(filename), sizeof(new_node->filename)-1 );
			new_node->filename = filename;
			new_node->line = line;
#endif
			new_node->size = size;
			
			//���뵽����;
			//new_node->next = node->next;
			//node->next = new_node;
			INSERT_AFTER( new_node, node );
			
			return (char*)ALIGN_PTR(new_node, sizeof(smemnode_t)); 
		}
		node=node->next;
		//���뵽��������;
	}
	return NULL;
}


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
void *sw_mem_alloc( HANDLE h, int size, const char *filename, int line )
{
	if (h == NULL)
		return NULL;
	sxmem_t *xm = (sxmem_t*)h;
	smemnode_t *tail = NULL;
	smemnode_t *node;
	unsigned int align = xm->align-1;
	char *p;

	if( size<0 )
	{
		UTIL_LOG_DEBUG( "%s error  %s:%d size=%d\n", __FUNCTION__, filename, line, size );
		return NULL;
	}
	size += MEM_CHECK_SIZE;
	sw_mutex_lock( xm->mutex );
	//��free�����
	p = (char *)malloc_old( h, size, filename, line );
	if( p )
	{
#ifdef _DEBUG
		if( xm->maxpos < (p-xm->buf) + size )
			xm->maxpos = (p-xm->buf) + size;
#endif

		xm->alloc_size += (size+(int)sizeof(smemnode_t));//�Ƿ���Ҫsize����?	
		sw_mutex_unlock( xm->mutex );
#if MEM_CHECK_SIZE > 0
		memcpy((char*)p+size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE);
#endif
		return p;
	}
	
	//��һ������
	if( xm->list == NULL )
		node = (smemnode_t *)xm->buf;
	else
	{
		tail = xm->tail;
		
		p = (char*)ALIGN_PTR( tail, sizeof(smemnode_t) );	//����Ǹ�����buf
		node = (smemnode_t *)ALIGN_PTR( p, tail->size );	//����Ǹ�����buf��β��������Ϊ��һ����ַ
	}
	
	p = (char*)ALIGN_PTR( node, sizeof(smemnode_t) );	//��ȡ����buf
	
	//����Ƿ����㹻���ڴ�
	if( xm->buf+xm->size < p+size )
	{
		if( xm->mutex != NULL )
		{
			UTIL_LOG_DEBUG("no enough memory h=%p, size=%d at %s %d\n", h, size-MEM_CHECK_SIZE, filename, line);
			sw_mem_check(h);
		}
		sw_mutex_unlock( xm->mutex );
		return NULL;
	}
	
	//���node
	memset( node, 0, sizeof(smemnode_t) );
	node->size = size;
#ifdef _DEBUG
	//strncpy( node->filename, GetPathFile(filename), sizeof(node->filename)-1 );
	node->filename = filename;
	node->line = line;
#endif
	
	//�ѵ�ǰ�ڵ�ҵ�����β��
	if( tail == NULL )
		xm->list = node;
	else
		tail->next = node;
#ifdef _DEBUG
	if( xm->maxpos < (p-xm->buf) + size )
		xm->maxpos = (p-xm->buf) + size;
#endif
	
	node->prev = tail;
	xm->tail = node;
	xm->alloc_size += (size+(int)sizeof(smemnode_t));//�Ƿ���Ҫsize����?
	
	sw_mutex_unlock( xm->mutex );
#if MEM_CHECK_SIZE > 0
	memcpy((char*)p+size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE);
#endif
	return p;
}

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
void *sw_mem_realloc( HANDLE h, void *p, int size, const char *filename, int line )
{
	if (h == NULL)
		return NULL;
	void *buf = NULL;
	sxmem_t *xm = (sxmem_t*)h;
	smemnode_t* node = NULL;
	unsigned int align = xm->align-1;
	int oldsize = 0;
	//��λ;
	if( p )
	{
		size += MEM_CHECK_SIZE;
		sw_mutex_lock( xm->mutex );
		if( xm->buf<(char*)p && (char*)p<xm->buf+xm->size )
		{
			node = (smemnode_t*)(((unsigned long)p - sizeof(smemnode_t))&~align);
			oldsize = node->size;
		}
		if( node &&
			(size<=node->size ||
			(char*)ALIGN_PTR(node,sizeof(smemnode_t))+size <= ( node->next ? (char*)node->next : xm->buf + xm->size) ) )
		{
#if MEM_CHECK_SIZE > 0
			if( memcmp((char*)p + node->size - MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE)!=0 )
			{
				UTIL_LOG_ERROR( "%s:%d  %s(%p, size=%d-->%d from %s:%d) xm=%p, memory is crashed.\n", filename, line, __FUNCTION__, p, node->size-MEM_CHECK_SIZE, crash_len((char*)p + node->size - MEM_CHECK_SIZE), node->filename, node->line, xm );
			}
#endif
			node->size = size;

			xm->alloc_size = (xm->alloc_size-oldsize+size);
			sw_mutex_unlock( xm->mutex );
#if MEM_CHECK_SIZE > 0
			memcpy((char*)p+size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE);
#endif
			return p;
		}
		sw_mutex_unlock( xm->mutex );
		size -= MEM_CHECK_SIZE;
	}
	
	//�ڴ治��,���·���;
	buf = sw_mem_alloc( h, size, filename, line );
	if( buf && p )
	{
		//��������;
		memcpy( buf, p, (node && node->size < size && node->size-MEM_CHECK_SIZE >= 0) ? node->size-MEM_CHECK_SIZE : size );
		//�ͷ�ԭ�����ڴ�;
		sw_mem_free( h, p, filename, line );
	}
	
	return buf;
}

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
char *sw_mem_strdup( HANDLE h, const char *s, const char *filename, int line )
{
	if (h == NULL || s == NULL)
		return NULL;
	int len = strlen(s);
	char *p = (char*)sw_mem_alloc( h, len+1, filename, line );
	if( p )
		memcpy(p, s, len+1);
	return p;
}



/** 
 * @brief 			�ͷ��ڴ�����ָ���һ���ڴ�
 * 
 * @param h 		�ڴ���
 * @param p 		ָ����ڴ��ַ
 * @param filename 	���ڵĵ�ǰ�ļ���
 * @param line 		���ڵĵ�ǰ�к�
 */
void sw_mem_free( HANDLE h, void *p, const char *filename, int line )
{
	if (h == NULL)
		return;
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node = NULL;
	unsigned int align = xm->align-1;
	int freesize = 0;

	sw_mutex_lock( xm->mutex );
	if( xm->buf<(char*)p && (char*)p<xm->buf+xm->size )
	{
		node = (smemnode_t*)(((unsigned long)p - sizeof(smemnode_t))&~align);
		freesize = node->size+(int)sizeof(smemnode_t);
	}
	//�ͷ�һ�鲻���ڵ��ڴ�
	if( node == NULL || xm->list == NULL )
	{
		UTIL_LOG_DEBUG( "sw_mem_free error: %s:%d, memory %p\n", filename, line, p );
	}
	else
	{
#ifdef _DEBUG
		char* end = xm->buf + xm->size;
		if( node->size < 0
			|| (char*)p + node->size > end
			|| (node > xm->list && (node->prev < xm->list || node->prev >= node || node->prev->next != node))
			|| (node->next && (node->next <= node || (char*)node->next > end || node->next->prev != node)) )
		{
			UTIL_LOG_DEBUG( "sw_mem_free error: %s:%d, memory %p is damaged.\n", filename, line, p );
			sw_mem_check_ex( xm, 1 );
			sw_mutex_unlock( xm->mutex );
			return;
		}
#endif
#if MEM_CHECK_SIZE > 0
		if( memcmp((char*)p + node->size - MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE)!=0 )
		{
			UTIL_LOG_ERROR( "%s:%d  %s(%p, size=%d-->%d, from %s:%d) xm=%p, memory is crashed.\n", filename, line, __FUNCTION__, p, node->size-MEM_CHECK_SIZE, crash_len((char*)p + node->size - MEM_CHECK_SIZE), node->filename, node->line, xm );
		}
#endif
		REMOVE_NODE(node);

		if( xm->list == node )
			xm->list = node->next;
		if( xm->first >= node )
			xm->first = node->prev;
		if( xm->tail == node )
			xm->tail = node->prev;
	}
	xm->alloc_size -= freesize;
	sw_mutex_unlock( xm->mutex );
}

/** 
 * @brief �����ڴ��Ƿ���ʹ����(�з������û�ͷŵ��ڴ�)
 * 
 * @param h �ڴ���
 * 
 * @return ����ڴ�û��ʹ��,�򷵻���(true);����,���ؼ�(false)
 */
bool sw_mem_is_empty( HANDLE h )
{
	if (h == NULL)
		return false;
	sxmem_t *xm = (sxmem_t *)h;
	return xm->list == NULL;
}

/***
 *   ��ȡ�ѷ����ڴ���ڴ���С
 * @param h: �ڴ���
 * @param p: �� sw_mem_alloc �� sw_mem_realloc ���ص��ڴ�ָ��
 */
int sw_mem_get_size( HANDLE h, void* p )
{
	if (h == NULL || p == NULL)
		return 0;
	sxmem_t* xm = (sxmem_t*)h;
	smemnode_t* node = NULL;
	unsigned int align = xm->align-1;
	if( xm->buf<(char*)p && (char*)p<xm->buf+xm->size )
	{
		node = (smemnode_t*)(((unsigned long)p - sizeof(smemnode_t))&~align);
		return node->size-MEM_CHECK_SIZE;
	}
	return 0;
}

/***
 *   ��ȡ�ѷ����ڴ��ܴ�С
 * @param h: �ڴ���
 */
int sw_mem_get_alloc_size( HANDLE h )
{
	if (h == NULL)
		return 0;
	sxmem_t* xm = (sxmem_t*)h;
	return xm->alloc_size;
}