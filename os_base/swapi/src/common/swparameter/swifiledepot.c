/**
 * @file swifiledepot.c
 * @brief  ʵ���ļ������洢�ֿ�
 * @author chenkai
 * @history  liuchunrui created
 *			 chenkai  2011-02-25 review
 */
#include "swapi.h"
#include "swidepot.h"

typedef struct _ifile_depot
{
	sw_idepot_t depot;
	char file[256];
	char file_bak[256];
	char buf[4096];
}ifile_depot_t;

static bool ifile_load( sw_idepot_t *depot,spread_para_func spread );
static bool ifile_get( sw_idepot_t *depot, char* name ,char* value,int size );
static bool ifile_set( sw_idepot_t *depot, char* name ,char* value );
static bool ifile_save( sw_idepot_t *depot,gather_para_func gather );

/**
 * @brief ���ļ������ֿ�
 *
 * @param filename �ļ�·��
 *
 * @return ���ش�����idepot����
 */
sw_idepot_t* sw_ifiledepot_open(char* filename)
{
	ifile_depot_t* idepot = NULL;
	 
	idepot = (ifile_depot_t*)malloc(sizeof(ifile_depot_t));
	if(idepot == NULL )
		return NULL;
	memset((void*)idepot,0,sizeof(ifile_depot_t));

	strlcpy(idepot->file,filename,sizeof(idepot->file));
	snprintf(idepot->file_bak, sizeof(idepot->file_bak), "%s.bak", filename);
	strlcpy(idepot->depot.name,"ifile",sizeof(idepot->depot.name));
	idepot->depot.load = ifile_load;
	idepot->depot.get = ifile_get;
	idepot->depot.set = ifile_set;
	idepot->depot.save = ifile_save;
	idepot->depot.type = IDEPOT_FILE;
	return (sw_idepot_t*)idepot;
}


/**
 * @brief �رմ�����idepot����
 *
 * @param idepot����
 */
void sw_ifiledepot_close(sw_idepot_t* depot)
{
	if( depot )
	{
		free(depot);
	}
}


static bool ifile_load_file( sw_idepot_t *depot,spread_para_func spread, const char *filename )
{
	FILE *fp = NULL;
	char *str=NULL, *p;
	char *name, *value;
	int  size = 0;
	int  totalcnt = 0;
	int  file_total_param = 0;
	ifile_depot_t* idepot  = (ifile_depot_t*) depot;
	int ifile_version = 1;
	size = sizeof(idepot->buf);
	str = idepot->buf;
	memset(str,0,size);
	fp = fopen( filename, "r" );
	if(fp)
	{
		/* ���ļ��ж�ȡvalue */
		while( ! feof(fp) )
		{
			memset( str, 0, size);
			/* ��ȡһ�� */
			fgets( str, size, fp );
			if( str[0] == 0 || str[1] == 0 )
				continue;
			/* ȥ����ʼ�Ŀո� */
			p = str;
			while( *p == ' ' ||  *p == '\t' ||  *p == '\r' ||  *p == '\n' )
				p++;
			/* ����ע�� */
			if( *p == '#' )
			{
				if (strncmp(p, "#total:", 7) == 0)
					file_total_param = atoi(p+7);
				else if (strncmp(p, "#version:", 9) == 0)
					ifile_version = atoi(p+9);
				continue;
			}
			/* �ҵ�name */
			name = p;
			/* �ҵ�name/value�ָ��� */
			while( *p != ' ' && *p != '\t' && *p != ':' && *p != '=' && *p != '\r' && *p != '\n' && *p != '\0' )
				p++;
			*p = 0;

			/* ����value֮ǰ�ķָ��� */
			p++;
			while( *p == ' ' ||  *p == ':' ||  *p == '=' ||  *p == '\t' )
				p++;

			/* value��ʼ */
			value = p;
			/* ��λvalue���� */
			while( *p != '\r' && *p != '\n' && *p != '\0' )
				p++;
			*p = 0;
			if( name[0] /* && value[0] */)
			{
				spread(depot,name,value);
				totalcnt++;
			}
		}
		fclose( fp );
	}

	if (ifile_version == 2 && 0 < file_total_param && totalcnt == file_total_param)
		return true;//�����ϵ�û��bak�ļ�����bak�ǲ������õ�,�Ͳ����ж��ϰ汾�Ƿ���سɹ�
	else
		return false;
}
static bool ifile_load( sw_idepot_t *depot,spread_para_func spread)
{
	ifile_depot_t* idepot  = (ifile_depot_t*)depot;
	if (ifile_load_file(depot, spread, idepot->file))
		return true;
	return ifile_load_file(depot, spread, idepot->file_bak);
}
static bool ifile_get( sw_idepot_t *depot, char* name ,char* value,int size )
{
	return false;
}

static bool ifile_set( sw_idepot_t *depot, char* name ,char* value )
{
	return true;
}

static bool ifile_save_file(sw_idepot_t *depot,gather_para_func gather, const char *filename)
{
	char* name  = NULL;
	char* value = NULL;	
	int i=0;
	FILE *fp;
	ifile_depot_t* idepot  = (ifile_depot_t*)depot;
	char buf[8192];
	int len = 0;
	int paracnt = 0;
	fp =  fopen( filename, "w" );
	len = snprintf(buf, sizeof(buf), "#version:2\n");
	if( fp )
	{
		while( (i = gather( depot,i,&name,&value) ) >= 0 )
		{
			len += strlcpy(&buf[len], name, sizeof(buf)-len);
			buf[len] = buf[len+2] = ' ';
			buf[len+1] = '=';
			len += 3;
      if(value != NULL)
          len += strlcpy(&buf[len], value, sizeof(buf)-len);
			if ( len >= (int)sizeof(buf) )
				len = (int)sizeof(buf)-1;
			buf[len++] = '\n';
			if ( len >= 2048 )
			{
				int j = len&0x7ff;
				int wlen = len - j;
				fwrite(buf, wlen, 1, fp);
				if ( j )
					memcpy(buf, &buf[wlen], j);
				len = j;
			}
			paracnt++;
		}
		len += snprintf(&buf[len], sizeof(buf)-len, "#total:%d\n", paracnt);
		fwrite(buf, len, 1, fp);
		fflush( fp );
		fclose( fp );
		fp = NULL;
	}
	return true;
}

static bool ifile_save( sw_idepot_t *depot,gather_para_func gather )
{
	ifile_depot_t* idepot  = (ifile_depot_t*)depot;
	ifile_save_file(depot, gather, idepot->file);
	ifile_save_file(depot, gather, idepot->file_bak);
	return true;
}