# syntax=docker/dockerfile:1

FROM ubuntu:18.04
ADD . /app
RUN chmod +x /app/init.sh
RUN /app/init.sh /app/uzytkownicy.txt
