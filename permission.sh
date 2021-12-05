#!/bin/bash

file="$1"

workers=(worker1, worker2)

for worker in "${workers[@]}"
do
	useradd $worker
done

any_client=""

while IFS= read -r line
do
	id="$(cut -d' ' -f1 <<<"$line")"
	name="$(cut -d' ' -f2 <<<"$line")"
	surname="$(cut -d' ' -f3 <<<"$line")"
	useradd $id

	mkdir $id/kredity
	setfacl -m u:$id:r $id/kredity
	for worker in "${workers[@]}"
	do
		setfacl -m u:$worker:rw $id/kredity
	done
	
	mkdir $id/lokaty
	setfacl -m u:$id:rwx $id/lokaty
	for worker in "${workers[@]}"
	do
		setfacl -m u:$worker:r $id/lokaty
	done
	
	any_client=$id
	
done < "$file"

sudo -u $any_client touch $any_client/lokaty/new
