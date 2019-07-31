/***************************************************************************
* AUTHOR:  tanzongren
* CONTENT: util�ӿ�
* NOTE:	
* HISTORY:
* [2006-01-05] created
****************************************************************************/
#include "swapi.h"
#include "swmem.h"
#include "util.h"
#include "swutil_priv.h"

/* ���ַ�����ʽ��ʱ��ת���ɶ�������ʽ����λ���� */
int sw_timezone2minute( char* p_timezone )
{
	short  hour=0, min=0, sign=1, n=0;
	char *p = p_timezone;
	if( *p=='+' )
	{
		sign=1;
		p++;
	}
	else if( *p=='-' )
	{
		sign=-1;
		p++;
	}
	while( '0'<=*p && *p<='9' )
	{
		hour = hour*10 + *p-'0';
		p++;
	}
	if( *p==':' )
	{
		for( p++; '0'<=*p && *p<='9'; p++ )
			min = min*10 + *p-'0';
	}
	n = hour * 60 + min;
	if( sign==-1 )
		n = -n;
	return n;
}

/* ��������ʱ��(��λ����)ת�����ַ��� */
void sw_minute2timezone( int minute, char* buf )
{
	short hour, min;
	char *p;

	p=buf;
	if( minute<0 )
	{
		*p = '-';
		p++;
		minute = -minute;
	}
	hour = minute/60;
	min = minute%60;
	if( min )
		sprintf( p, "%d:%d", hour, min );
	else
		sprintf( p, "%d", hour );
}

