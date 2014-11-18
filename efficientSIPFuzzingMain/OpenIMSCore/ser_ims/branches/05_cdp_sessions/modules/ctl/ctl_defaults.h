/* $Id: ctl_defaults.h 2 2006-11-14 22:37:20Z vingarzan $
 */

#ifndef __ctl_defaults_h
#define __ctl_defaults_h
/*listen by default on: */
#define DEFAULT_CTL_SOCKET  "unixs:/tmp/ser_ctl"
/* port used by default for tcp/udp if no port is explicitely specified */
#define DEFAULT_CTL_PORT 2049

#define PROC_CTL -32

#endif
