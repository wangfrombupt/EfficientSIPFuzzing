/*
 * $Id: ifc_datastruct.h 516 2008-02-01 19:33:35Z albertoberlios $
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
 * Serving-CSCF - User Profile Data Structures
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 */

#ifndef S_CSCF_IFC_DATASTRUCT_H_
#define S_CSCF_IFC_DATASTRUCT_H_

#include "registrar.h"
#include "../../sr_module.h"
#include "../../locking.h"

/* IMS Subscription structures */

/** SPT for checking a SIP Header */
typedef struct _ims_sip_header
{
	str header;				/**< name of the header to match	*/
	str content;			/**< regex to match             	*/
	short type;				/**< if known header, precalculated	*/
} ims_sip_header;


/** SPT for checking a SDP line */
typedef struct _ims_session_desc
{
	str line;				/**< name of line from description */
	str content;			/**< regex to match                */
} ims_session_desc;

/** unknown SPT type */
#define IFC_UNKNOWN -1
/** SPT for checking the Request-URI */
#define IFC_REQUEST_URI 1	
/** SPT for checking the Method */
#define IFC_METHOD 2
/** SPT for checking a SIP Header */
#define IFC_SIP_HEADER 3
/** SPT for checking the Session Case */
#define IFC_SESSION_CASE 4
/** SPT for checking a SDP line */
#define IFC_SESSION_DESC 5

/** Session case originating */
#define IFC_ORIGINATING_SESSION 0
/** Session case terminating */
#define IFC_TERMINATING_SESSION 1
/** Session case terminating to unregistered user*/
#define IFC_TERMINATING_UNREGISTERED 2

/** Initial Registration */
#define IFC_INITIAL_REGISTRATION 	1
/** Re-Registration */
#define IFC_RE_REGISTRATION 		1<<1
/** De-Registration */
#define IFC_DE_REGISTRATION 		1<<2

/** Service Point Trigger Structure */
typedef struct _ims_spt
{
	char condition_negated;				/**< if to negate entire condition	*/
	int group;			 			  	/**< group to which it belongs		*/
	char type;							/**< type of condition				*/
	union
	{
		str request_uri;				/**< Request URI regex				*/
		str method;						/**< the SIP method should be this	*/
		ims_sip_header sip_header;		/**< match of a certain SIP header	*/
		char session_case;				/**< session direction and case		*/
		ims_session_desc session_desc;	/**< session description match 		*/
	};									/**< union for SPT 					*/
	char registration_type;				/**< set of registration types		*/
} ims_spt;

/** Conjunctive Normal Format */
#define IFC_CNF 1
/** Disjunctive Normal Format */
#define IFC_DNF 0

/** Trigger Point Structure */
typedef struct _ims_trigger_point
{
	char condition_type_cnf;	/**< if it's CNF or DNF     		*/
	ims_spt *spt;			/**< service point triggers 1..n 		*/
	unsigned short spt_cnt;			/**< number of service point triggers 	*/
} ims_trigger_point;

/** No default handling */
#define IFC_NO_DEFAULT_HANDLING -1
/** Session should continue on failure to contact AS */
#define IFC_SESSION_CONTINUED 0
/** Session should be terminated on failure to contact AS */
#define IFC_SESSION_TERMINATED 1

/** Application Server Structure */
typedef struct _ims_application_server
{
	str server_name;			/**< SIP URL of the app server                      */
	char default_handling;		/**< enum SESSION_CONTINUED SESSION_TERMINATED 0..1 */
	str service_info;			/**< optional info to be sent to AS 0..1            */
} ims_application_server;

/** Public Identity Structure */
typedef struct {
	char barring;				/**< Barring state									*/
	str public_identity;		/**< Public Identity string							*/
	str wildcarded_psi;		/** if exists is the wildcarded psi					*/
} ims_public_identity;

/** Initial Filter Criteria Structure */
typedef struct _ims_filter_criteria
{
	int priority;								/**< checking priority, lower means more important */
	ims_trigger_point *trigger_point;			/**< definition of trigger 0..1 */
	ims_application_server application_server;	/**< target of the trigger   */
	char *profile_part_indicator;				/**< profile part indicator 0..1 */	
} ims_filter_criteria;

/** CoreNetwork Service Authorization */
typedef struct _ims_cn_service_auth
{
	int subscribed_media_profile_id;	/* must be >=0 */
} ims_cn_service_auth;

/** Service Profile Structure */
typedef struct {
	ims_public_identity *public_identities;	/**< array of public identities		*/
	unsigned short public_identities_cnt;				/**< number of public identities	*/

	ims_filter_criteria *filter_criteria;	/**< vector of filter criteria 0..n */
	unsigned short filter_criteria_cnt;				/**< size of the vector above		*/

	ims_cn_service_auth *cn_service_auth;	/**< core net. services auth. 0..1	*/

	int *shared_ifc_set;					/**< shared ifc set ids 0..n 		*/
	unsigned short shared_ifc_set_cnt;					/**< size of above vector 			*/	
} ims_service_profile;

/** User Subscription Structure */ 
typedef struct {	
	str private_identity;					/**< private identity 				*/
	int wpsi;								/**
												This is not in the standards
												0 normal user or distinct psi inside
												1 wildcarded psi 
											**/
	ims_service_profile *service_profiles;	/**< array of service profiles		*/
	unsigned short service_profiles_cnt;				/**< size of the array above		*/
		
	int ref_count;							/**< referenced count 				*/
	gen_lock_t *lock;						/**< lock for operations on it 		*/
} ims_subscription;

#endif //S_CSCF_IFC_DATASTRUCT_H_
