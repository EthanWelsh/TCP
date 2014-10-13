#!/bin/sh
#
# module name + args
#

##############
# TEMP
echo "run_module.sh: Calling \"$*\" (using display: $MINET_DISPLAY)"
##############

case $MINET_DISPLAY in
    none)
	$* 1> /dev/null 2> /dev/null &
	echo $! >> pids
    ;;
    log)
	$* 1> $1.stdout.log 2> $1.stderr.log &
	echo $! >> pids
    ;;
    gdb)
	xterm -fg cyan -bg black -sb -sl 5000 -T gdb-$1 -e gdb $1 &
	echo $! >> pids
    ;;
    xterm_pause)
        xterm -fg cyan -bg black -sb -sl 5000 -T $1 -e xterm_pause.sh $* &
        echo $! >> pids
    ;;
    xterm | *)
	xterm -fg cyan -bg black -sb -sl 5000 -T $1 -e $* &
	echo $! >> pids
    ;;
esac

