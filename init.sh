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

  echo "$id:$id" | chpasswd

  if [ "$group" == "client" ]; then
      mkdir "$credits/$id" "$deposits/$id"
      setfacl -m "u:$id:rwx" "$credits/$id" "$deposits/$id"
      setfacl -m "g:officers:rwx" "$credits/$id" "$deposits/$id"
      setfacl -m "g:clients:r--" "$credits/$id" "$deposits/$id"
      setfacl -d -m "u:$id:rwx" "$credits/$id" "$deposits/$id"
      setfacl -d -m "g:officers:rwx" "$credits/$id" "$deposits/$id"
      setfacl -d -m "g:clients:r--" "$credits/$id" "$deposits/$id"

  else
      chsh -s /app/officer_app/off_app "$id"
  fi
done <"$file"

rm /var/www/html/index.html
ln -s /home/bank /var/www/html/

#todo check
service ssh start
apache2ctl -D FOREGROUND