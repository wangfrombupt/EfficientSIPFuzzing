/** 
 * $Id: worker.c 561 2008-05-22 15:40:28Z vingarzan $
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
 * CDiameterPeer Worker Procedures
 * 
 * This the process pool representation that is used for processing incoming messages. 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */
#include <time.h> 
#include <stdlib.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/ipc.h>
#include <sys/sem.h>

#include "utils.h"
#include "globals.h"
#include "config.h"

#include "worker.h"
#include "diameter_api.h"

/* defined in ../diameter_peer.c */
int dp_add_pid(pid_t pid);
void dp_del_pid(pid_t pid);

extern dp_config *config;		/**< Configuration for this diameter peer 	*/

task_queue_t *tasks;			/**< queue of tasks */

cdp_cb_list_t *callbacks;		/**< list of callbacks for message processing */

struct sembuf cdp_sem_lock=  { 0, -1, 0};	/**< sembuf structure to lock a semaphore */
struct sembuf cdp_sem_unlock={ 0, +1, 0};	/**< sembuf structure to unlock a semaphore */

union semun {int val;struct semid_ds *buf;ushort *array;} 
	cdp_semun_lock   = {0}, 
	cdp_semun_unlock = {1},
	cdp_semun_init   = {0666|IPC_CREAT},
	cdp_semun_destroy= {0};
	
	
	
/**
 * Gets the lock on a semaphore and blocks until it is available.
 * This procedures does not consume CPU cycles as a busy-waiting would and it is used for
 * blocking on the task queue without a big impact on performance.
 * @param sid - semaphore id
 * @returns when the sempahore is aquired or shutdown
 */
static inline void cdp_lock_get(int sid)
{
	if((semop(sid, &cdp_sem_lock, 1)) == -1)
	{
		if (shutdownx&&(*shutdownx)) return;
    	LOG(L_INFO,"ERROR:cdp_lock_get(): Error on semop > %s\n",strerror(errno));
	}
}

/**
 * Releases the lock on a sempahore.
 * @param sid - the semaphore id
 */
static inline void cdp_lock_release(int sid)
{
	if( semctl(sid, 0, SETVAL, cdp_semun_unlock) == -1
	/*semop(sid, &cdp_sem_unlock, 1) == -1*/)
	{
		if (shutdownx&&(*shutdownx)) return;
    	LOG(L_INFO,"ERROR:cdp_lock_release(): Error on semop %s > %d: %s Q[%2d/%2d]\n",
    		sid==tasks->full?"full":"empty",errno,strerror(errno),
    		tasks->start,tasks->end);
	}
}

/**
 * Initializes the worker structures, like the task queue.
 */
void worker_init()
{
	tasks = shm_malloc(sizeof(task_queue_t));
	
	tasks->lock = lock_alloc();
	tasks->lock = lock_init(tasks->lock);
	
	tasks->empty = semget(IPC_PRIVATE,1,0666 | IPC_CREAT );
	if (tasks->empty==-1){
		LOG(L_ERR,"ERROR:worker_init(): Error creating semaphore for empty queue > %s\n",strerror(errno));
	}else
		semctl(tasks->empty, 0, SETVAL, cdp_semun_init );
	tasks->full = semget(IPC_PRIVATE,1, 0666 | IPC_CREAT );
	if (tasks->full==-1){
		LOG(L_ERR,"ERROR:worker_init(): Error creating semaphore for full queue > %s\n",strerror(errno));
	}else
		semctl(tasks->full, 0, SETVAL, cdp_semun_init);
	
	tasks->start = 0;
	tasks->end = 0;
	tasks->max = config->queue_length;
	tasks->queue = shm_malloc(tasks->max*sizeof(task_t));
	if (!tasks->queue) LOG_NO_MEM("shm",tasks->max*sizeof(task_t));
	memset(tasks->queue,0,tasks->max*sizeof(task_t));
		
	callbacks = shm_malloc(sizeof(cdp_cb_list_t));
	callbacks->head = 0; 
	callbacks->tail = 0;
	
	cdp_lock_get(tasks->empty);
//	lock_release(tasks->full);	
}

/**
 * Destroys the worker structures. 
 */
void worker_destroy()
{
	int i;
//	LOG(L_CRIT,"-1-\n");
/*	lock_get(tasks->lock);*/
	for(i=0;i<tasks->max;i++){
		if (tasks->queue[i].msg) AAAFreeMessage(&(tasks->queue[i].msg));
	}
//	LOG(L_CRIT,"-2-\n");	
	shm_free(tasks->queue);
	lock_destroy(tasks->lock);
	lock_dealloc((void*)tasks->lock);
//	LOG(L_CRIT,"-3-\n");	
	
	//lock_release(tasks->empty);
	semctl(tasks->empty, 0, IPC_RMID, cdp_semun_destroy);
//	LOG(L_CRIT,"-4-\n");	
	
	semctl(tasks->full, 0, IPC_RMID, cdp_semun_destroy);
	
	shm_free(tasks);
	
	while(callbacks->head)
		cb_remove(callbacks->head);
	shm_free(callbacks);
}

/*unsafe*/
int cb_add(cdp_cb_f cb,void *ptr)
{
	cdp_cb_t *x;
	x = shm_malloc(sizeof(cdp_cb_t));
	if (!x){
		LOG_NO_MEM("shm",sizeof(cdp_cb_t));
		return 0;
	}
	x->cb = cb;
	x->ptr = shm_malloc(sizeof(void*));
	if (!x->ptr){
		LOG_NO_MEM("shm",sizeof(void*));
		return 0;
	}
	*(x->ptr) = ptr;
	x->next = 0;
	x->prev = callbacks->tail;
	if (callbacks->tail) callbacks->tail->next = x;
	callbacks->tail = x;
	if (!callbacks->head) callbacks->head = x;
	return 1;	
}

/*unsafe*/
void cb_remove(cdp_cb_t *cb)
{
	cdp_cb_t *x;
	x = callbacks->head;
	while(x && x!=cb) x = x->next;
	if (!x) return;
	if (x->prev) x->prev->next = x->next;
	else callbacks->head = x->next;
	if (x->next) x->next->prev = x->prev;
	else callbacks->tail = x->prev;
	
	if (x->ptr) shm_free(x->ptr);
	shm_free(x);
}

/**
 * Adds a message as a task to the task queue.
 * This blocks if the task queue is full, untill there is space.
 * @param p - the peer that the message was received from
 * @param msg - the message
 * @returns 1 on success, 0 on failure (eg. shutdown in progress)
 */ 
int put_task(peer *p,AAAMessage *msg)
{
//	LOG(L_CRIT,"+1+\n");
	lock_get(tasks->lock);
//	LOG(L_CRIT,"+2+\n");
	while ((tasks->end+1)%tasks->max == tasks->start){
//		LOG(L_CRIT,"+3+\n");
		lock_release(tasks->lock);
//		LOG(L_CRIT,"+4+\n");
		if (*shutdownx) {
			cdp_lock_release(tasks->full);
			return 0;
		}
//		LOG(L_ERR,"+");
		cdp_lock_get(tasks->full);
//		LOG(L_CRIT,"+5+\n");
		lock_get(tasks->lock);
	}
	tasks->queue[tasks->end].p = p;
	tasks->queue[tasks->end].msg = msg;
	tasks->end = (tasks->end+1) % tasks->max;
	cdp_lock_release(tasks->empty);
	lock_release(tasks->lock);
	return 1;
}

/**
 * Remove and return the first task from the queue (FIFO).
 * This blocks until there is something in the queue.
 * @returns the first task from the queue or an empty task on error (eg. shutdown in progress)
 */
task_t take_task()
{
	task_t t={0,0};
//	LOG(L_CRIT,"-1-\n");
	lock_get(tasks->lock);
//	LOG(L_CRIT,"-2-\n");
	while(tasks->start == tasks->end){
//		LOG(L_CRIT,"-3-\n");
		lock_release(tasks->lock);
//		LOG(L_CRIT,"-4-\n");
		if (*shutdownx) {
			cdp_lock_release(tasks->empty);
			return t;
		}
//		LOG(L_ERR,"-");
		cdp_lock_get(tasks->empty);
//		LOG(L_CRIT,"-5-\n");
		
		lock_get(tasks->lock);
//		LOG(L_CRIT,"-6-\n");
	}
//	LOG(L_CRIT,"-7-\n");
	
	t = tasks->queue[tasks->start];
	tasks->queue[tasks->start].msg = 0;
	tasks->start = (tasks->start+1) % tasks->max;
	cdp_lock_release(tasks->full);
	lock_release(tasks->lock);
	return t;
}

/**
 * Poisons the worker queue.
 * Actually it just releases the task queue locks so that the workers get to evaluate
 * if a shutdown is in process and exit.
 */
void worker_poison_queue()
{
//	int i;
//	for(i=0;i<config->workers;i++)
	cdp_lock_release(tasks->empty);
}

/**
 * This is the main worker process.
 * Takes tasks from the queue in a loop and processes them by calling the registered callbacks.
 * @param id - id of the worker
 * @returns never, exits on shutdown.
 */
void worker_process(int id)
{
	task_t t;
	cdp_cb_t *cb;
	int r;
	LOG(L_INFO,"INFO:[%d] Worker process started...\n",id);	
	/* init the application level for this child */
	while(1){
		if (shutdownx&&(*shutdownx)) break;
		t = take_task();
		if (!t.msg) {
			if (shutdownx&&(*shutdownx)) break;
			LOG(L_INFO,"INFO:worker_process(): [%d] got empty task Q(%d/%d)\n",id,tasks->start,tasks->end);
			continue;
		}		
		LOG(L_DBG,"DBG:worker_process(): [%d] got task Q(%d/%d)\n",id,tasks->start,tasks->end);
		r = is_req(t.msg);
		for(cb = callbacks->head;cb;cb = cb->next)
			(*(cb->cb))(t.p,t.msg,*(cb->ptr));
		
		if (r){
			AAAFreeMessage(&(t.msg));
		}else{
			/* will be freed by the user in upper api */
			/*AAAFreeMessage(&(t.msg));*/
		}
	}
	worker_poison_queue();
	LOG(L_INFO,"INFO:[%d]... Worker process finished\n",id);	
#ifdef CDP_FOR_SER
#else
	#ifdef PKG_MALLOC
		LOG(memlog, "Worker[%d] Memory status (pkg):\n",id);
		//pkg_status();
		#ifdef pkg_sums
			pkg_sums();
		#endif 
	#endif
	dp_del_pid(getpid());	
#endif
	exit(0);
}

