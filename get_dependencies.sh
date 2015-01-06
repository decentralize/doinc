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

if [ ! -d "protobuf-2.6.0" ]; then
  wget https://protobuf.googlecode.com/svn/rc/protobuf-2.6.0.tar.gz
  tar -zxf protobuf-2.6.0.tar.gz
  pushd protobuf-2.6.0
  ./configure && make
  popd
fi

if [ ! -d "asd" ]; then
  wget http://www.cryptopp.com/cryptopp562.zip
  unzip cryptopp562.zip -d cryptopp562
  pushd cryptopp562
  popd
fi

rm -f LuaJIT-2.0.3.tar.gz LuaState-master.zip protobuf-2.6.0.tar.gz cryptopp562.zip
