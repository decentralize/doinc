#!/bin/bash

if [ ! -d "lib" ]; then
  mkdir lib
fi
cd lib

if [ ! $? ]; then
  exit 1
fi

if [ ! -d "LuaJIT-2.0.3" ]; then
  wget http://luajit.org/download/LuaJIT-2.0.3.tar.gz
  tar -zxf LuaJIT-2.0.3.tar.gz
  pushd LuaJIT-2.0.3
  make
  popd
fi

if [ ! -d "LuaState-master" ]; then
  wget https://github.com/AdUki/LuaState/archive/master.zip
  mv master.zip LuaState-master.zip
  unzip LuaState-master.zip
fi

rm -f LuaJIT-2.0.3.tar.gz LuaState-master.zip


