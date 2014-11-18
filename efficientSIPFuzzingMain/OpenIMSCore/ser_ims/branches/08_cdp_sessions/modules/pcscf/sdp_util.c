#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include "../../timer.h"
#include "../../dprint.h"
#include "../../mem/mem.h"
#include "../../str.h"
#include "../../trim.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parser_f.h"
#include "../../parser/parse_from.h"
#include "../../parser/parse_uri.h"
#include "../../parser/contact/parse_contact.h"
#include "../../data_lump.h"
#include "sdp_util.h"
#include "mod.h"
#include "nat_helper.h"
#include "sip.h"

extern int pcscf_nat_enable;
extern struct rtpp_head rtpp_list;
extern int rtpp_node_count ;
extern char *rtpproxy_sock ; /* list */
extern int rtpproxy_enable; 
extern int rtpproxy_disable_tout ;
extern int rtpproxy_retr ;
extern int rtpproxy_tout ; 
unsigned int myseqn ;

static str sup_ptypes[] = {
	{.s = "udp", .len = 3},
	{.s = "udptl", .len = 5},
	{.s = "rtp/avp", .len = 7},
	{.s = NULL, .len = 0}
};

extern network_t nets_1918[];


int check_content_type(struct sip_msg *msg)
{
	static unsigned int appl[16] = {
		0x6c707061/*appl*/,0x6c707041/*Appl*/,0x6c705061/*aPpl*/,
		0x6c705041/*APpl*/,0x6c507061/*apPl*/,0x6c507041/*ApPl*/,
		0x6c505061/*aPPl*/,0x6c505041/*APPl*/,0x4c707061/*appL*/,
		0x4c707041/*AppL*/,0x4c705061/*aPpL*/,0x4c705041/*APpL*/,
		0x4c507061/*apPL*/,0x4c507041/*ApPL*/,0x4c505061/*aPPL*/,
		0x4c505041/*APPL*/};
	static unsigned int icat[16] = {
		0x74616369/*icat*/,0x74616349/*Icat*/,0x74614369/*iCat*/,
		0x74614349/*ICat*/,0x74416369/*icAt*/,0x74416349/*IcAt*/,
		0x74414369/*iCAt*/,0x74414349/*ICAt*/,0x54616369/*icaT*/,
		0x54616349/*IcaT*/,0x54614369/*iCaT*/,0x54614349/*ICaT*/,
		0x54416369/*icAT*/,0x54416349/*IcAT*/,0x54414369/*iCAT*/,
		0x54414349/*ICAT*/};
	static unsigned int ion_[8] = {
		0x006e6f69/*ion_*/,0x006e6f49/*Ion_*/,0x006e4f69/*iOn_*/,
		0x006e4f49/*IOn_*/,0x004e6f69/*ioN_*/,0x004e6f49/*IoN_*/,
		0x004e4f69/*iON_*/,0x004e4f49/*ION_*/};
	static unsigned int sdp_[8] = {
		0x00706473/*sdp_*/,0x00706453/*Sdp_*/,0x00704473/*sDp_*/,
		0x00704453/*SDp_*/,0x00506473/*sdP_*/,0x00506453/*SdP_*/,
		0x00504473/*sDP_*/,0x00504453/*SDP_*/};
	str           str_type;
	unsigned int  x;
	char          *p;

	if (!msg->content_type){
		if (parse_headers(msg, HDR_CONTENTTYPE_F, 0)<0){
			LOG(L_ERR,"ERR:"M_NAME":check_content_type: error parsing headers\n");
			return 1;	
		}
	}
	if (!msg->content_type)
	{
		LOG(L_WARN,"WARNING: check_content_type: Content-TYPE header absent!"
			"let's assume the content is text/plain ;-)\n");
		return 1;
	}

	trim_len(str_type.len,str_type.s,msg->content_type->body);
	p = str_type.s;
	advance(p,4,str_type,error_1);
	x = READ(p-4);
	if (!one_of_16(x,appl))
		goto other;
	advance(p,4,str_type,error_1);
	x = READ(p-4);
	if (!one_of_16(x,icat))
		goto other;
	advance(p,3,str_type,error_1);
	x = READ(p-3) & 0x00ffffff;
	if (!one_of_8(x,ion_))
		goto other;

	/* skip spaces and tabs if any */
	while (*p==' ' || *p=='\t')
		advance(p,1,str_type,error_1);
	if (*p!='/')
	{
		LOG(L_ERR, "ERROR:check_content_type: parse error:"
			"no / found after primary type\n");
		goto error;
	}
	advance(p,1,str_type,error_1);
	while ((*p==' ' || *p=='\t') && p+1<str_type.s+str_type.len)
		advance(p,1,str_type,error_1);

	advance(p,3,str_type,error_1);
	x = READ(p-3) & 0x00ffffff;
	if (!one_of_8(x,sdp_))
		goto other;

	if (*p==';'||*p==' '||*p=='\t'||*p=='\n'||*p=='\r'||*p==0) {
		DBG("DEBUG:check_content_type: type <%.*s> found valid\n",
			(int)(p-str_type.s), str_type.s);
		return 1;
	} else {
		LOG(L_ERR,"ERROR:check_content_type: bad end for type!\n");
		return -1;
	}

error_1:
	LOG(L_ERR,"ERROR:check_content_type: parse error: body ended :-(!\n");
error:
	return -1;
other:
	LOG(L_ERR,"ERROR:check_content_type: invalid type for a message\n");
	return -1;
}

int extract_body(struct sip_msg *msg, str *body )
{
	
	body->s = get_body(msg);
	if (body->s==0) {
		LOG(L_ERR, "ERROR: extract_body: failed to get the message body\n");
		goto error;
	}
	body->len = msg->len -(int)(body->s-msg->buf);
	if (body->len==0) {
		LOG(L_ERR, "ERROR: extract_body: message body has length zero\n");
		goto error;
	}
	
	/* no need for parse_headers(msg, EOH), get_body will 
	 * parse everything */
	/*is the content type correct?*/
	if (check_content_type(msg)==-1)
	{
		LOG(L_ERR,"ERROR: extract_body: content type mismatching\n");
		goto error;
	}
	
	/*DBG("DEBUG:extract_body:=|%.*s|\n",body->len,body->s);*/

	return 1;
error:
	return -1;
}



static int isnulladdr(str *sx, int pf)
{
	char *cp;

	if (pf == AF_INET6) {
		for(cp = sx->s; cp < sx->s + sx->len; cp++)
			if (*cp != '0' && *cp != ':')
				return 0;
		return 1;
	}
	return (sx->len == 7 && memcmp("0.0.0.0", sx->s, 7) == 0);
}

static int extract_mediaport(str *body, str *mediaport)
{
	char *cp, *cp1;
	int len, i;
	str ptype;

	cp1 = NULL;
	for (cp = body->s; (len = body->s + body->len - cp) > 0;) {
		cp1 = ser_memmem(cp, "m=", len, 2);
		if (cp1 == NULL || cp1[-1] == '\n' || cp1[-1] == '\r')
			break;
		cp = cp1 + 2;
	}
	if (cp1 == NULL) {
		LOG(L_ERR, "ERROR: extract_mediaport: no `m=' in SDP\n");
		return -1;
	}
	mediaport->s = cp1 + 2; /* skip `m=' */
	mediaport->len = eat_line(mediaport->s, body->s + body->len -
	  mediaport->s) - mediaport->s;
	trim_len(mediaport->len, mediaport->s, *mediaport);

	/* Skip media supertype and spaces after it */
	cp = eat_token_end(mediaport->s, mediaport->s + mediaport->len);
	mediaport->len -= cp - mediaport->s;
	if (mediaport->len <= 0 || cp == mediaport->s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no port in `m='\n");
		return -1;
	}
	mediaport->s = cp;
	cp = eat_space_end(mediaport->s, mediaport->s + mediaport->len);
	mediaport->len -= cp - mediaport->s;
	if (mediaport->len <= 0 || cp == mediaport->s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no port in `m='\n");
		return -1;
	}
	/* Extract port */
	mediaport->s = cp;
	cp = eat_token_end(mediaport->s, mediaport->s + mediaport->len);
	ptype.len = mediaport->len - (cp - mediaport->s);
	if (ptype.len <= 0 || cp == mediaport->s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no port in `m='\n");
		return -1;
	}
	ptype.s = cp;
	mediaport->len = cp - mediaport->s;
	/* Skip spaces after port */
	cp = eat_space_end(ptype.s, ptype.s + ptype.len);
	ptype.len -= cp - ptype.s;
	if (ptype.len <= 0 || cp == ptype.s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no protocol type in `m='\n");
		return -1;
	}
	/* Extract protocol type */
	ptype.s = cp;
	cp = eat_token_end(ptype.s, ptype.s + ptype.len);
	if (cp == ptype.s) {
		LOG(L_ERR, "ERROR: extract_mediaport: no protocol type in `m='\n");
		return -1;
	}
	ptype.len = cp - ptype.s;

	for (i = 0; sup_ptypes[i].s != NULL; i++)
		if (ptype.len == sup_ptypes[i].len &&
		    strncasecmp(ptype.s, sup_ptypes[i].s, ptype.len) == 0)
			return 0;
	/* Unproxyable protocol type. Generally it isn't error. */
	return -1;
}


static int extract_mediaip(str *body, str *mediaip, int *pf)
{
	char *cp, *cp1;
	int len, nextisip;

	cp1 = NULL;
	for (cp = body->s; (len = body->s + body->len - cp) > 0;) {
		cp1 = ser_memmem(cp, "c=", len, 2);
		if (cp1 == NULL || cp1[-1] == '\n' || cp1[-1] == '\r')
			break;
		cp = cp1 + 2;
	}
	if (cp1 == NULL) {
		LOG(L_ERR, "ERROR: extract_mediaip: no `c=' in SDP\n");
		return -1;
	}
	mediaip->s = cp1 + 2;
	mediaip->len = eat_line(mediaip->s, body->s + body->len - mediaip->s) - mediaip->s;
	trim_len(mediaip->len, mediaip->s, *mediaip);

	nextisip = 0;
	for (cp = mediaip->s; cp < mediaip->s + mediaip->len;) {
		len = eat_token_end(cp, mediaip->s + mediaip->len) - cp;
		if (nextisip == 1) {
			mediaip->s = cp;
			mediaip->len = len;
			nextisip++;
			break;
		}
		if (len == 3 && memcmp(cp, "IP", 2) == 0) {
			switch (cp[2]) {
			case '4':
				nextisip = 1;
				*pf = AF_INET;
				break;

			case '6':
				nextisip = 1;
				*pf = AF_INET6;
				break;

			default:
				break;
			}
		}
		cp = eat_space_end(cp + len, mediaip->s + mediaip->len);
	}
	if (nextisip != 2 || mediaip->len == 0) {
		LOG(L_ERR, "ERROR: extract_mediaip: "
		    "no `IP[4|6]' in `c=' field\n");
		return -1;
	}
	return 1;
}



//static int sdp_1918(struct sip_msg* msg)
//{
//	str body, ip;
//	int pf;
//
//	if (extract_body(msg, &body) == -1) {
//		LOG(L_ERR,"ERROR: sdp_1918: cannot extract body from msg!\n");
//		return 0;
//	}
//	if (extract_mediaip(&body, &ip, &pf) == -1) {
//		LOG(L_ERR, "ERROR: sdp_1918: can't extract media IP from the SDP\n");
//		return 0;
//	}
//	if (pf != AF_INET || isnulladdr(&ip, pf))
//		return 0;
//
//	return (is1918addr(&ip) == 1) ? 1 : 0;
//}

static int alter_mediaip(struct sip_msg *msg, str *body, str *oldip, int oldpf,
  			str *newip, int newpf, int preserve)
{
	char *buf;
	int offset;
	struct lump* anchor;
	str omip, nip, oip;

	/* check that updating mediaip is really necessary */
	if (oldpf == newpf && isnulladdr(oldip, oldpf))
		return 0;
	if (newip->len == oldip->len && memcmp(newip->s, oldip->s, newip->len) == 0)
		return 0;

	/*
	 * Since rewriting the same info twice will mess SDP up,
	 * apply simple anti foot shooting measure - put flag on
	 * messages that have been altered and check it when
	 * another request comes.
	 */
#if 0
	/* disabled:
	 *  - alter_mediaip is called twice if 2 c= lines are present
	 *    in the sdp (and we want to allow it)
	 *  - the message flags are propagated in the on_reply_route
	 *  => if we set the flags for the request they will be seen for the
	 *    reply too, but we don't want that
	 *  --andrei
	 */
	if (msg->msg_flags & FL_SDP_IP_AFS) {
		LOG(L_ERR, "ERROR: alter_mediaip: you can't rewrite the same "
		  "SDP twice, check your config!\n");
		return -1;
	}
#endif

	if (preserve != 0) {
		anchor = anchor_lump(msg, body->s + body->len - msg->buf  , 0, 0);
		if (anchor == NULL) 
		{
			LOG(L_ERR, "ERROR: alter_mediaip: anchor_lump failed\n");
			return -1;
		}
		if (oldpf == AF_INET6) 
		{
			omip.s = AOLDMEDIP6 ;
			omip.len = AOLDMEDIP6_LEN;
		} else {
			omip.s = AOLDMEDIP;
			omip.len = AOLDMEDIP_LEN;
		}
		buf = pkg_malloc(omip.len + oldip->len + CRLF_LEN);
		if (buf == NULL) 
		{
			LOG(L_ERR, "ERROR: alter_mediaip: out of memory\n");
			return -1;
		}
		memcpy(buf, omip.s, omip.len);
		memcpy(buf + omip.len, oldip->s, oldip->len);
		memcpy(buf + omip.len + oldip->len, CRLF, CRLF_LEN);
		if (insert_new_lump_after(anchor, buf,
		    omip.len + oldip->len + CRLF_LEN, 0) == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: insert_new_lump_after failed\n");
			pkg_free(buf);
			return -1;
		}
	}

	if (oldpf == newpf) {
		nip.len = newip->len;
		nip.s = pkg_malloc(nip.len);
		if (nip.s == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: out of memory\n");
			return -1;
		}
		memcpy(nip.s, newip->s, newip->len);
	} else {
		nip.len = newip->len + 2;
		nip.s = pkg_malloc(nip.len);
		if (nip.s == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaip: out of memory\n");
			return -1;
		}
		memcpy(nip.s + 2, newip->s, newip->len);
		nip.s[0] = (newpf == AF_INET6) ? '6' : '4';
		nip.s[1] = ' ';
	}

	oip = *oldip;
	if (oldpf != newpf) {
		do {
			oip.s--;
			oip.len++;
		} while (*oip.s != '6' && *oip.s != '4');
	}
	offset = oip.s - msg->buf;
	anchor = del_lump(msg, offset, oip.len, 0);
	if (anchor == NULL) {
		LOG(L_ERR, "ERROR: alter_mediaip: del_lump failed\n");
		pkg_free(nip.s);
		return -1;
	}

#if 0
	msg->msg_flags |= FL_SDP_IP_AFS;
#endif

	if (insert_new_lump_after(anchor, nip.s, nip.len, 0) == 0) {
		LOG(L_ERR, "ERROR: alter_mediaip: insert_new_lump_after failed\n");
		pkg_free(nip.s);
		return -1;
	}
	return 0;
}




static int alter_mediaport(struct sip_msg *msg, str *body, str *oldport, str *newport,
  int preserve)
{
	char *buf;
	int offset;
	struct lump* anchor;

	/* check that updating mediaport is really necessary */
	if (newport->len == oldport->len &&
	    memcmp(newport->s, oldport->s, newport->len) == 0)
		return 0;

	/*
	 * Since rewriting the same info twice will mess SDP up,
	 * apply simple anti foot shooting measure - put flag on
	 * messages that have been altered and check it when
	 * another request comes.
	 */
#if 0
	/* disabled: - it propagates to the reply and we don't want this
	 *  -- andrei */
	if (msg->msg_flags & FL_SDP_PORT_AFS) {
		LOG(L_ERR, "ERROR: alter_mediaip: you can't rewrite the same "
		  "SDP twice, check your config!\n");
		return -1;
	}
#endif

	if (preserve != 0) {
		anchor = anchor_lump(msg, body->s + body->len - msg->buf, 0, 0);
		if (anchor == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaport: anchor_lump failed\n");
			return -1;
		}
		buf = pkg_malloc(AOLDMEDPRT_LEN + oldport->len + CRLF_LEN);
		if (buf == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaport: out of memory\n");
			return -1;
		}
		memcpy(buf, AOLDMEDPRT, AOLDMEDPRT_LEN);
		memcpy(buf + AOLDMEDPRT_LEN, oldport->s, oldport->len);
		memcpy(buf + AOLDMEDPRT_LEN + oldport->len, CRLF, CRLF_LEN);
		if (insert_new_lump_after(anchor, buf,
		    AOLDMEDPRT_LEN + oldport->len + CRLF_LEN, 0) == NULL) {
			LOG(L_ERR, "ERROR: alter_mediaport: insert_new_lump_after failed\n");
			pkg_free(buf);
			return -1;
		}
	}

	buf = pkg_malloc(newport->len);
	if (buf == NULL) {
		LOG(L_ERR, "ERROR: alter_mediaport: out of memory\n");
		return -1;
	}
	offset = oldport->s - msg->buf;
	anchor = del_lump(msg, offset, oldport->len, 0);
	if (anchor == NULL) {
		LOG(L_ERR, "ERROR: alter_mediaport: del_lump failed\n");
		pkg_free(buf);
		return -1;
	}
	memcpy(buf, newport->s, newport->len);
	if (insert_new_lump_after(anchor, buf, newport->len, 0) == 0) {
		LOG(L_ERR, "ERROR: alter_mediaport: insert_new_lump_after failed\n");
		pkg_free(buf);
		return -1;
	}

#if 0
	msg->msg_flags |= FL_SDP_PORT_AFS;
#endif
	return 0;
}

char * find_sdp_line(char* p, char* plimit, char linechar)
{
	static char linehead[3] = "x=";
	char *cp, *cp1;
	linehead[0] = linechar;
	/* Iterate thru body */
	cp = p;
	for (;;) {
		if (cp >= plimit)
			return NULL;
		cp1 = ser_memmem(cp, linehead, plimit-cp, 2);
		if (cp1 == NULL)
			return NULL;
		/*
		 * As it is body, we assume it has previous line and we can
		 * lookup previous character.
		 */
		if (cp1[-1] == '\n' || cp1[-1] == '\r')
			return cp1;
		/*
		 * Having such data, but not at line beginning.
		 * Skip them and reiterate. ser_memmem() will find next
		 * occurence.
		 */
		if (plimit - cp1 < 2)
			return NULL;
		cp = cp1 + 2;
	}
	/*UNREACHED*/
	return NULL;
}

char * find_next_sdp_line(char* p, char* plimit, char linechar, char* defptr)
{
	char *t;
	if (p >= plimit || plimit - p < 3)
		return defptr;
	t = find_sdp_line(p + 2, plimit, linechar);
	return t ? t : defptr;
}

static inline int rfc1918address(str *address)
{
    struct in_addr inaddr;
    uint32_t netaddr;
    int i, result;
    char c;

    c = address->s[address->len];
    address->s[address->len] = 0;

    result = inet_aton(address->s, &inaddr);

    address->s[address->len] = c;

    if (result==0)
        return -1; /* invalid address to test */

    netaddr = ntohl(inaddr.s_addr);

    for (i=0; nets_1918[i].cnetaddr!=NULL; i++) {
        if ((netaddr & nets_1918[i].mask)==nets_1918[i].netaddr) {
            return 1;
        }
    }

    return 0;
}




static inline int get_to_tag(struct sip_msg* _m, str* _tag)
{
	if (!_m->to) {
		LOG(L_ERR, "get_to_tag(): To header field missing\n");
		return -1;
	}

	if (get_to(_m)->tag_value.len) {
		_tag->s = get_to(_m)->tag_value.s;
		_tag->len = get_to(_m)->tag_value.len;
	} else {
		_tag->s = 0; /* fixes gcc 4.0 warnings */
		_tag->len = 0;
	}

	return 0;
}

/*
 * Extract tag from From header field of a request
 */
static inline int get_from_tag(struct sip_msg* _m, str* _tag)
{

	if (parse_from_header(_m) == -1) {
		LOG(L_ERR, "get_from_tag(): Error while parsing From header\n");
		return -1;
	}

	if (get_from(_m)->tag_value.len) {
		_tag->s = get_from(_m)->tag_value.s;
		_tag->len = get_from(_m)->tag_value.len;
	} else {
		_tag->len = 0;
	}

	return 0;
}

static inline int get_callid(struct sip_msg* _m, str* _cid)
{

	if ((parse_headers(_m, HDR_CALLID_F, 0) == -1)) {
		LOG(L_ERR, "get_callid(): parse_headers() failed\n");
		return -1;
	}

	if (_m->callid == NULL) {
		LOG(L_ERR, "get_callid(): Call-ID not found\n");
		return -1;
	}

	_cid->s = _m->callid->body.s;
	_cid->len = _m->callid->body.len;
	trim(_cid);
	return 0;
}

static char * gencookie()
{
	static char cook[34];

	sprintf(cook, "%d_%u ", (int)getpid(), myseqn);
	myseqn++;
	return cook;
}

static char * send_rtpp_command(struct rtpp_node *node, struct iovec *v, int vcnt)
{
	struct sockaddr_un addr;
	int fd, len, i;
	char *cp;
	static char buf[256];
	struct pollfd fds[1];

	len = 0;
	cp = buf;
	if (node->rn_umode == 0) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_LOCAL;
		strncpy(addr.sun_path, node->rn_address,
		    sizeof(addr.sun_path) - 1);
#ifdef HAVE_SOCKADDR_SA_LEN
		addr.sun_len = strlen(addr.sun_path);
#endif

		fd = socket(AF_LOCAL, SOCK_STREAM, 0);
		if (fd < 0) {
			LOG(L_ERR, "ERROR: send_rtpp_command: can't create socket\n");
			goto badproxy;
		}
		if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			close(fd);
			LOG(L_ERR, "ERROR: send_rtpp_command: can't connect to RTP proxy\n");
			goto badproxy;
		}

		do {
			len = writev(fd, v + 1, vcnt - 1);
		} while (len == -1 && errno == EINTR);
		if (len <= 0) {
			close(fd);
			LOG(L_ERR, "ERROR: send_rtpp_command: can't send command to a RTP proxy\n");
			goto badproxy;
		}
		do {
			len = read(fd, buf, sizeof(buf) - 1);
		} while (len == -1 && errno == EINTR);
		close(fd);
		if (len <= 0) {
			LOG(L_ERR, "ERROR: send_rtpp_command: can't read reply from a RTP proxy\n");
			goto badproxy;
		}
	} else {
		fds[0].fd = node->rn_fd;
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		/* Drain input buffer */
		while ((poll(fds, 1, 0) == 1) &&
		    ((fds[0].revents & POLLIN) != 0)) {
			recv(node->rn_fd, buf, sizeof(buf) - 1, 0);
			fds[0].revents = 0;
		}
		v[0].iov_base = gencookie();
		v[0].iov_len = strlen(v[0].iov_base);
		for (i = 0; i < rtpproxy_retr; i++) {
			do {
				len = writev(node->rn_fd, v, vcnt);
			} while (len == -1 && (errno == EINTR || errno == ENOBUFS));
			if (len <= 0) {
				LOG(L_ERR, "ERROR: send_rtpp_command: "
				    "can't send command to a RTP proxy\n");
				goto badproxy;
			}
			while ((poll(fds, 1, rtpproxy_tout * 1000) == 1) &&
			    (fds[0].revents & POLLIN) != 0) {
				do {
					len = recv(node->rn_fd, buf, sizeof(buf) - 1, 0);
				} while (len == -1 && errno == EINTR);
				if (len <= 0) {
					LOG(L_ERR, "ERROR: send_rtpp_command: "
					    "can't read reply from a RTP proxy\n");
					goto badproxy;
				}
				if (len >= (v[0].iov_len - 1) &&
				    memcmp(buf, v[0].iov_base, (v[0].iov_len - 1)) == 0) {
					len -= (v[0].iov_len - 1);
					cp += (v[0].iov_len - 1);
					if (len != 0) {
						len--;
						cp++;
					}
					goto out;
				}
				fds[0].revents = 0;
			}
		}
		if (i == rtpproxy_retr) {
			LOG(L_ERR, "ERROR: send_rtpp_command: "
			    "timeout waiting reply from a RTP proxy\n");
			goto badproxy;
		}
	}

out:
	cp[len] = '\0';
	return cp;
badproxy:
	LOG(L_ERR, "send_rtpp_command(): proxy <%s> does not responding, disable it\n", node->rn_url);
	node->rn_disabled = 1;
	node->rn_recheck_ticks = get_ticks() + rtpproxy_disable_tout;
	return NULL;
}


int rtpp_test(struct rtpp_node *node, int isdisabled, int force)
{
	int rtpp_ver;
	char *cp;
	struct iovec v[2] = {{NULL, 0}, {"V", 1}};
	struct iovec vf[4] = {{NULL, 0}, {"VF", 2}, {" ", 1},
	    {REQ_CPROTOVER, 8}};

	if (force == 0) {
		if (isdisabled == 0)
			return 0;
		if (node->rn_recheck_ticks > get_ticks())
			return 1;
	}
	do {
		cp = send_rtpp_command(node, v, 2);
		if (cp == NULL) {
			LOG(L_WARN,"WARNING: rtpp_test: can't get version of "
			    "the RTP proxy\n");
			break;
		}
		rtpp_ver = atoi(cp);
		if (rtpp_ver != SUP_CPROTOVER) {
			LOG(L_WARN, "WARNING: rtpp_test: unsupported "
			    "version of RTP proxy <%s> found: %d supported, "
			    "%d present\n", node->rn_url,
			    SUP_CPROTOVER, rtpp_ver);
			break;
		}
		cp = send_rtpp_command(node, vf, 4);
		if (cp == NULL) {
			LOG(L_WARN,"WARNING: rtpp_test: RTP proxy went down during "
			    "version query\n");
			break;
		}
		if (cp[0] == 'E' || atoi(cp) != 1) {
			LOG(L_WARN, "WARNING: rtpp_test: of RTP proxy <%s>"
			    "doesn't support required protocol version %s\n",
			    node->rn_url, REQ_CPROTOVER);
			break;
		}
		LOG(L_INFO, "rtpp_test: RTP proxy <%s> found, support for "
		    "it %senabled\n",
		    node->rn_url, force == 0 ? "re-" : "");
		return 0;
	} while(0);
	LOG(L_WARN, "WARNING: rtpp_test: support for RTP proxy <%s>"
	    "has been disabled%s\n", node->rn_url,
	    rtpproxy_disable_tout < 0 ? "" : " temporarily");
	if (rtpproxy_disable_tout >= 0)
		node->rn_recheck_ticks = get_ticks() + rtpproxy_disable_tout;

	return 1;
}

static struct rtpp_node * select_rtpp_node(str callid, int do_test, int node_idx) 
{
	unsigned sum, sumcut, weight_sum;
	struct rtpp_node* node;
	int was_forced;

	/* Most popular case: 1 proxy, nothing to calculate */
	if (rtpp_node_count == 1) {
		if (node_idx > 0) {
			LOG(L_ERR, "ERROR: select_rtpp_node: node index out or range\n");
			return NULL;
		}
		node = rtpp_list.rn_first;
		if (node->rn_disabled && node->rn_recheck_ticks <= get_ticks()) {
			/* Try to enable if it's time to try. */
			node->rn_disabled = rtpp_test(node, 1, 0);
		}
		return node->rn_disabled ? NULL : node;
	}

	if (node_idx != -1) {
		for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
			if (node_idx > 0) {
				node_idx--;
				continue;
			}
			if (node->rn_disabled && node->rn_recheck_ticks <= get_ticks()) {
				/* Try to enable if it's time to try. */
				node->rn_disabled = rtpp_test(node, 1, 0);
			}
			return node->rn_disabled ? NULL : node;
		}
		LOG(L_ERR, "ERROR: select_rtpp_node: node index out or range\n");
		return NULL;
	}

	/* XXX Use quick-and-dirty hashing algo */
	for(sum = 0; callid.len > 0; callid.len--)
		sum += callid.s[callid.len - 1];
	sum &= 0xff;

	was_forced = 0;
retry:
	weight_sum = 0;
	for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
		if (node->rn_disabled && node->rn_recheck_ticks <= get_ticks()) {
			/* Try to enable if it's time to try. */
			node->rn_disabled = rtpp_test(node, 1, 0);
		}
		if (!node->rn_disabled)
			weight_sum += node->rn_weight;
	}
	if (weight_sum == 0) {
		/* No proxies? Force all to be redetected, if not yet */
		if (was_forced)
			return NULL;
		was_forced = 1;
		for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
			node->rn_disabled = rtpp_test(node, 1, 1);
		}
		goto retry;
	}
	sumcut = sum % weight_sum;
	/*
	 * sumcut here lays from 0 to weight_sum-1.
	 * Scan proxy list and decrease until appropriate proxy is found.
	 */
	for (node = rtpp_list.rn_first; node != NULL; node = node->rn_next) {
		if (node->rn_disabled)
			continue;
		if (sumcut < node->rn_weight)
			goto found;
		sumcut -= node->rn_weight;
	}
	/* No node list */
	return NULL;
found:
	if (do_test) {
		node->rn_disabled = rtpp_test(node, node->rn_disabled, 0);
		if (node->rn_disabled)
			goto retry;
	}
	return node;
}


static int
force_rtp_proxy2_f(struct sip_msg* msg, char* str1, char* str2)
{
	str body, body1, oldport, oldip, newport, newip;
	str callid, from_tag, to_tag, tmp;
	int create, port, len, asymmetric, flookup, argc, proxied, real;
	int oidx, pf=0, pf1, force, node_idx;
	char opts[16];
	char *cp, *cp1;
	char  *cpend, *next;
	char **ap, *argv[10];
	struct lump* anchor;
	struct rtpp_node *node;
	struct iovec v[14] = {
		{NULL, 0},	/* command */
		{NULL, 0},	/* options */
		{" ", 1},	/* separator */
		{NULL, 0},	/* callid */
		{" ", 1},	/* separator */
		{NULL, 7},	/* newip */
		{" ", 1},	/* separator */
		{NULL, 1},	/* oldport */
		{" ", 1},	/* separator */
		{NULL, 0},	/* from_tag */
		{";", 1},	/* separator */
		{NULL, 0},	/* medianum */
		{" ", 1},	/* separator */
		{NULL, 0}	/* to_tag */
	};
	char *v1p, *v2p, *c1p, *c2p, *m1p, *m2p, *bodylimit;
	char medianum_buf[20];
	int medianum, media_multi;
	str medianum_str, tmpstr1;
	int c1p_altered;

	v[1].iov_base=opts;
	asymmetric = flookup = force = real = 0;
	oidx = 1;
	node_idx = -1;
	for (cp = str1; *cp != '\0'; cp++) {
		switch (*cp) {
		case ' ':
		case '\t':
			break;

		case 'a':
		case 'A':
			opts[oidx++] = 'A';
			asymmetric = 1;
			real = 1;
			break;

		case 'i':
		case 'I':
			opts[oidx++] = 'I';
			break;

		case 'e':
		case 'E':
			opts[oidx++] = 'E';
			break;

		case 'l':
		case 'L':
			flookup = 1;
			break;

		case 'f':
		case 'F':
			force = 1;
			break;

		case 'r':
		case 'R':
			real = 1;
			break;

		case 'n':
		case 'N':
			cp++;
			for (len = 0; isdigit(cp[len]); len++)
				continue;
			if (len == 0) {
				LOG(L_ERR, "ERROR: force_rtp_proxy2: non-negative integer"
				    "should follow N option\n");
				return -1;
			}
			node_idx = strtoul(cp, NULL, 10);
			cp += len - 1;
			break;

		default:
			LOG(L_ERR, "ERROR: force_rtp_proxy2: unknown option `%c'\n", *cp);
			return -1;
		}
	}

	if (msg->first_line.type == SIP_REQUEST &&
	    msg->first_line.u.request.method_value == METHOD_INVITE) {
		create = 1;
	} else if (msg->first_line.type == SIP_REPLY) {
		create = 0;
	} else {
		return -1;
	}
	/* extract_body will also parse all the headers in the message as
	 * a side effect => don't move get_callid/get_to_tag in front of it
	 * -- andrei */
	if (extract_body(msg, &body) == -1) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't extract body "
		    "from the message\n");
		return -1;
	}
	if (get_callid(msg, &callid) == -1 || callid.len == 0) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't get Call-Id field\n");
		return -1;
	}
	if (get_to_tag(msg, &to_tag) == -1) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't get To tag\n");
		return -1;
	}
	if (get_from_tag(msg, &from_tag) == -1 || from_tag.len == 0) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: can't get From tag\n");
		return -1;
	}
	if (flookup != 0) {
		if (create == 0 || to_tag.len == 0)
			return -1;
		create = 0;
		tmp = from_tag;
		from_tag = to_tag;
		to_tag = tmp;
	}
	proxied = 0;
	for (cp = body.s; (len = body.s + body.len - cp) >= ANORTPPROXY_LEN;) {
		cp1 = ser_memmem(cp, ANORTPPROXY, len, ANORTPPROXY_LEN);
		if (cp1 == NULL)
			break;
		if (cp1[-1] == '\n' || cp1[-1] == '\r') {
			proxied = 1;
			break;
		}
		cp = cp1 + ANORTPPROXY_LEN;
	}
	if (proxied != 0 && force == 0)
		return -1;
	/*
	 * Parsing of SDP body.
	 * It can contain a few session descriptions (each starts with
	 * v-line), and each session may contain a few media descriptions
	 * (each starts with m-line).
	 * We have to change ports in m-lines, and also change IP addresses in
	 * c-lines which can be placed either in session header (fallback for
	 * all medias) or media description.
	 * Ports should be allocated for any media. IPs all should be changed
	 * to the same value (RTP proxy IP), so we can change all c-lines
	 * unconditionally.
	 */
	bodylimit = body.s + body.len;
	v1p = find_sdp_line(body.s, bodylimit, 'v');
	if (v1p == NULL) {
		LOG(L_ERR, "ERROR: force_rtp_proxy2: no sessions in SDP\n");
		return -1;
	}
	v2p = find_next_sdp_line(v1p, bodylimit, 'v', bodylimit);
	media_multi = (v2p != bodylimit);
	v2p = v1p;
	medianum = 0;
	for(;;) {
		/* Per-session iteration. */
		v1p = v2p;
		if (v1p == NULL || v1p >= bodylimit)
			break; /* No sessions left */
		v2p = find_next_sdp_line(v1p, bodylimit, 'v', bodylimit);
		/* v2p is text limit for session parsing. */
		m1p = find_sdp_line(v1p, v2p, 'm');
		/* Have this session media description? */
		if (m1p == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: no m= in session\n");
			return -1;
		}
		/*
		 * Find c1p only between session begin and first media.
		 * c1p will give common c= for all medias.
		 */
		c1p = find_sdp_line(v1p, m1p, 'c');
		c1p_altered = 0;
		/* Have session. Iterate media descriptions in session */
		m2p = m1p;
		for (;;) {
			m1p = m2p;
			if (m1p == NULL || m1p >= v2p)
				break;
			m2p = find_next_sdp_line(m1p, v2p, 'm', v2p);
			/* c2p will point to per-media "c=" */
			c2p = find_sdp_line(m1p, m2p, 'c');
			/* Extract address and port */
			tmpstr1.s = c2p ? c2p : c1p;
			if (tmpstr1.s == NULL) {
				/* No "c=" */
				LOG(L_ERR, "ERROR: force_rtp_proxy2: can't"
				    " find media IP in the message\n");
				return -1;
			}
			tmpstr1.len = v2p - tmpstr1.s; /* limit is session limit text */
			if (extract_mediaip(&tmpstr1, &oldip, &pf) == -1) {
				LOG(L_ERR, "ERROR: force_rtp_proxy2: can't"
				    " extract media IP from the message\n");
				return -1;
			}
			tmpstr1.s = m1p;
			tmpstr1.len = m2p - m1p;
			if (extract_mediaport(&tmpstr1, &oldport) == -1) {
				LOG(L_ERR, "ERROR: force_rtp_proxy2: can't"
				    " extract media port from the message\n");
				return -1;
			}
			++medianum;
			if (asymmetric != 0 || real != 0) {
				newip = oldip;
			} else {
				newip.s = ip_addr2a(&msg->rcv.src_ip);
				newip.len = strlen(newip.s);
			}
			/* XXX must compare address families in all addresses */
			if (pf == AF_INET6) {
				opts[oidx] = '6';
				oidx++;
			}
			snprintf(medianum_buf, sizeof medianum_buf, "%d", medianum);
			medianum_str.s = medianum_buf;
			medianum_str.len = strlen(medianum_buf);
			opts[0] = (create == 0) ? 'L' : 'U';
			v[1].iov_len = oidx;
			STR2IOVEC(callid, v[3]);
			STR2IOVEC(newip, v[5]);
			STR2IOVEC(oldport, v[7]);
			STR2IOVEC(from_tag, v[9]);
			if (1 || media_multi) /* XXX netch: can't choose now*/
			{
				STR2IOVEC(medianum_str, v[11]);
			} else {
				v[10].iov_len = v[11].iov_len = 0;
			}
			STR2IOVEC(to_tag, v[13]);
			do {
				node = select_rtpp_node(callid, 1, node_idx);
				if (!node) {
					LOG(L_ERR, "ERROR: force_rtp_proxy2: no available proxies\n");
					return -1;
				}
				cp = send_rtpp_command(node, v, (to_tag.len > 0) ? 14 : 12);
			} while (cp == NULL);
			/* Parse proxy reply to <argc,argv> */
			argc = 0;
			memset(argv, 0, sizeof(argv));
			cpend=cp+strlen(cp);
			next=eat_token_end(cp, cpend);
			for (ap = argv; cp<cpend; cp=next+1, next=eat_token_end(cp, cpend)){
				*next=0;
				if (*cp != '\0') {
					*ap=cp;
					argc++;
					if ((char*)++ap >= ((char*)argv+sizeof(argv)))
						break;
				}
			}
			if (argc < 1) {
				LOG(L_ERR, "force_rtp_proxy2: no reply from rtp proxy\n");
				return -1;
			}
			port = atoi(argv[0]);
			if (port <= 0 || port > 65535) {
				LOG(L_ERR, "force_rtp_proxy2: incorrect port in reply from rtp proxy\n");
				return -1;
			}

			pf1 = (argc >= 3 && argv[2][0] == '6') ? AF_INET6 : AF_INET;

			if (isnulladdr(&oldip, pf)) {
				if (pf1 == AF_INET6) {
					newip.s = "::";
					newip.len = 2;
				} else {
					newip.s = "0.0.0.0";
					newip.len = 7;
				}
			} else {
				newip.s = (argc < 2) ? str2 : argv[1];
				newip.len = strlen(newip.s);
			}
			newport.s = int2str(port, &newport.len); /* beware static buffer */
			/* Alter port. */
			body1.s = m1p;
			body1.len = bodylimit - body1.s;
			if (alter_mediaport(msg, &body1, &oldport, &newport, 0) == -1)
				return -1;
			/*
			 * Alter IP. Don't alter IP common for the session
			 * more than once.
			 */
			if (c2p != NULL || !c1p_altered) {
				body1.s = c2p ? c2p : c1p;
				body1.len = bodylimit - body1.s;
				if (alter_mediaip(msg, &body1, &oldip, pf, &newip, pf1, 0) == -1)
					return -1;
				if (!c2p)
					c1p_altered = 1;
			}
		} /* Iterate medias in session */
	} /* Iterate sessions */

	if (proxied == 0) {
		cp = pkg_malloc(ANORTPPROXY_LEN * sizeof(char));
		if (cp == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: out of memory\n");
			return -1;
		}
		anchor = anchor_lump(msg, body.s + body.len - msg->buf, 0, 0);
		if (anchor == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: anchor_lump failed\n");
			pkg_free(cp);
			return -1;
		}
		memcpy(cp, ANORTPPROXY, ANORTPPROXY_LEN);
		if (insert_new_lump_after(anchor, cp, ANORTPPROXY_LEN, 0) == NULL) {
			LOG(L_ERR, "ERROR: force_rtp_proxy2: insert_new_lump_after failed\n");
			pkg_free(cp);
			return -1;
		}
	}

	return 1;
}
static int unforce_rtp_proxy_f(struct sip_msg* msg, int node_idx)
{
	str callid, from_tag, to_tag;
	struct rtpp_node *node;
	struct iovec v[1 + 4 + 3] = {{NULL, 0}, {"D", 1}, {" ", 1}, {NULL, 0}, {" ", 1}, {NULL, 0}, {" ", 1}, {NULL, 0}};
						/* 1 */   /* 2 */   /* 3 */    /* 4 */   /* 5 */    /* 6 */   /* 1 */
	if (get_callid(msg, &callid) == -1 || callid.len == 0) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: can't get Call-Id field\n");
		return -1;
	}
	if (get_to_tag(msg, &to_tag) == -1) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: can't get To tag\n");
		return -1;
	}
	if (get_from_tag(msg, &from_tag) == -1 || from_tag.len == 0) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: can't get From tag\n");
		return -1;
	}
	STR2IOVEC(callid, v[3]);
	STR2IOVEC(from_tag, v[5]);
	STR2IOVEC(to_tag, v[7]);
	node = select_rtpp_node(callid, 1, node_idx);
	if (!node) {
		LOG(L_ERR, "ERROR: unforce_rtp_proxy: no available proxies\n");
		return -1;
	}
	send_rtpp_command(node, v, (to_tag.len > 0) ? 8 : 6);

	return 1;
}


/* initialization of rtp_proxy module */
int rtpproxy_init()
{

	memset(&rtpp_list, 0, sizeof(rtpp_list));
	rtpp_node_count = 0;
	if (rtpproxy_enable) {
		/* Make rtp proxies list. */
		char *p, *p1, *p2, *plim;

		p = rtpproxy_sock;
		plim = p + strlen(p);
		for(;;) {
			struct rtpp_node *pnode;
			int weight;

			weight = 1;
			while (*p && isspace(*p))
				++p;
			if (p >= plim)
				break;
			p1 = p;
			while (*p && !isspace(*p))
				++p;
			if (p <= p1)
				break; /* may happen??? */
			/* Have weight specified? If yes, scan it */
			p2 = memchr(p1, '=', p - p1);
	

			if (p2 != NULL) {
				weight = strtoul(p2 + 1, NULL, 10);
			} else {
				p2 = p;
			}
			pnode = pkg_malloc(sizeof(struct rtpp_node));
			if (pnode == NULL) {
				LOG(L_ERR, "nathelper: Can't allocate memory\n");
				return -1;
			}
			memset(pnode, 0, sizeof(*pnode));
			pnode->rn_recheck_ticks = 0;
			pnode->rn_weight = weight;
			pnode->rn_umode = 0;
			pnode->rn_fd = -1;
			pnode->rn_disabled = 0;
			pnode->rn_url = pkg_malloc(p2 - p1 + 1);
	
			LOG(L_INFO,"INFO:"M_NAME"node is created\n") ;

		

			if (pnode->rn_url == NULL) {
				LOG(L_ERR, "nathelper: Can't allocate memory\n");
				return -1;
			}
	
			/*LOG(L_CRIT,"started to add the pnode to rtpp list\n") ; */

			/* adding proxy nodes to the list */
			memmove(pnode->rn_url, p1, p2 - p1);
			pnode->rn_url[p2 - p1] = 0;
			if (rtpp_list.rn_first == NULL) {
				rtpp_list.rn_first = pnode;
			} else {
				rtpp_list.rn_last->rn_next = pnode;
			}
			rtpp_list.rn_last = pnode;
			++rtpp_node_count;
			
			/* Leave only address in rn_address */
			pnode->rn_address = pnode->rn_url;
	

			if (strncmp(pnode->rn_address, "udp:", 4) == 0) {
				pnode->rn_umode = 1;
				pnode->rn_address += 4;
			} else if (strncmp(pnode->rn_address, "udp6:", 5) == 0) {
				pnode->rn_umode = 6;
				pnode->rn_address += 5;
			} else if (strncmp(pnode->rn_address, "unix:", 5) == 0) {
				pnode->rn_umode = 0;
				pnode->rn_address += 5;
			}	
		 /* 	LOG(L_CRIT,"is getting out of the rtp block\n") ;*/
		}
	}
	return 1;
}

int rtpproxy_child_init(int rank)
{
	int n;
	char *cp;
	struct addrinfo hints, *res;
	struct rtpp_node *pnode;
	
	LOG(L_INFO,"INFO:"M_NAME":mod_init: Initialization of module/pcscf/rtpproxy in child [%d] \n",
		rank);
	/* Iterate known RTP proxies - create sockets */	
	for (pnode = rtpp_list.rn_first; pnode != NULL; pnode = pnode->rn_next) {
		char *old_colon;

		if (pnode->rn_umode == 0)
			goto rptest;
		/*
		 * This is UDP or UDP6. Detect host and port; lookup host;
		 * do connect() in order to specify peer address
		 */
		old_colon = cp = strrchr(pnode->rn_address, ':');
		if (cp != NULL) {
			old_colon = cp;
			*cp = '\0';
			cp++;
		}
		if (cp == NULL || *cp == '\0')
			cp = CPORT;

		memset(&hints, 0, sizeof(hints));
		hints.ai_flags = 0;
		hints.ai_family = (pnode->rn_umode == 6) ? AF_INET6 : AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		if ((n = getaddrinfo(pnode->rn_address, cp, &hints, &res)) != 0) {
			LOG(L_ERR, "nathelper: getaddrinfo: %s\n", gai_strerror(n));
			return -1;
		}
		if (old_colon)
			*old_colon = ':'; /* restore rn_address */

		pnode->rn_fd = socket((pnode->rn_umode == 6)
		    ? AF_INET6 : AF_INET, SOCK_DGRAM, 0);
		if (pnode->rn_fd == -1) {
			LOG(L_ERR, "nathelper: can't create socket\n");
			freeaddrinfo(res);
			return -1;
		}

		if (connect(pnode->rn_fd, res->ai_addr, res->ai_addrlen) == -1) {
			LOG(L_ERR, "nathelper: can't connect to a RTP proxy\n");
			close(pnode->rn_fd);
			pnode->rn_fd = -1;
			freeaddrinfo(res);
			return -1;
		}
		freeaddrinfo(res);
rptest:
		pnode->rn_disabled = rtpp_test(pnode, 0, 1);
	}

	if (!rtpproxy_enable)
		rtpproxy_disable_tout = -1;
		/* rtpproxy  */
	return 1;		
}

//static int check_user_natted(struct sip *msg, struct sip_uri *uri){
int P_SDP_manipulate(struct sip_msg *msg,char *str1,char *str2)
{
	int response = CSCF_RETURN_FALSE ;
	int method;
	struct sip_msg *req=0;

	if (!pcscf_nat_enable || !rtpproxy_enable) return CSCF_RETURN_FALSE;
	
    if( check_content_type(msg) ) 
    {
	    if (msg->first_line.type == SIP_REQUEST) method = msg->first_line.u.request.method_value;
	    else {
	    	req = cscf_get_request_from_reply(msg);
	    	if (req){
	    		method = req->first_line.u.request.method_value;
	    	}else method=METHOD_UNDEF;
	    }
		switch(method)
		{	
		    case METHOD_INVITE:
		    	if (msg->first_line.type == SIP_REQUEST){
			 		/* on INVITE */
					/* check the sdp if it has a 1918 */
					if(1)
					{
					/* get rtp_proxy/nathelper to open ports - get a iovec*/
						response = force_rtp_proxy2_f(msg,"","") ;
						LOG(L_CRIT,"DBG:"M_NAME":P_SDP_manipulate: ... rtp proxy done\n");			    	
				    } else {			
						/* using public ip */
						LOG(L_CRIT,"DBG:"M_NAME":P_SDP_manipulate: ... found public network in SDP.\n");
				    	response = CSCF_RETURN_FALSE ;
				    }
		    	}else{
		    		if (msg->first_line.u.reply.statuscode == 183 ||
				    	(msg->first_line.u.reply.statuscode >= 200 &&
				    	 msg->first_line.u.reply.statuscode < 300)) {
					    if(1)
					    {
						/* sdp_1918(msg) */
						/* str1 & str2 must be something */
						    response = force_rtp_proxy2_f(msg, "", "") ;						
							LOG(L_CRIT,"DBG:"M_NAME":P_SDP_manipulate: ... rtp proxy done\n");			    	
						} else {
							/* public ip found */
							response = CSCF_RETURN_FALSE ;						
						}
						break;
			    	}
		    	}
			    break ;
		    case METHOD_BYE:
		    case METHOD_CANCEL:
				LOG(L_CRIT,"DBG:"M_NAME":P_SDP_manipulate: on BYE/CANCEL...\n");
		    	if (msg->first_line.type == SIP_REQUEST){
    			    /* request/response not acceptable */
				    response = unforce_rtp_proxy_f(msg,-1) ;
		    	}
				break;
	
		    default:
		    	response = CSCF_RETURN_FALSE;
				break; 
		}    
    } else {
		LOG(L_ERR, "ERROR:check_content_type: parse error:"
			"see the content_type block\n");
		response = -1 ;
    }	

return response ;
}



