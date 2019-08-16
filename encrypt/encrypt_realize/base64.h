//iamshuke@hotmail.com 2017.12.25
#ifndef _BASE64_H_
#define _BASE64_H_
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
//typedef unsigned short wchar_t;

//Summary:
//	��������Ҫfree���ص��ַ���
char* EncodeBase64A(const unsigned char* pData, unsigned int nCount);
wchar_t* EncodeBase64W(const unsigned char* pData, unsigned int nCount);

//Summary:
//	��������ֵΪ�����ڴ�*ppRet�Ĵ�С����������Ҫfree���ص�*ppRet
int DecodeBase64A(const char* szBase64Data, unsigned int nCount, unsigned char** ppRet);
int DecodeBase64W(const wchar_t* szBase64Data, unsigned int nCount, unsigned char** ppRet);

#ifdef UNICODE
#define EncodeBase64 EncodeBase64W
#define DecodeBase64 DecodeBase64W
#else
#define EncodeBase64 EncodeBase64A
#define DecodeBase64 DecodeBase64A
#endif

#endif
