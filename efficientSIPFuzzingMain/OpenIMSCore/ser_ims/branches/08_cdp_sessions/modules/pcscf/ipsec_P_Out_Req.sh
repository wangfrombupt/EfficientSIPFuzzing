#!/bin/bash
#
# Proxy-CSCF SA for Outgoing Replies ( US <- PC )
#
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#

ue=$1
port_us=$2
pcscf=$3
port_pc=$4

spi_us=$5

ealg=$6
ck=$7
alg=$8
ik=$9

if [ "$6" = "null" ] 
then
	ck=""
fi

setkey -c << EOF
spdadd $pcscf/32[$port_pc] $ue/32[$port_us] tcp -P out ipsec esp/transport//unique:3 ;
spdadd $pcscf/32[$port_pc] $ue/32[$port_us] udp -P out ipsec esp/transport//unique:3 ;
add $pcscf $ue esp $spi_us -m transport -u 3 -E $ealg $ck -A  $alg $ik ;
EOF
