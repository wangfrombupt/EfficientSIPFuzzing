# $Id: Makefile 409 2007-07-24 15:24:24Z vingarzan $
#
# Makefile for the CDP module
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=isc.so

DEFS += -I/usr/include/libxml2
LIBS += -L$(LOCALBASE)/lib -lxml2

#DEFS += -DWITH_IMS_PM

include ../../Makefile.modules
