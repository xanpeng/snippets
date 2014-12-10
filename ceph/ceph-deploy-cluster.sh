#!/bin/bash
 
# prerequisite:
# 1. you are on admin node
# 2. ceph-deploy is installed in admin node
# 3. do not use ceph-deploy install/purege/puregedata, if you use your customized ceph
 
set -x
# http://ceph.com/docs/master/rados/deployment/
 
ssh mon0 stop ceph-mon-all
ssh osd0 stop ceph-osd-all
ssh osd1 stop ceph-osd-all
 
function clear_node {
  node=$1
  ssh $node rm -rf /var/lib/ceph/*
  ssh $node mkdir -p /var/lib/ceph/mon
  ssh $node mkdir -p /var/lib/ceph/osd
  ssh $node mkdir -p /var/lib/ceph/tmp
  ssh $node mkdir -p /var/lib/ceph/bootstrap-mds
  ssh $node mkdir -p /var/lib/ceph/bootstrap-osd
  ssh $node rm -rf /var/run/ceph/*
  ssh $node rm /etc/ceph/*
  ssh $node rm /var/log/ceph/*
}
ssh mon0 pkill -9 ceph-mon
ssh osd0 pkill -9 ceph-osd
ssh osd1 pkill -9 ceph-osd
clear_node mon0
clear_node osd0
clear_node osd1
rm ./ceph.conf
rm ./ceph.log
 
ceph-deploy forgetkeys
ceph-deploy new mon0
echo "osd_pool_default_erasure_code_directory = /usr/lib/ceph/erasure-code" >> ./ceph.conf
ceph-deploy --overwrite-conf mon create mon0
ceph-deploy gatherkeys mon0
ceph-deploy admin mon0
 
function umount_osd_disks {
  node=$1
  ssh $node umount /dev/vdb1
  ssh $node umount /dev/vdc1
  ssh $node umount /dev/vdd1
}
umount_osd_disks osd0
umount_osd_disks osd1
 
function node_osds_zap {
  node=$1
  ceph-deploy disk zap $node:vdb
  ceph-deploy disk zap $node:vdc
  ceph-deploy disk zap $node:vdd
}
node_osds_zap osd0
node_osds_zap osd1
 
function node_osds_prepare {
  node=$1
  ceph-deploy --overwrite-conf osd prepare $node:vdb:/dev/vde1
  ceph-deploy --overwrite-conf osd prepare $node:vdc:/dev/vde2
  ceph-deploy --overwrite-conf osd prepare $node:vdd:/dev/vde3
}
node_osds_prepare osd0
node_osds_prepare osd1
 
function node_osds_activate {
  node=$1
  ceph-deploy osd activate $node:/dev/vdb1:/dev/vde1
  ceph-deploy osd activate $node:/dev/vdc1:/dev/vde2
  ceph-deploy osd activate $node:/dev/vdd1:/dev/vde3
}
node_osds_activate osd0
node_osds_activate osd1
 
set +x
