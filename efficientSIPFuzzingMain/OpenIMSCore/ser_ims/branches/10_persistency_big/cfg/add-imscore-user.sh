#!/bin/sh
# 
# $Id: add-imscore-user.sh 186 2007-03-09 12:58:35Z jsbach $
# 
# add-imscore-user.sh
# Version: 0.2
# Released: 02/06/07
# Author: Sven Bornemann -at- materna de
#
# History:
#   0.3 (03/09/07(:
#     * sip2ims transactions are commented out.
#   0.2 (02/06/07): 
#     * Changed parameter handling (getopt).
#     * Allow direct mysql import.
#     * Remove temporary password file after usage.
#   0.1 (02/02/07): 
#     * Initial version
#
# Script for generating two SQL scripts for creating/deleting IMS Core users 
# in the HSS and the SIP2IMS gateway tables.
#
# Usage for add-imscore-user.sh: See Usage() procedure below
#
# Example for creating user 'brooke' with password 'brooke' for realm 
# 'open-ims.test':
#
# # ./add-imscore-user.sh -u brooke -a
# Successfully wrote add-user-brooke.sql
# Successfully wrote delete-user-brooke.sql
# Apply add-user-brooke.sql...
# Enter password:
# Successfully applied add-user-brooke.sql
# 
# After applying the add script, you should be able to register with IMS Core 
# with SIP clients (e.g. as 'brooke') via SIP2IMS. Use delete script or -d 
# option for removing the user from IMS Core database tables.
# 
# Known limits:
# * IMS Core installation in /opt/OpenIMSCore required.
# * Password is limited to 16 characters.
# 


Usage()
{
    echo "ERROR: Invalid parameters"
    echo "add-imscore-user.sh -u <user> [-r <realm> -p <password>] [-a|-d]"
    echo "  -u <user>: The username (e.g. 'brooke')"
    echo "  -r <realm>: The realm. Default is 'open-ims.test'"
    echo "  -p <password>: The password. Default is value of -u option"
    echo "  -a: Automatically apply created add script (script will not be deleted afterwards)"
    echo "  -d: Automatically apply created delete script (script will be deleted afterwards)"
    exit -1
}

OPTION_ADD=0
OPTION_DELETE=0
SCRIPT=
EXIT_CODE=0
DBUSER=root

while getopts u:r:p:o:ad? option;
do
    case $option in
        u) IMSUSER=$OPTARG;;
        r) REALM=$OPTARG;;
        p) PASSWORD=$OPTARG;;
        a) OPTION_ADD=1;;
        d) OPTION_DELETE=1;;
    esac
done

[ -z "$IMSUSER" ] && Usage
[ -z "$PASSWORD" ] && PASSWORD=$IMSUSER
[ -z "$REALM" ] && REALM=open-ims.test

# Some checks
[ $OPTION_ADD -eq 1 ] && [ $OPTION_DELETE -eq 1 ] && Usage;
[ -z "$IMSUSER" ] && Usage;

KEY=`/opt/OpenIMSCore/ser_ims/utils/gen_ha1/gen_ha1 $IMSUSER@$REALM $REALM $PASSWORD`

CREATE_SCRIPT="add-user-$IMSUSER.sql"
DELETE_SCRIPT="delete-user-$IMSUSER.sql"
SED_SCRIPT="s/<USER>/$IMSUSER/g"
PASSWORD_FILE=~temp~password~

echo -n $PASSWORD > $PASSWORD_FILE
ENCODED_PASSWORD=`hexdump -C < $PASSWORD_FILE|cut -b 10-60|sed 's/ //g'|cut -b 1-32`00000000000000000000000000000000
ENCODED_PASSWORD=`echo $ENCODED_PASSWORD|cut -b 1-32`
rm $PASSWORD_FILE

CREATE_SCRIPT_TEMPLATE="insert into hssdb.imsu(name) values ('<USER>_imsu');

--add Private Identity

--Add <USER>@$REALM
insert into hssdb.impi(
        impi_string,
        imsu_id,
        imsi,
        scscf_name,
        s_key,
        chrg_id,
        sqn)
values( '<USER>@$REALM',
        (select imsu_id from hssdb.imsu where hssdb.imsu.name='<USER>_imsu'),
        '<USER>_ISDN_User_part_ID',
        'sip:scscf.$REALM:6060',
        '$ENCODED_PASSWORD',
        (select chrg_id from hssdb.chrginfo where hssdb.chrginfo.name='default_chrg'),
        '000000000000');

--add Public Identity
insert into hssdb.impu(sip_url, tel_url, svp_id) values ('sip:<USER>@$REALM', '', (select svp_id from hssdb.svp where hssdb.svp.name='default_sp'));

--add Public Identity to Private Identity
insert into hssdb.impu2impi(impi_id, impu_id) values ((select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM'), (select impu_id from hssdb.impu where hssdb.impu.sip_url='sip:<USER>@$REALM'));

--add roaming network
insert into hssdb.roam(impi_id, nw_id) values((select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM'), (select nw_id from hssdb.networks where hssdb.networks.network_string='$REALM'));

-- add SIP2IMS credentials
-- insert into sip2ims.credentials values ('<USER>', '_none', '$REALM', '$PASSWORD',1,'','$KEY',(select imsu_id from hssdb.imsu where hssdb.imsu.name='<USER>_imsu'));"


DELETE_SCRIPT_TEMPLATE=" --delete from sip2ims.credentials where auth_username='<USER>';

delete from hssdb.roam where impi_id = (select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM');

delete from hssdb.impu2impi where impi_id = (select impi_id from hssdb.impi where hssdb.impi.impi_string='<USER>@$REALM');

delete from hssdb.impi where imsu_id = (select imsu_id from hssdb.imsu where hssdb.imsu.name='<USER>_imsu');

delete from hssdb.impu where sip_url = 'sip:<USER>@$REALM';

delete from hssdb.imsu where name = '<USER>_imsu';"

# Create SQL add script
echo "$CREATE_SCRIPT_TEMPLATE" | sed $SED_SCRIPT > $CREATE_SCRIPT 
if [ $? -ne 0 ]; then
    echo "Failed to write $CREATE_SCRIPT"
    exit -1
fi
echo "Successfully wrote $CREATE_SCRIPT"

# Create SQL delete script
echo "$DELETE_SCRIPT_TEMPLATE" | sed $SED_SCRIPT > $DELETE_SCRIPT
if [ $? -ne 0 ]; then
    echo "Failed to write $DELETE_SCRIPT"
    exit -1
fi
echo "Successfully wrote $DELETE_SCRIPT"

# Apply scripts directly?
if [ $OPTION_ADD -eq 1 ]; then
    echo Apply $CREATE_SCRIPT as user $DBUSER...
    mysql -u $DBUSER -p < $CREATE_SCRIPT
    EXIT_CODE=$?
    SCRIPT=$CREATE_SCRIPT
elif [ $OPTION_DELETE -eq 1 ]; then
    echo Apply $DELETE_SCRIPT as user $DBUSER...
    mysql -u $DBUSER -p < $DELETE_SCRIPT
    EXIT_CODE=$?
    SCRIPT=$DELETE_SCRIPT
fi

# Evaluate exit code
if [ ! -z "$SCRIPT" ]; then
    if [ $EXIT_CODE -ne 0 ]; then
        echo "ERROR: Failed to apply $SCRIPT"
    else    
        echo "Successfully applied $SCRIPT"
    fi
fi
exit $EXIT_CODE
