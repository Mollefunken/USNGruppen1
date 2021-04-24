#!/bin/bash

#docker kill web; docker rm web;
docker kill web;
docker kill api;
docker rm web;
docker rm api;
docker rmi web:latest;
docker rmi api:latest;

cgcreate -g cpu:cpuContainers
echo 20000 > /sys/fs/cgroup/cpu/cpuContainers/cpu.cfs_quota_us
echo 100000 > /sys/fs/cgroup/cpu/cpuContainers/cpu.cfs_period_us
#gcc --static -o etc/web daniel.c

touch error.log access.log
#docker build -f server.dockerfile -t web .
docker build -f test.dockerfile -t web --target web .
docker build -f test.dockerfile -t api --target api .

#docker build -f test.dockerfile -t mp3 .
rm -f error.log access.log

#docker run -d --cap-drop ALL --cap-add NET_BIND_SERVICE --cap-add SETGID --cap-add SETUID --cap-add SYS_CHROOT --cpu-shares=20 --name "web" -p 8080:80 web:latest
docker run -d --cgroup-parent=/cpuContainers/ --cap-drop=all --cap-add=CHOWN --cap-add=AUDIT_WRITE --cap-add=DAC_OVERRIDE --cap-add=KILL --cap-add=NET_RAW --cap-add=NET_BIND_SERVICE  --name "web" -p 8080:80 web:latest
docker run -d --cgroup-parent=/cpuContainers/ --cap-drop=all --cap-add=CHOWN --cap-add=AUDIT_WRITE --cap-add=DAC_OVERRIDE --cap-add=KILL --cap-add=NET_RAW --cap-add=NET_BIND_SERVICE  --name "api" -p 9090:80 api:latest

#docker run -d --userns=host --name "web" -p 8080:80 web:latest
#docker run -d --userns=host --name "api" -p 8080:80 api:latest
#--cgroup-parent=/cpuContainers/
