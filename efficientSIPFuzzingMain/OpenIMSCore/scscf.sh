#!/bin/bash



./ser_ims/cfg/killser scscf
LD_LIBRARY_PATH=/usr/local/lib/ser /opt/OpenIMSCore/ser_ims/ser -f /opt/OpenIMSCore/scscf.cfg -D -D

ipcs -s
