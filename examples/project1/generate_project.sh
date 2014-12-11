#!/bin/bash

function warning(){
  echo "$*" >&2
}
function error(){
  echo "$*" >&2
  exit 1
}

read -p "Generate key pair? " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
  openssl genrsa -out priv.pem 2048
  openssl rsa -pubout -in priv.pem -out pub.pem
fi

read -p "Generate server skeleton? " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then
  # Create server.lua with some nice example project
  # Create blueprint.lua
fi


