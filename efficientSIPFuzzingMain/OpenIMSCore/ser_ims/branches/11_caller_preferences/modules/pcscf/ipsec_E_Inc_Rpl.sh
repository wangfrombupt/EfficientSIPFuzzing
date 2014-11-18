#!/bin/bash
#
# UserEndpoint SA for Incoming Replies ( UC <- PS )
#
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#

ue=$1
port_uc=$2
pcscf=$3
port_ps=$4

spi_uc=$5

ealg=$6
ck=$7
alg=$8
ik=$9

if [ "$6" = "null" ] 
then
	ck=""
fi


setkey -c << EOF
spdadd $pcscf/32[$port_ps] $ue/32[$port_uc] tcp -P in ipsec esp/transport//require ;
spdadd $pcscf/32[$port_ps] $ue/32[$port_uc] udp -P in ipsec esp/transport//require ;
add $pcscf $ue esp $spi_uc -m transport -E $ealg $ck -A $alg $ik ;
EOF

