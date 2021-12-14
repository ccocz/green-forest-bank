#!/bin/bash

iptables -F DOCKER-USER

iptables -A DOCKER-USER -p icmp -j ACCEPT
iptables -A DOCKER-USER -p tcp --dport 22 -j ACCEPT
iptables -A DOCKER-USER -p tcp --dport 443 -j ACCEPT
iptables -A DOCKER-USER -p tcp --dport 80 -j ACCEPT #optional
iptables -A DOCKER -j DROP
