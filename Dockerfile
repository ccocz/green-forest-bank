# syntax=docker/dockerfile:1

FROM ubuntu:18.04

RUN apt-get update -y \
    && apt-get install -y acl \
    && apt-get install -y cmake \
    && apt-get install -y libpam0g-dev \
    && apt-get install -y ssh

ADD . /app

RUN cmake /app/officer_app/CMakeLists.txt && make -C /app/officer_app

CMD chmod +x /app/init.sh && /app/init.sh app/uzytkownicy.txt