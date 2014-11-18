# $Id: Makefile 165 2007-03-02 15:15:46Z vingarzan $
#
# example module makefile
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=cpl-c.so

DEFS +=-I/usr/include/libxml2 -I$(LOCALBASE)/include/libxml2 \
	-I$(LOCALBASE)/include  # iconv.h

LIBS= -L$(LOCALBASE)/lib -lxml2

include ../../Makefile.modules
