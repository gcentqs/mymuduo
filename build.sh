#!/bin/bash

set -e

# 如果没有build目录 创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -fr `pwd`/build/*
cd `pwd`/build &&
    cmake .. &&
    make -j16

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/mymuduo       .so库拷贝到 /usr/lib
if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo
done

if [ ! -d /usr/include/mymuduo/utils ]; then
    mkdir /usr/include/mymuduo/utils
fi

for header in `ls ./utils/*.h`
do
    cp $header /usr/include/mymuduo/utils
done


cp `pwd`/lib/libmymuduo.so /usr/lib
cp `pwd`/lib/libutils.so /usr/lib

ldconfig
