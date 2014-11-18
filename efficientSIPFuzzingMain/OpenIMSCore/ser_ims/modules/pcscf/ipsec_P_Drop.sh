#!/bin/bash
#
# Proxy-CSCF drop all 4 SA
#
# \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
#

ue=$1
port_uc=$2
port_us=$3

pcscf=$4
port_pc=$5
port_ps=$6

spi_uc=$7
spi_us=$8

spi_pc=$9
spi_ps=${10}



setkey -c << EOF
spddelete $ue/32[$port_uc] $pcscf/32[$port_ps] tcp -P in ;
spddelete $ue/32[$port_uc] $pcscf/32[$port_ps] udp -P in ;
delete $ue $pcscf esp $spi_ps ;

spddelete $pcscf/32[$port_ps] $ue/32[$port_uc] tcp -P out ;
spddelete $pcscf/32[$port_ps] $ue/32[$port_uc] udp -P out ;
delete $pcscf $ue esp $spi_uc ;

spddelete $pcscf/32[$port_pc] $ue/32[$port_us] tcp -P out ;
spddelete $pcscf/32[$port_pc] $ue/32[$port_us] udp -P out ;
delete $pcscf $ue esp $spi_us ;

spddelete $ue/32[$port_us] $pcscf/32[$port_pc] tcp -P in ;
spddelete $ue/32[$port_us] $pcscf/32[$port_pc] udp -P in ;
delete $ue $pcscf esp $spi_pc ;

EOF
