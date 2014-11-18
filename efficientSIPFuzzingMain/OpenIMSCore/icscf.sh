#!/bin/bash

./ser_ims/cfg/killser icscf
/opt/OpenIMSCore/ser_ims/ser -f /opt/OpenIMSCore/icscf.cfg -D -D

ipcs -s
