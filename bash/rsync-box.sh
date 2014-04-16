#!/bin/sh

declare -a dirs_to_rsync=("/bin" "/sbin" "/lib" "/lib64" "/usr/bin" "/usr/sbin" "/usr/lib" "/usr/lib64" "/usr/local/bin" "/usr/local/sbin" "/usr/local/lib" "/usr/local/lib64")

if [ "$#" -ne 1 ]; then
    echo "Usage: sh rsync-box.sh REMOTE_HOST"
    exit 1
fi
box=$1

for dir in "${dirs_to_rsync[@]}"
do
    # -v : verbose
    # -r : copies data recursively (but don’t preserve timestamps and permission while transferring dat
    # -a : archive mode, archive mode allows copying files recursively and it also preserves symbolic links, file permissions, user & group ownerships and timestamps
    # -z : compress file data
    # -h : human-readable, output numbers in a human-readable format
    rsync -azrhv $dir/ $box:$dir
done

