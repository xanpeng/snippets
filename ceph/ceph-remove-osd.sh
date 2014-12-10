#!/bin/sh
 
if ["$#" -ne 1 ]; then
  echo "Usage: $0 osd.id" >&2
  exit 1
fi
 
id=$1
set -x
ceph osd crush remove osd.$id
ceph auth del osd.$id
ceph osd down $id
ceph osd rm $id
ceph auth list
ceph osd ls
set +x
