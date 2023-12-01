#!/bin/bash
set -e

if [ ! -d `pwd`/build ]; then
  mkdir `pwd`/"build"
fi

rm -rf `pwd`/build/*

if [ -d usr/include/netpoll ]; then
  rm -rf usr/include/netpoll
fi

if [ -f usr/lib/libnetpoll.so ]; then
  rm usr/lib/libnetpoll.so
fi

cd build &&
   cmake .. &&
   make

cd ..


if [ ! -d /usr/include/netpoll ]; then
  mkdir /usr/include/netpoll
fi



for header in `ls ./netpoll/*.h`
do
  cp $header /usr/include/netpoll
done


cp `pwd`/lib/libnetpoll.so /usr/lib

ldconfig