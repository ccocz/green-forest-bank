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
    libapache2-mod-authnz-pam

ADD . /app

RUN chmod +x /app/www-server/init.sh && /app/www-server/init.sh
RUN cmake /app/officer_app/CMakeLists.txt && make -C /app/officer_app

CMD chmod +x /app/init.sh && /app/init.sh app/uzytkownicy.txt

#is this needed?
EXPOSE 80

EXPOSE 443