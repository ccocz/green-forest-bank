# syntax=docker/dockerfile:1

FROM ubuntu:20.04

MAINTAINER resul

ENV TZ=Europe/Warsaw
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update -y && apt-get install -y acl \
    cmake \
    libpam0g-dev \
    ssh \
    apache2 \
    libssl-dev \
    libapache2-mod-authnz-pam \
    && rm -rf /var/lib/apt/lists/*

# todo move up
RUN apt-get update && apt-get install -y libcap2-bin
#RUN apt-get update && apt-get install -y iptables

#RUN iptables -S

ADD . /app

#todo: mkdir here /home/bank

RUN chmod +x /app/www-server/init.sh && /app/www-server/init.sh
RUN cmake /app/officer_app/CMakeLists.txt && make -C /app/officer_app

# search about vulnarabilites
RUN setcap 'cap_chown=eip' /app/officer_app/off_app

# http(optional)
EXPOSE 80
# https
EXPOSE 443
# ssh
EXPOSE 22

CMD chmod +x /app/init.sh && /app/init.sh app/uzytkownicy.txt