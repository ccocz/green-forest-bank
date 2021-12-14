# Green Forest Bank

## Dockerfile

Dockerfile creates the initial structure of users, directories and rights to them using wrapper script ```init.sh``` 
(see below), and an instance of both application modules (see ```www-server``` and ```officer-app``` below).

## officer-app
Module contains C source code and ```CMakeLists.txt``` to build the app.

## www-server
Contains following configurations and wrapper script to initiate WWW server:
### init.sh
Script to run all ```apache2``` related settings to generate ```x.509``` certificate enable https connection.
Additionally, sets up environment for clients needed to log in using ```PAM```. 
### cert.cnf
This file contains prompts needed to generate root certificate.

### 000-default.conf
Default config for ```http``` connections. (This is optional - not required by the task)
### default-ssl.conf
Default config for ```https``` connection.

## firewall.sh
Since Docker manipulates host's iptables to maintain isolation (https://docs.docker.com/network/iptables/)
it allows adding rules to ```DOCKER-USER``` chain. This script adds rules to filter out packages as mentioned
in the problem statement. Needs to be run on the host machine and requires iptables to be installed.

## init.sh
Script creates users as per given argument file and builds file structure and permissions accordingly.
This is a wrapper script running as a last command in the Dockerfile, therefore when running the container
it might take few seconds before being able to connect via SSH or web server.

## Examples

### Officer trying to connect to the system via SSH
````
docker build .
docker run -d -p 1235:22 <container_id>
````
````
ssh -p 1235 officer@localhost # might be refused for few seconds after running.
````

User lands on the ```officer_app``` and after providing credentials (```date +%s``` for getting epoch seconds) can choose options from the menu.

For instance, choosing display option lists all periods in reverse-chronological order. Periods are separated by an empty line
and each period is either sum, new percentage or ending positions. If sum position was followed by another
sum position the ending dates will be shown as a part of that period otherwise starting dates also correspond
to ending dates of previous period.

### Client connecting to web server
````
docker build .
docker run -d -p 1236:443 <container_id>
````

Then, in the browser following URL https://localhost:1236 launches the app.
After providing login credentials can skim through credits and deposits directories.
However, can only see the content of the files which he/she owns. 

## Note
All users have password equal to their user IDs.