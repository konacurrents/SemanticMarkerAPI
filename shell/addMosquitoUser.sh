#!/bin/tcsh
# version 1.0 1.22.22
#set echo
if ($#argv < 3) then
   echo "Usage: addMosquitoUser user password guestPassword"
	exit 1
endif
# user  $1 $2 $3
set user = $1
set password = $2
set guestPassword = $3
set guest = $1_guest

#create file with this info..
set userFile = "/Users/scott/MQTT/passwords/userList"
echo " " >> $userFile
echo { \"user\": \"$user\", \"password\": \"$password\", \"guest\": \"$guest\", \"guestPassword\": \"$guestPassword\", \"date\": \"`date`\" }, >> $userFile


set aclfile = "/Users/scott/MQTT/passwords/aclfile.example"
set passwordFile = "/Users/scott/MQTT/passwords/passwords"

date
echo $user
echo $password
echo $guest
echo $guestPassword

mosquitto_passwd -b $passwordFile $user $password
mosquitto_passwd -b $passwordFile $guest $guestPassword

echo "" >> $aclfile
echo "#" `date` >> $aclfile
echo "" >> $aclfile
echo "#User $user" >> $aclfile
echo "user $user" >> $aclfile
echo "topic $user/bark " >> $aclfile
echo "topic $user/images " >> $aclfile
echo "topic idogwatch/info  " >> /$aclfile

echo "" >> $aclfile
echo "#User $guest" >> $aclfile
echo "user $guest" >> $aclfile
echo "topic $user/bark " >> $aclfile
echo "topic $user/images " >> $aclfile
echo "topic idogwatch/info  " >> $aclfile

echo "####" >> $aclfile


sleep 1

#now send -HUP signal to mosquitto which will then reread these 2 files..
pkill -HUP mosquitto


echo "Added user $user and their guest $guest to MQTT"
