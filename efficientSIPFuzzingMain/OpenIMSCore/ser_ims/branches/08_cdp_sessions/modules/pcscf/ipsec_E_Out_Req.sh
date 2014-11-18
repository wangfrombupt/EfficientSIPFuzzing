#!/bin/bash
#
# UserEndpoint SA for Outgoing Requests ( UC -> PS )
#
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#

ue=$1
port_uc=$2
pcscf=$3
port_ps=$4

spi_ps=$5

ealg=$6
ck=$7
alg=$8
ik=$9

if [ "$6" = "null" ] 
then
	ck=""
fi

setkey -c << EOF
spdadd $ue/32[$port_uc] $pcscf/32[$port_ps] tcp -P out ipsec esp/transport//unique:1 ;
spdadd $ue/32[$port_uc] $pcscf/32[$port_ps] udp -P out ipsec esp/transport//unique:1 ;
add $ue $pcscf esp $spi_ps -m transport -u 1 -E $ealg $ck -A  $alg $ik ;
EOF

