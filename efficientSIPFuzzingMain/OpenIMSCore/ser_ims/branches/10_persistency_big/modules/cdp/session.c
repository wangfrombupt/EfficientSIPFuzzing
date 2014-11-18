/**
 * $Id: session.c 2 2006-11-14 22:37:20Z vingarzan $
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
 * CDiameterPeer Session Handling 
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "session.h"
#include "diameter.h"
#include "config.h"

extern dp_config *config;		/**< Configuration for this diameter peer 	*/

gen_lock_t *session_lock;		/**< lock for session operation */
unsigned int *session_id1;		/**< first part of the session id */
unsigned int *session_id2;		/**< second part of the session id */

/**
 * Initializes the session related structures.
 */
int session_init()
{
	session_lock = lock_alloc();
	if (!session_lock){
		LOG_NO_MEM("lock",sizeof(gen_lock_t));
		goto error;
	}
	session_lock = lock_init(session_lock);
	session_id1 = shm_malloc(sizeof(unsigned int));
	if (!session_id1){
		LOG_NO_MEM("shm",sizeof(unsigned int));
		goto error;
	}
	session_id2 = shm_malloc(sizeof(unsigned int));
	if (!session_id2){
		LOG_NO_MEM("shm",sizeof(unsigned int));
		goto error;
	}
	srand((unsigned int)time(0));	
	*session_id1 = rand();
	*session_id1 <<= 16;
	*session_id1 += time(0)&0xFFFF;
	*session_id2 = 0;
	return 1;
error:
	return 0;
}

/**
 * Destroys the session related structures.
 */
int session_destroy()
{
	lock_get(session_lock);
	lock_destroy(session_lock);
	lock_dealloc((void*)session_lock);	
	shm_free(session_id1);
	shm_free(session_id2);
	return 1;
}


/*
 * Generates a new session_ID (conforming with draft-ietf-aaa-diameter-17).
 * This function is thread safe.
 * @returns an 1 if success or -1 if error.
 */
static int generate_sessionID( str *sID, unsigned int end_pad_len)
{
	unsigned int s2;

	/* some checks */
	if (!sID)
		goto error;

	/* compute id's len */
	sID->len = config->identity.len +
		1/*;*/ + 10/*high 32 bits*/ +
		1/*;*/ + 10/*low 32 bits*/ +
//		1/*;*/ + 8/*optional value*/ +
		1 /* terminating \0 */ +
		end_pad_len;

	/* get some memory for it */
	sID->s = (char*)shm_malloc( sID->len );
	if (sID->s==0) {
		LOG(L_ERR,"ERROR:generate_sessionID: no more free memory!\n");
		goto error;
	}

	lock_get(session_lock);
	s2 = *session_id2 +1;
	*session_id2 = s2;
	lock_release(session_lock);
	
	/* build the sessionID */
	sprintf(sID->s,"%.*s;%u;%u",config->identity.len,config->identity.s,*session_id1,s2);
	sID->len = strlen(sID->s);
	return 1;
error:
	return -1;
}






/****************************** API FUNCTIONS ********************************/

/**
 * Creates a AAASessionId.
 * @returns the new AAASessionId or an empty string on failure
 */
AAASessionId AAACreateSession()
{
	AAASessionId sID={0,0};
	
	/* generates a new session-ID - the extra pad is used to append to 
	 * session-ID the hash-code and label of the session ".XXXXXXXX.XXXXXXXX"*/
	if (generate_sessionID( &(sID), 0 )!=1)
		goto error;

	return sID;
error:
	LOG(L_ERR,"ERROR:AAACreateSession(): Error on new session generation\n");
	return sID;
}

/**
 * Deallocates the memory taken by a AAASessionId
 * @param s - the AAASessionId to be deallocated
 */
int AAADropSession(AAASessionId *s)
{
	if (s->s) shm_free(s->s);
	s->s =0;
	s->len=0;
	return AAA_ERR_SUCCESS;
}
