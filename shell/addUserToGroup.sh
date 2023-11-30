#!/bin/tcsh
# version 1.0 1.22.22
#set echo
if ($#argv < 2) then
   echo "Usage: addUserToGroup user group"
	exit 1
endif
# user  $1 $2 $3
set user = $1
set group = $2

#create file with this info..

set aclfile = "/Users/scott/MQTT/passwords/aclfile.example"
#set aclfile = "/home/ec2-user/MQTT/passwords/aclfile.example"


date
echo $user
echo $group

echo "" >> $aclfile
echo "#" `date` >> $aclfile
echo "" >> $aclfile
echo "#Group $user" >> $aclfile
echo "user $user" >> $aclfile
echo "topic readwrite usersP/groups/$group" >> $aclfile
# NOTE: no "/" to begin

echo "####" >> $aclfile


sleep 1

#now send -HUP signal to mosquitto which will then reread these 2 files..
pkill -HUP mosquitto


echo "Added user $user to group $group MQTT"
