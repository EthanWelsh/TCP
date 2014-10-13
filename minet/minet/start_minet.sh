#!/bin/sh

ll_driver=/usr/bin/device_driver2

# boilerplate junk
IFS=

if [ -f pids ]; then
    echo "PID file exists. Is Minet already running?"
    exit
fi

rm -f pids


if [ ! -f minet.cfg ]; then
    echo "Minet configuration file not present. Please run setup.sh"
    exit
fi


IFS="
" # no, you can't actually use "\n" to specify a newline....

for cfg in `cat minet.cfg`; do
    if  [ ! -z ${cfg} ]; then
	eval export ${cfg}
    fi
done
IFS=

if [ -z "${MINET_DISPLAY}" ]; then
    export MINET_DISPLAY=xterm
fi

run_module() {

   case $* in 
 	device_driver) cmd="$ll_driver" ;;
	*)    cmd="bin/"$* ;;
    esac

    case $MINET_DISPLAY in
	none)
           $cmd 1> /dev/null 2> /dev/null &
           echo $! >> pids
        ;;
        log)
           $cmd 1> $1.stdout.log 2> $1.stderr.log &
           echo $! >> pids
        ;;
        gdb)
            xterm -fg cyan -bg black -sb -sl 5000 -T gdb-$1 -e gdb bin/${1} &
            echo $! >> pids
        ;;
        xterm_pause)
            xterm -fg cyan -bg black -sb -sl 5000 -T $1 -e scripts/xterm_pause.sh $cmd &
            echo $! >> pids
        ;;
        xterm | *)
            xterm -fg cyan -bg black -sb -sl 5000 -T $1 -e $cmd &
            echo $! >> pids
        ;;
    esac
}


# start monitor
# Was there any good reason for the "foo" in this statement:
# case "foo$MINET_MONITORTYPE" in
# If so, change it back!
case $MINET_MONITORTYPE in
   text)  run_module monitor;;
   javagui)  java -jar mmonitor.jar & echo $! >> pids;;
   *)  run_module monitor;;
esac

echo "Starting Minet modules..."
# start modules

if [ ! -u $ll_driver -o -z `find $ll_driver -user root` ]; then
    
    echo ""
    echo "Error: Incorrect permissions on $ll_driver. Please make it owned by root and SUID."
    echo ""
    echo "Stopping Minet..."
    ./stop_minet.sh
    exit 
fi


#exit
run_module device_driver

run_module ethernet_mux
run_module "arp_module $MINET_IPADDR $MINET_ETHERNETADDR"
run_module ip_module 
run_module other_module
run_module ip_mux
run_module icmp_module
run_module udp_module 
run_module tcp_module 
run_module ipother_module
run_module sock_module 

case "foo$*" in
  foo)  run_module app;;
  foo?*)  run_module $* ;;
esac






