#!/bin/bash

killser icscf
/opt/OpenIMSCore/ser_ims/ser -f /opt/OpenIMSCore/icscf.cfg -D -D

ipcs -s