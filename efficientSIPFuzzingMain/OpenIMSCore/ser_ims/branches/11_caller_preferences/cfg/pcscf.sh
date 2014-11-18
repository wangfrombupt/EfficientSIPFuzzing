#!/bin/bash

./ser_ims/cfg/killser pcscf
setkey -F
setkey -FP

LD_LIBRARY_PATH="/usr/local/lib/ser" /opt/OpenIMSCore/ser_ims/ser -f /opt/OpenIMSCore/pcscf.cfg -D -D

