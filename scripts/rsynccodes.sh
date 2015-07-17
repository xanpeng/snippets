#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage $0 TARGET_HOST"
  exit 1
fi

MEBS_LOCATION="/Users/xan/workspace/mebs/" # trailing '/' needed
REMOTE_LOCATION="root@$1:~/mebs"
EXCLUDE_PATTERN=".git"

# -r: recursive for diretories
# -a: archive,
#   recursively,
#   preserves symbolic links,
#   preserves special and device files,
#   preserves modification times
#   preserves group, owner and permissions
# -v: verbose
# -z: compress transferring files
# -P: --progress + --partial (resume interrupted transfers)
# --delete: remove file in DEST which is not in SRC
rsync -azP --exclude=$EXCLUDE_PATTERN $MEBS_LOCATION $REMOTE_LOCATION # push mode 
