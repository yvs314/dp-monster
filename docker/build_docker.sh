#!/usr/bin/env bash
CNAME=dpm

docker build -t $CNAME .
if [ ! "$(hostname)" == "umt.imm.uran.ru" ]
then
  docker push --tls-verify=false localhost/$CNAME 172.16.0.1:5000/$CNAME
fi
