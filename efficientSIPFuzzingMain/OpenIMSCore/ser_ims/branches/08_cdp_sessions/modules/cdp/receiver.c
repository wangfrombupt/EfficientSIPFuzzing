/**
 * $Id: receiver.c 468 2007-10-31 11:20:36Z albertoberlios $
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
 * CDiameterPeer Receiver process procedures 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils.h"
#include "globals.h"
#include "diameter_api.h"
#include "peerstatemachine.h"
#include "peermanager.h"
#include "config.h"

#include "receiver.h"

extern dp_config *config;		/**< Configuration for this diameter peer 	*/

int dp_add_pid(pid_t pid);
void dp_del_pid(pid_t pid);

void receive_loop(int sock);

void receive_message(AAAMessage *msg,int sock);

peer *this_peer=0;			/**< pointer to the peer serviced by this process */

/** prefix for the send FIFO pipes */
#define PIPE_PREFIX "/tmp/cdp_send_"
int pipe_fd;		/**< file descriptor for reading from the send pipe */
int pipe_fd_out;	/**< file descriptor for writting to the send pipe */
str pipe_name;		/**< full path to the pipe	*/

/**
 * Sets the send pipe name for the peer serviced by this process.
 */
static inline void set_peer_pipe()
{
	if (!this_peer) return;
//	lock_get(this_peer->lock);
	this_peer->send_pipe = pipe_name;
//	lock_release(this_peer->lock);
}

/**
 * Initializes the receiver.
 * @param sock - the socket to initialize with
 * @param p - the peer to initialize with
 */
void receiver_init(int sock,peer *p)
{
	this_peer = p;
	
	pipe_name.s = shm_malloc(sizeof(PIPE_PREFIX)+64);
	sprintf(pipe_name.s,"%s%d_%d_%d",PIPE_PREFIX,(unsigned int) time(0),sock,getpid());
	pipe_name.len = strlen(pipe_name.s);
		
	set_peer_pipe();	

	mkfifo(pipe_name.s, 0666);	
	pipe_fd = open(pipe_name.s, O_RDONLY | O_NDELAY);
	if (pipe_fd<0){
		LOG(L_ERR,"ERROR:receiver_process(): FIFO open failed > %s\n",strerror(errno));
	}
	// we open it for writting just to keep it alive - won't close when all other writers close it
	pipe_fd_out = open(pipe_name.s, O_WRONLY);
}

/**
 * The Receiver Process - calls the receiv_loop and it never returns.
 * @param sock - socket to receive data from
 * @returns never, when disconnected it will exit
 */
void receiver_process(int sock)
{
	LOG(L_INFO,"INFO:receiver_process(): [%d] Receiver process starting up...\n",sock);

		
	receive_loop(sock);
	LOG(L_INFO,"INFO:receiver_process(): [%d]... Receiver process cleaning-up.\n",sock);
	close(sock);
	close(pipe_fd);
	close(pipe_fd_out);
	remove(pipe_name.s);
	if (this_peer){
		lock_get(this_peer->lock);
		this_peer->send_pipe.s=0;
		this_peer->send_pipe.len=0;
		lock_release(this_peer->lock);
	}
	shm_free(pipe_name.s);
//done:		
	/* remove pid from list of running processes */
	dp_del_pid(getpid());
	
#ifdef CDP_FOR_SER
	drop_my_process();		
#else
#ifdef PKG_MALLOC
	#ifdef PKG_MALLOC
		LOG(memlog, "Receiver[%d] Memory status (pkg):\n",sock);
		//pkg_status();
		#ifdef pkg_sums
			pkg_sums();
		#endif 
	#endif
#endif
#endif		
		
	LOG(L_INFO,"INFO:receiver_process(): [%d]... Receiver process finished.\n",sock);
	exit(0);
}

/**
 * Select on sockets for receiving.
 * Selects on both the socket and on the send pipe.
 * @param s - the receive socket
 * @param buf - buffer to read into
 * @param len - max length of the read buffer
 * @param opt - recv() flags
 * @returns number of bytes read or -1 on error 
 */ 
static inline int select_recv(int s,void * buf,int len,int opt)
{
	fd_set rfds,efds;
	struct timeval tv;
	int n,max;
	AAAMessage *msg=0;
	int r=0,cnt=0;
	
//	if (shutdownx) return -1;
	max = s;
	if (pipe_fd>max) max = pipe_fd;
	n = 0;
	
	while(!n){
   		if (shutdownx&&*shutdownx) break;	

		FD_ZERO(&rfds);
		FD_SET(s,&rfds);
		FD_SET(pipe_fd,&rfds);
		FD_ZERO(&efds);
		FD_SET(s,&efds);
		tv.tv_sec=1;
		tv.tv_usec=0;

//		LOG(L_CRIT,"ERROR:select_recv(): HERE\n");
		
		n = select(max+1,&rfds,0,&efds,&tv);
		if (n==-1){
			if (shutdownx&&*shutdownx) return -1;
			LOG(L_ERR,"ERROR:select_recv(): %s\n",strerror(errno));
			return -1;
		}else
			if (n){
				if (FD_ISSET(s,&efds)) return -1;				
				if (FD_ISSET(pipe_fd,&rfds)) {					
					LOG(L_DBG,"DBG:select_recv(): There is something on the pipe\n");
					cnt = read(pipe_fd,&msg,sizeof(AAAMessage *));
					LOG(L_DBG,"DBG:select_recv(): Pipe says [%p] %d\n",msg,cnt);
					if (cnt==0){
						//This is very stupid and might not work well - droped messages... to be fixed
						LOG(L_INFO,"INFO:select_recv(): ReOpening pipe for read. This should not happen...\n");
						close(pipe_fd);
						pipe_fd = open(pipe_name.s, O_RDONLY | O_NDELAY);
						goto receive;
					}
					if (cnt<sizeof(AAAMessage *)){
						if (cnt<0) LOG(L_ERR,"ERROR:select_recv(): Error reading from pipe\n");
						r = -1;
						goto receive;
					}	
					
					while( (cnt=write(s,msg->buf.s,msg->buf.len))==-1 ) {
						if (errno==EINTR)
							continue;
						LOG(L_ERR,"ERROR:select_recv(): write returned error> %s\n",
							strerror(errno));
						close(s);
						AAAFreeMessage(&msg);		
						r = -1;
						return r;
					}
											
					if (cnt!=msg->buf.len){
						LOG(L_ERR,"ERROR:select_recv(): only wrote %d/%d bytes\n",cnt,msg->buf.len);
						close(s);
						AAAFreeMessage(&msg);		
						r = -1;
						return r;
					}
					AAAFreeMessage(&msg);
					//don't return, maybe there is something to read
				}
receive:
				if (FD_ISSET(s,&rfds)) {
					cnt = recv(s,buf,len,opt);
					if (cnt==0) return -1;
					else return cnt;
				}
			}
		//LOG(L_ERR,".");
	}
	return r;
}

/** length of the Diameter message header */
#define hdr_len 20

/** 
 * Receive Loop for Diameter messages.
 * Decodes the message and calls receive_message().
 * @param sock - the socket to receive from
 * @returns when the socket is closed
 */
void receive_loop(int sock)
{
	char buf[hdr_len],*msg;
	int buf_len,length,version,cnt,msg_len;
	AAAMessage *dmsg;
	
    while(!*shutdownx){
   		buf_len=0;
    	while(buf_len<1){	
    		cnt = select_recv(sock,buf+buf_len,1,0);
	    	if (cnt<0) goto error;
	    	buf_len+=cnt;
    	}
    	version = (unsigned char)buf[0];
    	if (version!=1) {
    		LOG(L_ERR,"ERROR:receive_loop():[%d] Recv Unknown version [%d]\n",sock,(unsigned char)buf[0]);
			continue;    		
    	}    	
    	while(buf_len<hdr_len){
	    	cnt = select_recv(sock,buf+buf_len,hdr_len-buf_len,0);
	    	if (cnt<0) goto error;
	    	buf_len+=cnt;
    	}
    	length = get_3bytes(buf+1);
    	if (length>DP_MAX_MSG_LENGTH){
			LOG(L_ERR,"ERROR:receive_loop():[%d] Msg too big [%d] bytes\n",sock,length);
			goto error;
    	}
    	LOG(L_DBG,"DBG:receive_loop():[%d] Recv Version %d Length %d\n",sock,version,length);
    	msg = shm_malloc(length);
    	if (!msg) {
    		LOG_NO_MEM("shm",length);
			goto error;
    	}
    		
    	memcpy(msg,buf,hdr_len);
    	msg_len=hdr_len;
    	while(msg_len<length){
    		cnt = select_recv(sock,msg+msg_len,length-msg_len,0);
	    	if (cnt<0) {
	    		shm_free(msg);
		    	goto error;
	    	}
    		msg_len+=cnt;
    	}
    	LOG(L_DBG,"DBG:receive_loop():[%d] Recv message complete\n",sock);
    	
    	dmsg = AAATranslateMessage((unsigned char*)msg,(unsigned int)msg_len,1);
    	
    	/*shm_free(msg);*/

		if (dmsg) receive_message(dmsg,sock);
		else{
			shm_free(msg);
		}
		
    }
error:
	if (this_peer) {
		if (this_peer->I_sock == sock) sm_process(this_peer,I_Peer_Disc,0,0,sock);
		if (this_peer->R_sock == sock) sm_process(this_peer,R_Peer_Disc,0,0,sock);
	}
    LOG(L_ERR,"INFO:receive_loop():[%d] Client closed connection or error... BYE\n",sock);
}

/**
 * Initiate a connection to a peer.
 * @param p - peer to connect to
 * @returns socket if OK, -1 on error
 */
int peer_connect(peer *p)
{
	int sock;
	int pid;
	unsigned char servip[4];
	struct sockaddr_in servaddr;
	unsigned int option = 1;
	struct hostent *host=0;

	host = gethostbyname(p->fqdn.s);
	if (!host){
		LOG(L_WARN,"WARNING:peer_connect(): Error opening connection to %.*s:%d >%s\n",
			p->fqdn.len,p->fqdn.s,p->port,strerror(h_errno));
		goto error;
	}
		
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if ( sock==-1) {
		LOG(L_ERR,"ERROR:peer_connect(): cannot connect, failed to create "
				"new socket\n");
		goto error;
	}
	memset( &servip, 0, sizeof(servip) );
	memcpy( &servip, host->h_addr_list[0], 4);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(p->port);
	servaddr.sin_addr.s_addr = *(unsigned int*)servip;

	if (connect(sock,(struct sockaddr*)&servaddr, sizeof(struct sockaddr_in))!=0) {
		LOG(L_WARN,"WARNING:peer_connect(): Error opening connection to %.*s:%d >%s\n",
			p->fqdn.len,p->fqdn.s,p->port,strerror(errno));		
		goto error;
	}
	
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	
	LOG(L_INFO,"INFO:peer_connect(): Peer %.*s:%d connected\n",p->fqdn.len,p->fqdn.s,p->port);

	
	receiver_init(sock,p);
	
	#ifdef CDP_FOR_SER
		pid = fork_process(p->port,"receiver I",0);
	#else
		pid = fork();
	#endif
	if (pid<0){
		LOG(L_ERR,"ERROR:peer_connect(): fork() failed > %s\n",strerror(errno));
		goto error;
	}
	if (pid==0){
		/* child */
		receiver_process(sock);
		LOG(L_CRIT,"ERROR:peer_connect(): receiver_process finished without exit!\n");
		exit(-1);
	}else{
		/* parent */
		LOG(L_INFO,"INFO:peer_connect(): Receiver process forked [%d]\n",pid);
		
		dp_add_pid(pid);
	}
		
	return sock;
error:
	return -1;	
}


/**
 * Sends a message to a peer (to be called from other processes).
 * This just writes the pointer to the message in the send pipe. The specific
 * peer process will pick that up and send the message, as only that specific
 * process has the id of socket (we are forking the peers dynamically and as such,
 * the sockets are not visible between processes).
 * @param p - the peer to send to
 * @param msg - the message to send
 * @returns 1 on success, 0 on failure
 */
int peer_send_msg(peer *p,AAAMessage *msg)
{
	int fd,n;
	if (!AAABuildMsgBuffer(msg)) return 0;
	if (!p->send_pipe.s) {
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s has no attached send pipe\n",p->fqdn.len,p->fqdn.s);
		return 0;
	}
	fd = open(p->send_pipe.s,O_WRONLY);
	if (fd<0){
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s error on pipe open > %s\n",p->fqdn.len,p->fqdn.s,strerror(errno));		
		return 0;
	}
	LOG(L_DBG,"DBG:peer_send_msg(): Pipe push [%p]\n",msg);
	n = write(fd,&msg,sizeof(AAAMessage *));
	if (n<0) {
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s error on pipe write > %s\n",p->fqdn.len,p->fqdn.s,strerror(errno));		
		close(fd);
		return 0;
	}
	if (n!=sizeof(AAAMessage *)) {
		LOG(L_ERR,"ERROR:peer_send_msg(): Peer %.*s error on pipe write > only %d bytes written\n",p->fqdn.len,p->fqdn.s,n);		
		close(fd);
		return 0;
	}
	close(fd);
	
	return 1;
}

/**
 * Send a message to a peer (only to be called from the receiver process).
 * This directly writes the message on the socket. It is used for transmission during
 * the Capability Exchange procedure, when the send pipes are not opened yet.
 * @param p - the peer to send to
 * @param sock - the socket to send through
 * @param msg - the message to send
 * @param locked - whether the caller locked the peer already
 * @returns 1 on success, 0 on error
 */
int peer_send(peer *p,int sock,AAAMessage *msg,int locked)
{
	int n;
//	LOG(L_CRIT,"[%d]\n",sock);
	
	if (!p||!msg||sock<0) return 0;
	
	if (!AAABuildMsgBuffer(msg)) return 0;
	
	if (!locked) lock_get(p->lock);

	while( (n=write(sock,msg->buf.s,msg->buf.len))==-1 ) {
		if (errno==EINTR)
			continue;
		LOG(L_ERR,"ERROR:peer_send(): write returned error: %s\n",
			strerror(errno));
		if (p->I_sock==sock) sm_process(p,I_Peer_Disc,0,1,p->I_sock);
		if (p->R_sock==sock) sm_process(p,R_Peer_Disc,0,1,p->R_sock);
		if (!locked) lock_release(p->lock);
		AAAFreeMessage(&msg);		
		return 0;
	}

	if (n!=msg->buf.len){
		LOG(L_ERR,"ERROR:peer_send(): only wrote %d/%d bytes\n",n,msg->buf.len);
		if (!locked) lock_release(p->lock);
		AAAFreeMessage(&msg);		
		return 0;
	}
	if (!locked) lock_release(p->lock);
	AAAFreeMessage(&msg);			
	return 1;	
}


/**
 * Receives a mesasge and does basic processing or call the sm_process().
 * This gets called from the receive_loop for every message that is received.
 * @param msg - the message received
 * @param sock - socket received on
 */
void receive_message(AAAMessage *msg,int sock)
{
	AAA_AVP *avp1,*avp2;
	LOG(L_DBG,"DBG:receive_message(): [%d] Recv msg %d\n",sock,msg->commandCode);

	if (!this_peer) {
		this_peer = get_peer_from_sock(sock);
		set_peer_pipe();
	}
	
	if (!this_peer){
		switch (msg->commandCode){
			case Code_CE:
				if (is_req(msg)){
					avp1 = AAAFindMatchingAVP(msg,msg->avpList.head,AVP_Origin_Host,0,0);
					avp2 = AAAFindMatchingAVP(msg,msg->avpList.head,AVP_Origin_Realm,0,0);
					if (avp1&&avp2){
						this_peer = get_peer_from_fqdn(avp1->data,avp2->data);
					}
					if (!this_peer) {
						LOG(L_ERR,"ERROR:receive_msg(): Received CER from unknown peer (accept unknown=%d) -ignored\n",
							config->accept_unknown_peers);
						AAAFreeMessage(&msg);
					}else{
						set_peer_pipe();						
						sm_process(this_peer,R_Conn_CER,msg,0,sock);
					}
				}
				else{
					LOG(L_ERR,"ERROR:receive_msg(): Received CEA from an unknown peer -ignored\n");
					AAAFreeMessage(&msg);
				}
				break;
			default:
				LOG(L_ERR,"ERROR:receive_msg(): Received non-CE from an unknown peer -ignored\n");
				AAAFreeMessage(&msg);				
		}
	}else{
		touch_peer(this_peer);
		switch (this_peer->state){
			case Wait_I_CEA:
				if (msg->commandCode!=Code_CE||is_req(msg)){
					sm_process(this_peer,I_Rcv_Non_CEA,msg,0,sock);
				}else
					sm_process(this_peer,I_Rcv_CEA,msg,0,sock);
				break;
			case I_Open:			
				switch (msg->commandCode){
					case Code_CE:
						if (is_req(msg)) sm_process(this_peer,I_Rcv_CER,msg,0,sock);	
									else sm_process(this_peer,I_Rcv_CEA,msg,0,sock);
						break;
					case Code_DW:
						if (is_req(msg)) sm_process(this_peer,I_Rcv_DWR,msg,0,sock);	
									else sm_process(this_peer,I_Rcv_DWA,msg,0,sock);
						break;
					case Code_DP:
						if (is_req(msg)) sm_process(this_peer,I_Rcv_DPR,msg,0,sock);	
									else sm_process(this_peer,I_Rcv_DPA,msg,0,sock);
						break;
					default:
						sm_process(this_peer,I_Rcv_Message,msg,0,sock);
				}				
				break;				
			case R_Open:			
				switch (msg->commandCode){
					case Code_CE:
						if (is_req(msg)) sm_process(this_peer,R_Rcv_CER,msg,0,sock);	
									else sm_process(this_peer,R_Rcv_CEA,msg,0,sock);
						break;
					case Code_DW:
						if (is_req(msg)) sm_process(this_peer,R_Rcv_DWR,msg,0,sock);	
									else sm_process(this_peer,R_Rcv_DWA,msg,0,sock);
						break;
					case Code_DP:
						if (is_req(msg)) sm_process(this_peer,R_Rcv_DPR,msg,0,sock);	
									else sm_process(this_peer,R_Rcv_DPA,msg,0,sock);
						break;
					default:
						sm_process(this_peer,R_Rcv_Message,msg,0,sock);
				}				
				break;				
			default:
				LOG(L_ERR,"ERROR:receive_msg(): Received msg while peer in state %d -ignored\n",this_peer->state);
				AAAFreeMessage(&msg);								
		}
	}
}
