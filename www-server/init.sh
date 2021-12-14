#!/bin/bash

# all apache2 related config goes here

# first certificate

openssl req -x509 -config /app/www-server/cert.cnf -nodes -days 365 -newkey rsa:2048 -keyout /etc/ssl/private/BSK-CA-2021.key -out /etc/ssl/certs/BSK-CA-2021.crt

cp -f /app/www-server/000-default.conf /etc/apache2/sites-available/
cp -f /app/www-server/default-ssl.conf /etc/apache2/sites-available/

a2enmod ssl
a2enmod headers
a2ensite default-ssl
a2enmod authz_owner
# stop
apache2ctl configtest

# login
echo "auth required pam_unix.so" >> /etc/pam.d/apache
echo "account required pam_unix.so" >> /etc/pam.d/apache

usermod -a -G shadow www-data
chown root:shadow /etc/shadow
chmod g+r /etc/shadow
