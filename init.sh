#!/bin/bash

groupadd officers
groupadd clients

file="$1"

while read -r line;
do
  IFS=' ' read -ra parts <<< "$line"
  id="${parts[0]}"
  group="${parts[1]}s"
  f_name="${parts[2]}"
  l_name="${parts[3]}"
  useradd -g "$group" -c "$f_name $l_name" "$id"
  pswd=$(echo $RANDOM)
  echo "$id:$pswd" | chpasswd
  echo "$id:$pswd" >> "passwords.txt"

done <"$file"