##
## create by ChenKun(ckmx945@gmail.com)
## at 2016-04-18 10:27:59
##

TOOL_CHAIN_PATH =
CROSS_COMPILE = $(TOOL_CHAIN_PATH)
CC 		= $(CROSS_COMPILE)gcc
LD 		= $(CROSS_COMPILE)ld
AS 		= $(CROSS_COMPILE)as
AR 		= $(CROSS_COMPILE)ar
NM 		= $(CROSS_COMPILE)nm
STRIP 	= $(CROSS_COMPILE)strip
RANLIB 	= $(CROSS_COMPILE)ranlib


TOPDIR 		= $(shell pwd)
SRCDIR 		= $(TOPDIR)/src
OBJECTDIR 	= $(TOPDIR)/build
INCLUDESDIR = $(TOPDIR)/include
LIBSDIR 	= $(TOPDIR)/lib
LIBSRCSDIR 	= $(TOPDIR)/libsrc
RELEASEDIR 	= $(TOPDIR)/release

# version
VERSION_MAJOR = 0
VERSION_MINOR = 1

DEBUG = '' # Annotation is the release

# Common compiler and ld parameter
COMMON_LDFLAGS 	= -L$(LIBSDIR)
COMMON_CPPFLAGS =
COMMON_INCLUDES = -I$(INCLUDESDIR)
COMMON_CFLAGS 	= -Wall -MD -fPIC
