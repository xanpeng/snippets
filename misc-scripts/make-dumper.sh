#!/bin/bash
 
#
# Makefile hacks learn from http://blog.melski.net/2010/11/30/makefile-hacks-print-the-value-of-any-variable/
#
# Function: print make environment variables
# Usage: sh make-dumper.sh var0 [var1]
# Example: sh make-dumper.sh MAKE_VERSION prefix libdir
#
 
#
# TODO
# Method 1: scp the complied project direcotry to a remote machine, `make install` there.
# `make install` will fail. Why?
# Answer: Method 2 is better.
#
# Method 2: `make install` into a specified dir(e.x. runtime), scp the executables and libs to a remote machine.
# `make prefix=/path/to/specified install` will fail. Why?
# Answer: use `make install DESTDIR=*`, ref: http://stackoverflow.com/questions/11307465/destdir-and-prefix-of-make
# NOTE: use rsync instead of scp to persist symbolic links, `rsync --progress -avhe ssh . 10.100.203.228:/usr/local/`.
 
filename=""
if [ -f GNUmakefile ] ; then
  filename="GNUmakefile"
elif [ -f makefile ] ; then
  filename="makefile"
elif [ -f Makefile ] ; then
  filename="Makefile"
fi
 
if [ -n "$filename" ] ; then
  vars=""
  for n in $@ ; do
    vars="$vars print-$n"
  done
  #
  # cat printvar.mak
  # print-%:
  # @echo '$*=$($*)'
  #
  make -f $filename -f printvar.mak $vars
else
  echo "No makefile found" 1>&2
  exit 1
fi
