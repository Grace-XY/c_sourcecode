#############################################
# Author: Dou Hongchen, WangChao            #
# 2006-02-23 last update by Dou Hongchen    #
# 2007-03-06 last update by WangChao	    #
#############################################

#ģ�����ƺͰ汾��
NAME := swcommon

ifeq ($(NDK), y)
BUILD_TARGET_TYPE := static
endif


BINDIR := ../../pub/common/libs/$(SW_PLATFORM)


#Դ���ļ�
SRC +=							\
swfile/swhttpfile_priv.c		\
swfile/swrawfile.c				\
swfile/swhttpfile.c				\
swfile/swftpfile.c				\
swfile/swfile.c				    \
swfile/swftpfile_priv.c

ifeq ($(SUPPORT_HTTPS),y)
CFLAGS+=-DSUPPORT_HTTPS
SRC += \
swhttp/swhttpsclient.c \
swhttp/swhttpserver.c
endif



SRC += 							\
swftp/swftpclient.c 			\
swhttp/swhttpclient.c			\
swhttp/swhttpauth.c
SRC += swxml/document.c			\
	   swxml/element.c			\
	   swxml/map.c				\
	   swxml/membuf.c			\
	   swxml/node.c				\
	   swxml/nodelist.c			\
	   swxml/parser.c			\
	   swxml/swxml.c

#ͷ�ļ�����·��
INCDIR += \
-I.								\
-I./swfile/						\
-I../../pub/base/include 		\
-I../../pub/base/include/swos 	\
-I../../pub/upgrade/include 	\
-I../../pub/common/include 		\
-I../../pub/common/include/swxml	\
-I../../pub/common/include/swutil \
-I../common

ifeq ($(SUPPORT_HTTPS),y)
CFLAGS += -DSUPPORT_HTTPS
endif

ifeq ($(SUPPORT_OPENSSL),y)
OPENSSL_VER ?= openssl-0.9.8h
#$(warning ----- $(OPENSSL_VER))
CFLAGS += -DSUPPORT_OPENSSL
INCDIR += -I$(SWPARTSROOT)/pub/include/$(OPENSSL_VER)/
#$(warning ----- $(INCDIR))
DYN_LDS_WITH += -L$(SWPARTSROOT)/pub/libs/$(SW_PLATFORM)/$(OPENSSL_VER)  -lcrypto -lssl
#$(warning ----- $(DYN_LDS_WITH))
endif

ifeq ($(SUPPORT_WOLFSSL),y)
CFLAGS += -DSUPPORT_WOLFSSL
INCDIR += -I$(SWPARTSROOT)/pub/include/$(WOLFSSL_VER)/  
DYN_LDS_WITH += -L$(SWPARTSROOT)/pub/libs/$(SW_PLATFORM)/$(WOLFSSL_VER) -lwolfssl
$(warning ----- $(INCDIR))
$(warning ----- $(DYN_LDS_WITH))
endif

#���Ĺ���
-include ../../build/common.Makefile

copy:
	cp -f $(BINDIR)/lib$(NAME).so  $(RELEASEDIR)/rootfs/usr/local/lib
