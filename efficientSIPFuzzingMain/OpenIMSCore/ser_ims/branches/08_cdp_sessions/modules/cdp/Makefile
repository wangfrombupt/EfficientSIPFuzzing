# $Id: Makefile 2 2006-11-14 22:37:20Z vingarzan $
#
# Makefile for the CDP module
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=cdp.so



DEFS += -I/usr/include/libxml2 -DCDP_FOR_SER
LIBS += -L$(LOCALBASE)/lib -lxml2

include ../../Makefile.modules

