/**
 * $Id: mod.h 2 2006-11-14 22:37:20Z vingarzan $
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
 * \dir modules/cdp
 *
 * This is the CDiameterPeer module. For general documentation, look at \ref CDP
 * 
 */
 
 /**
  *  \file modules/cdp/Makefile 
  * CDiameterPeer SER module Makefile
  * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
  */
 
/** \page CDP The CDiameterPeer Module (cdp)
 *  \b Module \b Documentation
 * 
 * [\subpage cdp_overview]
 * [\subpage cdp_code]
 * [\subpage cdp_config]
 * [\subpage cdp_example]
 *
 * \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 * \todo Realm routing (because this is not yet supported, when using you will have to 
 * specify each time the FQDN of the destination host. This is usualy a configuration
 * parameter other the modules using this one).
 * 
 * \todo Session support (although not required on the IMS Cx interfaces).
 * 
 * 
 * \section cdp_overview Overview
 * This module is supposed to allow efficient Diameter communication to and from
 * SER. Some parts of the diameter codec are inherited from DISC 
 * http://developer.berlios.de/projects/disc/ . 
 * 
 * 
 * A series of processes are forked:
 * - 1 x Timer - timer.c
 * - 1 x Acceptor - acceptor.c
 * - k x Worker - worker.c (k configurable)   
 * - i x Receiver - receiver.c - one for each incoming or outgoing peer
 * 
 * \section cdp_code Code Structure
 * The SER module exported API can be seen in mod.c. The full Diameter API is in diameter.h
 * and diameter_api.h. IMS specific constants and assigned numbers are defined in 
 * diameter_ims.h. 
 * 
 * The protocol codec is implemented into diameter_avp.c and diameter_msg.c. 
 * The Diameter base protocol state machine is implemented in peerstatemachine.c. 
 * peermanager.c takes care of peer administration. Basic transactions are defined in 
 * transaction.c.   
 * 
 * \section cdp_config Configuration and Usage
 * For exported functions look at #cdp_cmds.\n
 * For configuration parameters look at #cdp_params.\n 
 * 
 * For input configuration file, take a look at configdtd.h for the structure. Here we have
 * an example:
 * \code
<?xml version="1.0" encoding="UTF-8"?>
<DiameterPeer 
	FQDN="xcscf.open-ims.test"
	Realm="open-ims.test"
	Vendor_Id="10415"
	Product_Name="CDiameterPeer"
	AcceptUnknownPeers="0"
	DropUnknownOnDisconnect="1"
	Tc="30"
	Workers="4"
	QueueLength="32"
>
	<Peer FQDN="hss.open-ims.test" Realm="open-ims.test" port="3868"/>

	<Acceptor port="3868"  />
	<Acceptor port="3869" bind="127.0.0.1" />
	<Acceptor port="3870" bind="192.168.1.1" />
	
	<Acct id="16777216" vendor="10415" />
	<Acct id="16777216" vendor="0" />
	<Auth id="16777216" vendor="10415"/>
	<Auth id="16777216" vendor="0" />

</DiameterPeer>
 * \endcode
 * 
 * It is used by :
 * - \ref ICSCF ;
 * - \ref SCSCF ;
 *
 * \section cdp_example Examples
 * 
 * And here are some real usage examples:
 * - Interrogating-CSCF CDiameterPeer configuration file \include icscf.xml
 * - Serving-CSCF CDiameterPeer configuration file \include scscf.xml
 * 
 */

/**
 * \file
 * 
 * CDiameterPeer SER module interface and declarations.
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 */
 
 
#ifndef _CDP__H
#define _CDP__H

#include "../../sr_module.h"

#define M_NAME "cdp"


static int cdp_init( void );
static int cdp_child_init( int rank );
static int cdp_exit( void );


#endif
