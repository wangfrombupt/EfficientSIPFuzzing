#!/bin/bash
#
# UserEndpoint SA for Outgoing Replies ( US -> PC )
#
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#

ue=$1
port_us=$2
pcscf=$3
port_pc=$4

spi_pc=$5

ealg=$6
ck=$7
alg=$8
ik=$9

if [ "$6" = "null" ] 
then
	ck=""
fi

setkey -c << EOF
spdadd $ue/32[$port_us] $pcscf/32[$port_pc] tcp -P out ipsec esp/transport//unique:2 ;
spdadd $ue/32[$port_us] $pcscf/32[$port_pc] udp -P out ipsec esp/transport//unique:2 ;
add $ue $pcscf esp $spi_pc -m transport -u 2 -E $ealg $ck -A  $alg $ik ;
EOF

