#!/bin/bash

#docker kill web; docker rm web;
docker kill mp3; 
docker rm mp3;

#gcc --static -o etc/web daniel.c

touch error.log access.log
#docker build -f server.dockerfile -t web .
docker build -f test.dockerfile -t mp3 .
rm -f error.log access.log

#docker run -d --cap-drop ALL --cap-add NET_BIND_SERVICE --cap-add SETGID --cap-add SETUID --cap-add SYS_CHROOT --mount type=volume,src=log,dst=/var/log/ --cpu-shares=20 --name "web" -p 80:80 web
docker run -d  --name "mp3" -p 8080:80 mp3:latest
