/**
 * $Id: configdtd.h 353 2007-06-28 14:27:06Z vingarzan $
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
 * CDiameterPeer - DTD definition for checking the configuration file
 * 
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 * 
 * \code  
<?xml version="1.0" encoding="UTF-8"?>
<!ELEMENT DiameterPeer (Peer*, Acceptor*, Auth*, Acct*)>
<!ATTLIST DiameterPeer 
	FQDN		CDATA				#REQUIRED
	Realm		CDATA				#REQUIRED
	Vendor_Id	CDATA				#REQUIRED
	Product_Name CDATA				#REQUIRED
	AcceptUnknownPeers CDATA		#REQUIRED
	DropUnknownOnDisconnect CDATA	#REQUIRED
	Tc			CDATA				#REQUIRED
	Workers		CDATA				#REQUIRED
	QueueLength	CDATA				#REQUIRED
>
<!ELEMENT Peer (#PCDATA)>
<!ATTLIST Peer
	FQDN		CDATA				#REQUIRED
	realm		CDATA				#REQUIRED
	port		CDATA				#REQUIRED
>
<!ELEMENT Acceptor (#PCDATA)>
<!ATTLIST Peer
	port		CDATA				#REQUIRED
	bind		CDATA				#IMPLIED
>
<!ELEMENT Auth (#PCDATA)>
<!ATTLIST Peer
	id			CDATA				#REQUIRED
	vendor		CDATA				#REQUIRED
>
<!ELEMENT Acct (#PCDATA)>
<!ATTLIST Peer
	id			CDATA				#REQUIRED
	vendor		CDATA				#REQUIRED
>
 * \endcode
 */
 
#define DP_CONFIG_DTD "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<!ELEMENT DiameterPeer (Peer*, Acceptor*, Auth*, Acct*)>\
<!ATTLIST DiameterPeer \
	FQDN		CDATA				#REQUIRED\
	Realm		CDATA				#REQUIRED\
	Vendor_Id	CDATA				#REQUIRED\
	Product_Name CDATA				#REQUIRED\
	AcceptUnknownPeers CDATA		#REQUIRED\
	DropUnknownOnDisconnect CDATA	#REQUIRED\
	Tc			CDATA				#REQUIRED\
	Workers		CDATA				#REQUIRED\
	QueueLength	CDATA				#REQUIRED\
>\
<!ELEMENT Peer (#PCDATA)>\
<!ATTLIST Peer\
	FQDN		CDATA				#REQUIRED\
	realm		CDATA				#REQUIRED\
	port		CDATA				#REQUIRED\
>\
<!ELEMENT Acceptor (#PCDATA)>\
<!ATTLIST Peer\
	port		CDATA				#REQUIRED\
	bind		CDATA				#IMPLIED\
>\
<!ELEMENT Auth (#PCDATA)>\
<!ATTLIST Peer\
	id			CDATA				#REQUIRED\
	vendor		CDATA				#REQUIRED\
>\
<!ELEMENT Acct (#PCDATA)>\
<!ATTLIST Peer\
	id			CDATA				#REQUIRED\
	vendor		CDATA				#REQUIRED\
>\
<!ELEMENT Realm (Route*)>\
<!ATTLIST Realm\
	name		CDATA				#REQUIRED\
>\
<!ELEMENT Route (#PCDATA)>\
<!ATTLIST Route\
	FQDN		CDATA				#REQUIRED\
	metric		CDATA				#REQUIRED\
>\
<!ELEMENT DefaultRoute (#PCDATA)>\
<!ATTLIST DefaultRoute\
	FQDN		CDATA				#REQUIRED\
	metric		CDATA				#REQUIRED\
>\
";

