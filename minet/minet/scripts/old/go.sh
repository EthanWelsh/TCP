#!/bin/sh

rm -f pids


export MINET_DISPLAY=xterm

case "foo$MINET_MONITORTYPE" in
   foo) run_module.sh monitor;;
   footext) run_module.sh monitor;;
   foojavagui) java -jar mmonitor.jar & echo $! >> pids;
esac


monitor.sh
device_driver.sh 
ethernet_mux.sh 
arp_module.sh 
ip_module.sh 
other_module.sh 
ip_mux.sh 
icmp_module.sh
udp_module.sh 
tcp_module.sh 
ipother_module.sh
sock_module.sh 
case "foo$*" in
  foo) export MINET_DISPLAY=xterm; run_module.sh app;;
  foo?*) export MINET_DISPLAY=xterm; run_module.sh $* ;;
esac




