#!/bin/bash

groupadd officers
groupadd clients

file="$1"

credits="/home/bank/credits"
deposits="/home/bank/deposits"

mkdir -p $credits $deposits

while read -r line;
do
  IFS=' ' read -ra parts <<< "$line"
  id="${parts[0]}"
  group="${parts[1]}"
  f_name="${parts[2]}"
  l_name="${parts[3]}"

  useradd -g "${group}s" -c "$f_name $l_name" "$id" -m

  pswd=$(echo $RANDOM)
  echo "$id:$pswd" | chpasswd
  echo "$id:$pswd" >> "passwords.txt"

  if [ "$group" == "client" ]; then
      mkdir "$credits/$id" "$deposits/$id"
      setfacl -m "u:$id:rwx" "$credits/$id" "$deposits/$id"
      setfacl -d -m "u:$id:rwx" "$credits/$id" "$deposits/$id"
  fi
done <"$file"

# check for other comments
setfacl -d -m g:officers:rwx $deposits $credits
setfacl -d -m g:clients:r-- $deposits $credits

service ssh start

apachectl -D FOREGROUND

#/usr/bin/sshd
#/bin/bash