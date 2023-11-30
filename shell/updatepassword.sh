#!/bin/tcsh
# https://www.tutorialspoint.com/unix/unix-special-variables.htm
# version 1.0 1.22.22
#set echo
if ($0 =~ *guest*)  then
  echo "guestPassword"
  set program = "guestpassword"
else 
  echo "userPassword"
  set program = "updatepassword"
endif

# need to strip ./ or other before name..
if ($#argv < 2) then
   if ($program =~ *guest*) then
     echo Usage: $program user guestPassword
	else
     echo Usage: $program user password
	endif
	exit 1
endif
# user  $1 $2 
set user = $1
set password = $2
set guestPassword = $2
#set guest = $1_guest
# This version will have the user == guest (not us creating _guest) version..
set guest = $1

set passwordFile = "/Users/scott/MQTT/passwords/passwords"


if ($program =~ *guest*) then
	 echo "Updating guestPassword: " $guest " to " $guestPassword
    mosquitto_passwd -b $passwordFile $guest $guestPassword
else
	 echo "Updating userPassword: " $user " to " $password
    mosquitto_passwd -b $passwordFile $user $password
endif

#sleep 1

#now send -HUP signal to mosquitto which will then reread these 2 files..
pkill -HUP mosquitto


