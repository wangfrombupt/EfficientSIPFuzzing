/**
 * $Id: diameter_peer.c 584 2008-09-30 11:49:40Z albertoberlios $
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
 * CDiameterPeer General procedures for init/start-up/shut-down
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <stdlib.h>
#include <sys/wait.h> 
#include <signal.h>
#include <stdio.h>

#include "utils.h"
#include "diameter_peer.h"

#include "config.h"
#include "acceptor.h"
#include "timer.h"
#include "peermanager.h"
#include "worker.h"
#include "api_process.h"
#include "transaction.h"
#include "session.h"

#ifdef CDP_FOR_SER
	#include "../../pt.h"
#endif

dp_config *config=0;		/**< Configuration for this diameter peer 	*/

int *shutdownx=0;			/**< whether a shutdown is in progress		*/
gen_lock_t *shutdownx_lock; /**< lock used on shutdown				*/

pid_t *dp_first_pid;		/**< first pid that we started from		*/

pid_list_head_t *pid_list;	/**< list of local processes			*/
gen_lock_t *pid_list_lock;	/**< lock for list of local processes	*/

extern handler_list *handlers; 		/**< list of handlers */
extern gen_lock_t *handlers_lock;	/**< lock for list of handlers */


/**
 * Add a pid to the local process list.
 * @param pid newly forked pid
 * @returns 1 on success or 0 on error
 */
inline int dp_add_pid(pid_t pid)
{
	pid_list_t *n;
	lock_get(pid_list_lock);
	n = shm_malloc(sizeof(pid_list_t));
	if (!n){
		LOG_NO_MEM("shm",sizeof(pid_list_t));
		lock_release(pid_list_lock);
		return 0;
	}
	n->pid = pid;
	n->next = 0;
	n->prev = pid_list->tail;
	if (!pid_list->head) pid_list->head = n;
	if (pid_list->tail) pid_list->tail->next = n;
	pid_list->tail = n;
	lock_release(pid_list_lock);
	return 1;
}

/**
 * Returns the last pid in the local process list.
 */
inline int dp_last_pid()
{
	int pid;
	lock_get(pid_list_lock);
	if (pid_list->tail)	pid = pid_list->tail->pid;
	else pid = -1;
	lock_release(pid_list_lock);
	return pid;
}

/**
 * Delete a pid from the process list
 * @param pid - the pid to remove 
 */
inline void dp_del_pid(pid_t pid)
{	
	pid_list_t *i;
	lock_get(pid_list_lock);
	i = pid_list->head;
	if (!i) {
		lock_release(pid_list_lock);
		return;
	}
	while(i && i->pid!=pid) i = i->next;
	if (i){
		if (i->prev) i->prev->next = i->next;
		else pid_list->head = i->next;
		if (i->next) i->next->prev = i->prev;
		else pid_list->tail = i->prev;
		shm_free(i);
	}
	lock_release(pid_list_lock);
}


/**
 * Initialize the CDiameterPeer from a configuration file.
 * The file is kept as dtd. See configdtd.h for the DTD and ConfigExample.xml.
 * @param cfg_filename - file with the configuration
 * @returns 1 on success, 0 on error
 */
int diameter_peer_init(char *cfg_filename)
{	
	pid_list_t *i,*j;

	config = parse_dp_config(cfg_filename);
	if (!config) {
		LOG(L_ERR,"ERROR:init_diameter_peer(): Error loading configuration file. Aborting...\n");
		goto error;
	}
	log_dp_config(L_INFO,config);
	
	dp_first_pid = shm_malloc(sizeof(pid_t));
	if (!dp_first_pid){
		LOG_NO_MEM("shm",sizeof(pid_t));
		goto error;
	}
	*dp_first_pid = getpid();
	
	shutdownx = shm_malloc(sizeof(int));
	if (!shutdownx){
		LOG_NO_MEM("shm",sizeof(int));
		goto error;
	}
	*shutdownx = 0;
	
	shutdownx_lock = lock_alloc();
	if (!shutdownx_lock){
		LOG_NO_MEM("shm",sizeof(gen_lock_t));
		goto error;
	}
	shutdownx_lock = lock_init(shutdownx_lock);

	handlers_lock = lock_alloc();
	if (!handlers_lock){
		LOG_NO_MEM("shm",sizeof(gen_lock_t));
		goto error;
	}
	handlers_lock = lock_init(handlers_lock);

	handlers = shm_malloc(sizeof(handler_list));
	if (!handlers){
		LOG_NO_MEM("shm",sizeof(handler_list));
		goto error;
	}
	handlers->head=0;
	handlers->tail=0;

	/* init the pid list */
	pid_list = shm_malloc(sizeof(pid_list_head_t));
	pid_list_lock = lock_alloc();
	pid_list_lock = lock_init(pid_list_lock);

	/* init shared mem pointers before forking */
	timer_cdp_init();
	worker_init();

	/* init the peer manager */
	peer_manager_init(config);
	
	/* init the session */
	if (!session_init()) goto error;
	

	/* init diameter transactions */
	trans_init();
	
	/* add callback for messages - used to implement the API */
	cb_add(api_callback,0);
	
	return 1;
	
error:
	if (shutdownx) shm_free(shutdownx);
	if (config) free_dp_config(config);
	i = pid_list->head;
	while(i){
		j = i->next;
		shm_free(i);
		i = j;
	}
	shm_free(pid_list);
	lock_get(pid_list_lock);
	lock_destroy(pid_list_lock);
	lock_dealloc((void*)pid_list_lock);
	return 0;	

}


/**
 * Start the CDiameterPeer operations.
 * It forks all the processes required.
 * @param blocking - if this is set, use the calling processes for the timer and never 
 * return; else fork a new one for the timer and return
 * @returns 1 on success, 0 on error, never if blocking
 */ 
int diameter_peer_start(int blocking)
{
	int pid;
	int k=0;


	/* Fork the acceptor process */
	#ifdef CDP_FOR_SER		
		pid = fork_process(1000,"cdp_acceptor",1);
	#else
		pid = fork();
	#endif
	if (pid==-1){
		LOG(L_CRIT,"ERROR:init_diameter_peer(): Error on fork() for acceptor!\n");
		return 0;
	}
	if (pid==0) {
		acceptor_process(config);
		LOG(L_CRIT,"ERROR:init_diameter_peer(): acceptor_process finished without exit!\n");
		exit(-1);		
	}else{
		dp_add_pid(pid);
	}

	/* fork workers */
	for(k=0;k<config->workers;k++){
		#ifdef CDP_FOR_SER		
			pid = fork_process(1001+k,"cdp_worker",1);
		#else
			pid = fork();
		#endif
		if (pid==-1){
			LOG(L_CRIT,"ERROR:init_diameter_peer(): Error on fork() for worker!\n");
			return 0;
		}
		if (pid==0) {
			srandom(time(0)*k);
			#ifdef CDP_FOR_SER
				snprintf(pt[process_no].desc, MAX_PT_DESC,
					"cdp worker child=%d", k );
			#endif	
			worker_process(k);
			LOG(L_CRIT,"ERROR:init_diameter_peer(): worker_process finished without exit!\n");
			exit(-1);		
		}else{
			dp_add_pid(pid);
		}
	}
				
	/* fork/become timer */
	if (blocking) {
		dp_add_pid(getpid());
		timer_process(1);
	}		
	else{		
		#ifdef CDP_FOR_SER		
			pid = fork_process(1001,"cdp_timer",1);
		#else
			pid = fork();
		#endif
		if (pid==-1){
			LOG(L_CRIT,"ERROR:init_diameter_peer(): Error on fork() for timer!\n");
			return 0;
		}
		if (pid==0) {
			timer_process(0);
			LOG(L_CRIT,"ERROR:init_diameter_peer(): timer_process finished without exit!\n");
			exit(-1);		
		}else{			
			dp_add_pid(pid);
		}
	}
	
	return 1;
}

extern int memlog;

/**
 * Shutdown the CDiameterPeer nicely.
 * It stops the workers, disconnects peers, drops timers and wait for all processes to exit.
 */
void diameter_peer_destroy()
{
	int pid,status;
	handler *h;
	
	lock_get(shutdownx_lock);
	if (*shutdownx) {
		/* already other process is cleaning stuff */
		lock_release(shutdownx_lock);			
		return;
	}else {
		/* indicating that we are shuting down */
		*shutdownx = 1;
		lock_release(shutdownx_lock);
	}


	worker_poison_queue();

	/* wait for all childs to clean up nicely (acceptor, receiver, timer, workers) */
	LOG(L_INFO,"INFO:destroy_diameter_peer(): Terminating all childs...\n");
	while(pid_list->tail){
		pid = dp_last_pid();
		if (pid<=0||pid==getpid()){
			dp_del_pid(pid);
			continue;
		}
		LOG(L_INFO,"INFO:destroy_diameter_peer(): Waiting for child [%d] to terminate...\n",pid);
		if (waitpid(pid,&status,0)<0){
			dp_del_pid(pid);
			continue;
		}
		if (!WIFEXITED(status) /*|| WIFSIGNALED(status)*/){
			worker_poison_queue();
			sleep(1);			
		} else {
			dp_del_pid(pid);
		}

	}
	LOG(L_INFO,"INFO:destroy_diameter_peer(): All processes terminated. Cleaning up.\n");
	
	/* clean upt the timer */
	timer_cdp_destroy();
	
	/* cleaning up workers */
	worker_destroy();
	
	/* cleaning peer_manager */
	peer_manager_destroy();
	
	/* cleaning up sessions */
	session_destroy();

	/* cleaning up global vars */
/*	lock_get(pid_list_lock);*/
	shm_free(dp_first_pid);
	shm_free(pid_list);
	lock_destroy(pid_list_lock);
	lock_dealloc((void*)pid_list_lock);
	
	shm_free(shutdownx);
	
	lock_destroy(shutdownx_lock);
	lock_dealloc((void*)shutdownx_lock);
	
	lock_get(handlers_lock);
	while(handlers->head){
		h = handlers->head->next;
		shm_free(handlers->head);
		handlers->head = h;
	}
	lock_destroy(handlers_lock);
	lock_dealloc((void*)handlers_lock);
	shm_free(handlers);
		
	free_dp_config(config);	
	LOG(L_CRIT,"INFO:destroy_diameter_peer(): Bye Bye from C Diameter Peer test\n");

#ifndef CDP_FOR_SER
	#ifdef PKG_MALLOC
		LOG(memlog, "Memory status (pkg):\n");
		//pkg_status();
		#ifdef pkg_sums
			pkg_sums();
		#endif 
	#endif
	#ifdef SHM_MEM
		LOG(memlog, "Memory status (shm):\n");
		//shm_status();
		#ifdef shm_sums
			shm_sums();
		#endif 
		/* zero all shmem alloc vars that we still use */
		shm_mem_destroy();
	#endif
#endif	
}


