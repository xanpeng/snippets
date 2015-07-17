#!/bin/bash
 
netcard="eth0"
netcard_config="/etc/sysconfig/network/ifcfg-$netcard"
 
config_static="BOOTPROTO='static'\n
BROADCAST=''\n
ETHTOOL_OPTIONS=''\n
IPADDR=''\n
MTU=''\n
NAME='Intel Ethernet controller'\n
NETMASK=''\n
NETWORK=''\n
REMOTE_IPADDR=''\n
STARTMODE='nfsroot'\n
USERCONTROL='no'"
 
config_dhcp="BOOTPROTO='dhcp'\n
BROADCAST=''\n
ETHTOOL_OPTIONS=''\n
IPADDR=''\n
MTU=''\n
NAME='Intel Ethernet controller'\n
NETMASK=''\n
NETWORK=''\n
REMOTE_IPADDR=''\n
STARTMODE='nfsroot'\n
USERCONTROL='no'"
 
set -x
 
echo -e $config_dhcp > $netcard_config && ifdown $netcard && ifup $netcard
 
brctl addbr br0
brctl addif br0 eth0
 
# http://stackoverflow.com/questions/2361709/efficient-way-to-get-your-ip-address-in-shell-scripts
ip=`ifconfig eth0 | awk -F':' '/inet addr/&&!/127.0.0.1/{split($2,_," ");print _[1]}'`
 
ifconfig eth0 0.0.0.0 up
ifconfig br0 $ip netmask 255.255.255.0 up
 
route add default gw 10.100.203.2
 
echo -e $config_static > $netcard_config && ifdown $netcard && ifup $netcard
 
set +x
