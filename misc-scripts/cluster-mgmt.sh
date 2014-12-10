#!/bin/sh

# comparison operators
#    http://tldp.org/LDP/abs/html/comparison-ops.html
# Special Characters
#    http://tldp.org/LDP/abs/html/special-chars.html

# ensure start with specified thing
#    http://stackoverflow.com/questions/2172352/in-bash-how-can-i-check-if-a-string-begins-with-some-value

#
# variables
#
N1=xxx.xxx.xxxx.xxx
N2=xxx.xxx.xxxx.xxx
N3=xxx.xxx.xxxx.xxx
N4=xxx.xxx.xxxx.xxx
NODES=($N1 $N2 $N3 $N4)

SSH="ssh -q -n"

#
# functions
#

validate_format()
{
  # real OP expect
  # if $1 $2 $3; then echo "INPUT ERROR!!!"; fi
  echo $1 $2 $3
  [ $1 $2 $3 ] && echo "INPUT ERROR!!!"
}

echo_and_exit()
{
  echo $1; exit 1
}

work_looply_sync()
{
  for node in ${NODES[@]}
  do
    echo "$@ @$node"
    $SSH root@$node "$@"
  done
}

# it's actually no need to implement EX in single node, as we can use SSH COMMAND directly.
workat_single_node()
{
  node=$1; shift
  $SSH root@$node "$@"
}

# it's actually no need to implement CP in single node, as we can use SCP COMMAND directly.
copyto_single_node()
{
  echo "--- scp '$2' to $1 ---"
  scp -q $2 root@$1:$2
}

scp_looply()
{
  for node in ${NODES[@]}
  do
    echo "--- scp '$1' to $node ---"
    scp -q $1 root@$node:$1
  done
}


#
# main
#
case "$1" in
  cex)
    if validate_format $# -lt 2; then echo_and_exit "Usage: cm cex COMMAND"; fi
    shift; work_looply_sync $@
    ;;

    # it's actually no need to implement EX in single node, as we can use SSH COMMAND directly.
    ex)
    if validate_format $# -lt 3; then echo_and_exit "Usage: cm ex NODE COMMAND"; fi
    shift; workat_single_node $@
    ;;
  ccp)
    if validate_format $# -lt 2; then echo_and_exit "Usage: cm ccp /PATH"; fi
    # THIS DONT WORK
    # if validate_format $2 -ne /*; then echo_and_exit "Usage: cm ccp /PATH ->Must be absolute path"; fi
    if [[ $2 != /* ]]; then echo_and_exit "Usage: cm ccp /PATH ->Must be absolute path"; fi
    shift; scp_looply $@
    ;;

    # it's actually no need to implement CP in single node, as we can use SCP COMMAND directly.
    cp)
    if validate_format $# -lt 3; then echo_and_exit "Usage: cm cp NODE /PATH"; fi
    if validate_format $3 -ne /*; then echo_and_exit "Usage: cm cp NODE /PATH ->Must be absolute path"; fi
    shift; copyto_single_node $@
    ;;
  *)
    echo "Usage: $0 scp FILE_OR_DIR"
    ;;
esac

exit 0
