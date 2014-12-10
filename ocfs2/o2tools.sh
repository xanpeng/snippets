#!/bin/sh

## a sample bash script to call ocfs2_tools

progname=/sbin/debugo
tmpfile=`mktemp`
fsbase=
mntdev=
filerel=
fileabs=
inode=

usage()
{
  echo "Usage: ./debugvdi.sh TARGET_FILE."
  echo "\tYou DO NOT need to write the absolute path."
}

# check existence of /sbin/debugvdi
stat $progname > /dev/null 2>&1
if [ 0 -ne $? ]; then echo "E: no $progname."; exit 1; fi

# check arguments: ./debugvdi.sh target_file
if [ 1 != $# ]; then echo "E: wrong arguments."; usage; exit 1; fi
filerel=$1

# check if DEV is mounted
#
# mntdev is designated by user.
#
# mount | grep $mntdev > $tmpfile
# if [ 0 -ne $? ]; then echo "E: $mntdev not mounted."; exit 1; fi
#
# mntdev is grepped out.
#
mount | grep -w "ocfs2" > $tmpfile
if [ 0 -ne $? ]; then echo "E: no mounted ocfs2 dev"; exit 1; fi

read line < $tmpfile && > $tmpfile
set -- "$line"
declare -a words=($*)
# set mounted dev
mntdev=${words[0]}
# set fs base directory
fsbase=${words[2]}

# construct absolute file path
fileabs=$fsbase/$filerel

# check file existence
# get inode number of target file
ls -i $fileabs > $tmpfile 2>&1
if [ 0 -ne $? ]; then echo "E: $fileabs didnot exist."; exit 1; fi
read line < $tmpfile && > $tmpfile
set -- "$line"
declare -a words=($*)
inode=${words[0]}

# encode inode
lockres=`$progname -e $mntdev V $inode`

# show specified lockres state
echo "dlm_locks -l $lockres" > $tmpfile
$progname $mntdev -f $tmpfile
> $tmpfile

exit 0
