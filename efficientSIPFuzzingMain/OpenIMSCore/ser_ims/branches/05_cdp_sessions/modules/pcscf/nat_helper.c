/*
 * $Id: nat_helper.c 161 2007-03-01 14:06:01Z vingarzan $
 *
 *
 * Copyright (C) 2005 Porta Software Ltd.
 *
 * This file is part of ser, a free SIP server.
 *
 * ser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * For a license to use the ser software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * ser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
* History:
 * ---------
 * 2003-10-09	nat_uac_test introduced (jiri)
 *
 * 2003-11-06   nat_uac_test permitted from onreply_route (jiri)
 *
 * 2003-12-01   unforce_rtp_proxy introduced (sobomax)
 *
 * 2004-01-07	RTP proxy support updated to support new version of the
 *		RTP proxy (20040107).
 *
 *		force_rtp_proxy() now inserts a special flag
 *		into the SDP body to indicate that this session already
 *		proxied and ignores sessions with such flag.
 *
 *		Added run-time check for version of command protocol
 *		supported by the RTP proxy.
 *
 * 2004-01-16   Integrated slightly modified patch from Tristan Colgate,
 *		force_rtp_proxy function with IP as a parameter (janakj)
 *
 * 2004-01-28	nat_uac_test extended to allow testing SDP body (sobomax)
 *
 *		nat_uac_test extended to allow testing top Via (sobomax)
 *
 * 2004-02-21	force_rtp_proxy now accepts option argument, which
 *		consists of string of chars, each of them turns "on"
 *		some feature, currently supported ones are:
 *
 *		 `a' - flags that UA from which message is received
 *		       doesn't support symmetric RTP;
 *		 `l' - force "lookup", that is, only rewrite SDP when
 *		       corresponding session is already exists in the
 *		       RTP proxy. Only makes sense for SIP requests,
 *		       replies are always processed in "lookup" mode;
 *		 `i' - flags that message is received from UA in the
 *		       LAN. Only makes sense when RTP proxy is running
 *		       in the bridge mode.
 *
 *		force_rtp_proxy can now be invoked without any arguments,
 *		as previously, with one argument - in this case argument
 *		is treated as option string and with two arguments, in
 *		which case 1st argument is option string and the 2nd
 *		one is IP address which have to be inserted into
 *		SDP (IP address on which RTP proxy listens).
 *
 * 2004-03-12	Added support for IPv6 addresses in SDPs. Particularly,
 *		force_rtp_proxy now can work with IPv6-aware RTP proxy,
 *		replacing IPv4 address in SDP with IPv6 one and vice versa.
 *		This allows creating full-fledged IPv4<->IPv6 gateway.
 *		See 4to6.cfg file for example.
 *
 *		Two new options added into force_rtp_proxy:
 *
 *		 `f' - instructs nathelper to ignore marks inserted
 *		       by another nathelper in transit to indicate
 *		       that the session is already goes through another
 *		       proxy. Allows creating chain of proxies.
 *		 `r' - flags that IP address in SDP should be trusted.
 *		       Without this flag, nathelper ignores address in the
 *		       SDP and uses source address of the SIP message
 *		       as media address which is passed to the RTP proxy.
 *
 *		Protocol between nathelper and RTP proxy in bridge
 *		mode has been slightly changed. Now RTP proxy expects SER
 *		to provide 2 flags when creating or updating session
 *		to indicate direction of this session. Each of those
 *		flags can be either `e' or `i'. For example `ei' means
 *		that we received INVITE from UA on the "external" network
 *		network and will send it to the UA on "internal" one.
 *		Also possible `ie' (internal->external), `ii'
 *		(internal->internal) and `ee' (external->external). See
 *		example file alg.cfg for details.
 *
 * 2004-03-15	If the rtp proxy test failed (wrong version or not started)
 *		retry test from time to time, when some *rtpproxy* function
 *		is invoked. Minimum interval between retries can be
 *		configured via rtpproxy_disable_tout module parameter (default
 *		is 60 seconds). Setting it to -1 will disable periodic
 *		rechecks completely, setting it to 0 will force checks
 *		for each *rtpproxy* function call. (andrei)
 *
 * 2004-03-22	Fix assignment of rtpproxy_retr and rtpproxy_tout module
 *		parameters.
 *
 * 2004-03-22	Fix get_body position (should be called before get_callid)
 * 				(andrei)
 *
 * 2004-03-24	Fix newport for null ip address case (e.g onhold re-INVITE)
 * 				(andrei)
 *
 * 2004-09-30	added received port != via port test (andrei)
 *
 * 2004-10-10   force_socket option introduced (jiri)
 *
 * 2005-02-24	Added support for using more than one rtp proxy, in which
 *		case traffic will be distributed evenly among them. In addition,
 *		each such proxy can be assigned a weight, which will specify
 *		which share of the traffic should be placed to this particular
 *		proxy.
 *
 *		Introduce failover mechanism, so that if SER detects that one
 *		of many proxies is no longer available it temporarily decreases
 *		its weight to 0, so that no traffic will be assigned to it.
 *		Such "disabled" proxies are periodically checked to see if they
 *		are back to normal in which case respective weight is restored
 *		resulting in traffic being sent to that proxy again.
 *
 *		Those features can be enabled by specifying more than one "URI"
 *		in the rtpproxy_sock parameter, optionally followed by the weight,
 *		which if absent is assumed to be 1, for example:
 *
 *		rtpproxy_sock="unix:/foo/bar=4 udp:1.2.3.4:3456=3 udp:5.6.7.8:5432=1"
 *
 * 2005-02-25	Force for pinging the socket returned by USRLOC (bogdan)
 *
 * 2005-03-22	Support for multiple media streams added (netch)
 *
 * 2005-04-27	Support for doing natpinging using real SIP requests added.
 *		Requires tm module for doing its work. Old method (sending UDP
 *		with 4 zero bytes can be selected by specifying natping_method="null".
 *
 * 2005-12-23	Support for selecting particular RTP proxy node has been added.
 *		In force_rtp_proxy() it can be done via new N modifier, followed
 *		by the index (starting at 0) of the node in the rtpproxy_sock
 *		parameter. For example, in the example above force_rtp_proxy("N1") will
 *		will select node udp:1.2.3.4:3456. In unforce_rtp_proxy(), the same
 *		can be done by specifying index as an argument directly, i.e.
 *		unforce_rtp_proxy(1).
 *
 *		Since nathelper is not transaction or call stateful, care should be
 *		taken to ensure that force_rtp_proxy() in request path matches
 *		force_rtp_proxy() in reply path, that is the same node is selected.
 *
 * 2006-06-10	select nathepler.rewrite_contact
 *		pingcontact function (tma)
 */

/**
 * $Id: nat_helper.c 161 2007-03-01 14:06:01Z vingarzan $
 *  
 * Copyright (C) 2004-2006 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * It has to be noted that this Open Source IMS Core System is not 
 * intended to become or act as a product in a commercial context! Its 
 * sole purpose is to provide an IMS core reference implementation for 
 * IMS technology testing and IMS application prototyping for research 
 * purposes, typically performed in IMS test-beds.
 * 
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being 
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this 
 * fact and have to agree to check out carefully before installing, 
 * using and extending the Open Source IMS Core System, if related 
 * patents and licences may become applicable to the intended usage 
 * context.  
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
 
/**
 * \file
 * 
 * Proxy-CSCF - NAT helper for signalling
 * 
 * \note Taken from SER nathelper module
 *  
 * \author adapted for the pcscf module by Marius Corici mco -at- fokus dot fraunhofer dot de
 * 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../parser/msg_parser.h"
#include "../../msg_translator.h"
#include "../../parser/parser_f.h"
#include "../../parser/contact/contact.h"
#include "../../parser/contact/parse_contact.h"
#include "../../parser/parse_uri.h"
#include "../../parser/hf.h"
#include "../../ut.h"
#include "../../forward.h"
#include "mod.h"
#include "registrar_storage.h"
#include "nat_helper.h"

static const char udp_ping[2] = { 10, 13};	/**< message to ping with - CRLF */

extern int pcscf_nat_enable; 				/**< whether to enable NAT */
extern int pcscf_nat_ping; 					/**< whether to ping anything */
extern int pcscf_nat_pingall; 				/**< whether to ping also the UA that don't look like being behind a NAT */
extern int pcscf_nat_detection_type; 		/**< the NAT detection tests */

#define READ(val) \
	(*(val + 0) + (*(val + 1) << 8) + (*(val + 2) << 16) + (*(val + 3) << 24))
#define advance(_ptr,_n,_str,_error) \
	do{\
		if ((_ptr)+(_n)>(_str).s+(_str).len)\
			goto _error;\
		(_ptr) = (_ptr) + (_n);\
	}while(0);
#define one_of_16( _x , _t ) \
	(_x==_t[0]||_x==_t[15]||_x==_t[8]||_x==_t[2]||_x==_t[3]||_x==_t[4]\
	||_x==_t[5]||_x==_t[6]||_x==_t[7]||_x==_t[1]||_x==_t[9]||_x==_t[10]\
	||_x==_t[11]||_x==_t[12]||_x==_t[13]||_x==_t[14])
#define one_of_8( _x , _t ) \
	(_x==_t[0]||_x==_t[7]||_x==_t[1]||_x==_t[2]||_x==_t[3]||_x==_t[4]\
	||_x==_t[5]||_x==_t[6])


network_t nets_1918[] = {
	{"10.0.0.0",    0, 0xffffffffu << 24},
	{"172.16.0.0",  0, 0xffffffffu << 20},
	{"192.168.0.0", 0, 0xffffffffu << 16},
	{NULL, 0, 0}
};


/**
 * Test if IP address pointed to by saddr belongs to RFC1918 networks.
 * @param saddr - string with address to check
 * @returns if the network is private one
 */
int is1918addr(str *saddr) {
	struct in_addr addr;
	uint32_t netaddr;
	int i, rval;
	char backup;

	rval = -1;
	backup = saddr->s[saddr->len];
	saddr->s[saddr->len] = '\0';
	if (inet_aton(saddr->s, &addr) != 1)
		goto is1918addr_end;
	netaddr = ntohl(addr.s_addr);
	for (i = 0; nets_1918[i].cnetaddr != NULL; i++) {
		if ((netaddr & nets_1918[i].mask) == nets_1918[i].netaddr) {
			rval = 1;
			goto is1918addr_end;
		}
	}
	rval = 0;

is1918addr_end:
	saddr->s[saddr->len] = backup;
	return rval;
}


/**
 * ser_memmem() returns the location of the first occurrence of data
 * pattern b2 of size len2 in memory block b1 of size len1 or
 * NULL if none is found. Obtained from NetBSD.
 */
char * ser_memmem(const void *b1, const void *b2, size_t len1, size_t len2)
{
	/* Initialize search pointer */
	char *sp = (char *) b1;

	/* Initialize pattern pointer */
	char *pp = (char *) b2;

	/* Initialize end of search address space pointer */
	char *eos = sp + len1 - len2;

	/* Sanity check */
	if(!(b1 && b2 && len1 && len2))
		return NULL;

	while (sp <= eos) {
		if (*sp == *pp)
			if (memcmp(sp, pp, len2) == 0)
				return sp;

			sp++;
	}

	return NULL;
}

/**
 * Fixes the 1918 private networks addresses.
 */
int nat_prepare_1918addr() {
	int i;
	struct in_addr addr;
	
	/* Prepare 1918 networks list */
	for (i = 0; nets_1918[i].cnetaddr != NULL; i++) {
		if (inet_aton(nets_1918[i].cnetaddr, &addr) != 1)
			abort();
		nets_1918[i].netaddr = ntohl(addr.s_addr) & nets_1918[i].mask;
	}
	return 1;
}

/**
 * Extract URI from the Contact header field
 * @param _m - the SIP message
 * @param uri - URI to fill
 * @param _c - contact to fill
 * @returns 0 on success, -1 on error
 */
static inline int get_contact_uri(struct sip_msg* _m, struct sip_uri *uri, contact_t** _c)
{

	if ((parse_headers(_m, HDR_CONTACT_F, 0) == -1) || !_m->contact)
		return -1;
	if (!_m->contact->parsed && parse_contact(_m->contact) < 0) {
		LOG(L_ERR, "get_contact_uri: Error while parsing Contact body\n");
		return -1;
	}
	*_c = ((contact_body_t*)_m->contact->parsed)->contacts;
	if (*_c == NULL) {
		LOG(L_DBG, "get_contact_uri: Error while parsing Contact body or star contact\n");
		return -1;
	}
	if (parse_uri((*_c)->uri.s, (*_c)->uri.len, uri) < 0 || uri->host.len <= 0) {
		LOG(L_ERR, "get_contact_uri: Error while parsing Contact URI\n");
		return -1;
	}
	return 0;
}

///**
// * Checks if the contact in the message is a 1918 address
// * @param msg - the SIP message
// * @returns 1 if it is, 0 if not, -1 on not found
// */
//static int contact_1918(struct sip_msg * msg) {
//	struct sip_uri uri;
//	contact_t * c;
//	
//	if(get_contact_uri(msg, &uri, &c) == -1)
//		return -1;
//	
//	return (is1918addr(&(uri.host)) == 1)?1:0; 
//}

/**
 * Checks if the first Via in the message is a 1918 address
 * @param msg - the SIP message
 * @returns 1 if it is, 0 if not
 */
static int via_1918(struct sip_msg * msg) {
	return (is1918addr(&(msg->via1->host)) == 1) ? 1 : 0;
}

/** 
 * Tests if the message was received from behind a NAT.
 * - tests received, contact and via
 * @param msg - the SIP message
 */
int nat_uac_test(struct sip_msg * msg) {

	if(pcscf_nat_pingall)
		return 1;
		
	if((pcscf_nat_detection_type && NAT_UAC_TEST_RPORT) && 
		(msg->rcv.src_port != (msg-> via1->port ? msg->via1->port:SIP_PORT)))
		return 1;
	
	if((pcscf_nat_detection_type & NAT_UAC_TEST_RCVD) && received_test(msg))
		return 1;
	
//	if((pcscf_nat_detection_type & NAT_UAC_TEST_C_1918) && contact_1918(msg))
		//return 1;
	
//	if((pcscf_nat_detection_type & NAT_UAC_TEST_S_1918) && sdp_1918(msg))
//		return 1;
		
	if((pcscf_nat_detection_type & NAT_UAC_TEST_V_1918) && via_1918(msg))
		return 1;
	
	return 0;
}

/**
 * Retrieves the originating source of a message into a r_nat_dest structure.
 * @param msg - the SIP message
 * @param pinhole - where to save to
 * @returns 1 on success, -1 on error
 */
int nat_msg_origin(struct sip_msg * msg, r_nat_dest ** pinhole) {
	if (!pcscf_nat_enable) {
		*pinhole = NULL;
		return -1;
	}
	if(pcscf_nat_pingall || nat_uac_test(msg)) {
		* pinhole = shm_malloc(sizeof(r_nat_dest));
		if(* pinhole == NULL) {
			LOG(L_ERR,"ERR:"M_NAME"nat_msg_origin:no memory\n");
			return -1;
		}
		memcpy(&(*pinhole)->nat_addr, &msg->rcv.src_ip, sizeof(struct ip_addr));
		(*pinhole) -> nat_port = msg -> rcv.src_port;
	} else {
		*pinhole = NULL;
	}
	return 1;
}

/**
 * Pings a contact to keep the NAT pinhole alive.
 * @param c - the r_contact to ping
 * @returns 1 on success, -1 on failure
 */
int nat_send_ping(r_contact *c) {
	struct dest_info dst;
	
	if(c->pinhole == NULL)
		return 1;
	if(c->transport != PROTO_UDP && c->transport != PROTO_NONE)
		return 1;
	init_dest_info(&dst);
	dst.proto = PROTO_UDP;
	
	memset(&(dst.to), 0, sizeof(union sockaddr_union));
	dst.to.s.sa_family=c->pinhole->nat_addr.af;
	switch(dst.to.s.sa_family) {
		case AF_INET6:
			memcpy(&dst.to.sin6.sin6_addr, c->pinhole->nat_addr.u.addr, c->pinhole->nat_addr.len);
			//dst.to.sin6.sin6_len=sizeof(struct sockaddr_in6);
			dst.to.sin6.sin6_port=htons(c->pinhole->nat_port);
			break;
		case AF_INET:
			memcpy(&dst.to.sin.sin_addr, c->pinhole->nat_addr.u.addr, c->pinhole->nat_addr.len);
			//dst.to.sin.sin_len=sizeof(struct sockaddr_in);
			dst.to.sin.sin_port=htons(c->pinhole->nat_port);
			break;
		default:
			LOG(L_CRIT,"CRIT:"M_NAME":nat_send_ping: unknown address family %d\n", dst.to.s.sa_family);
			return -1;
	}
	dst.send_sock=get_send_socket(0, &dst.to, PROTO_UDP);
	if(dst.send_sock == NULL) {
		LOG(L_ERR,"ERR:"M_NAME":nat_send_ping: cannot get sending socket\n");
		return -1;
	}
	udp_send(&dst, (char *)udp_ping, sizeof(udp_ping));
	return 1; 
}
