#!/bin/sh

mkdir container/
mkdir container/bin
cd    container/bin

cp    /bin/busybox .

PRGS=$(./busybox --list)
for P in $PRGS; do
    ln -s busybox $P;
done
