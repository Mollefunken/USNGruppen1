#!/bin/bash

sudo PATH=/bin unshare --fork --pid /usr/sbin/chroot container/ /bin/init demon
