#!/bin/bash
# USAGE "./rbd-loc <pool> <image>"
# 
# # ./rbd-loc rbd testimg1
# osdmap e14 pool 'rbd' (2) object 'rb.0.1027.74b0dc51.000000000000' -> pg 2.e20ccde3 (2.23) -> up [0,1] acting [0,1]
# osdmap e14 pool 'rbd' (2) object 'rb.0.1027.74b0dc51.000000000001' -> pg 2.c612b403 (2.3) -> up [0,2] acting [0,2]
# ...
# osdmap e14 pool 'rbd' (2) object 'rb.0.1027.74b0dc51.000000000087' -> pg 2.74faad96 (2.16) -> up [0,1] acting [0,1]
 
if [ -z ${1} ] || [ -z ${2} ];
then
  echo "USAGE: ./rbd-loc <pool> <image>"
  exit 1
fi
 
rbd_prefix=$(rbd -p ${1} info ${2} | grep block_name_prefix | awk '{print $2}')
for i in $(rados -p ${1} ls | grep ${rbd_prefix})
do
  ceph osd map ${1} ${i}
done
