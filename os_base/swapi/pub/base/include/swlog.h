#ifndef __SWLOG_H__
#define __SWLOG_H__

/* ��android�����±���Ĵ������Ҫʹ��ANDROID��log��Ҫ��Android.mk������ */
#if (defined(ANDROID) && defined(SUPPORT_SWAPI30))
#include "swlog_android.h"
#else
#include "swlog_linux.h"
#endif

#endif /* __SWLOG_H__ */