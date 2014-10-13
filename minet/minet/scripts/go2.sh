#!/bin/sh

xterm -sl 5000 -T device_driver -e device_driver.sh &
echo $! > pids
xterm -sl 5000 -T ethernet_mux -e ethernet_mux.sh &
echo $! >> pids
xterm -sl 5000 -T arp_module -e arp_module.sh &
echo $! >> pids
xterm -sl 5000 -T ip_module -e ip_module.sh &
echo $! >> pids
xterm -sl 5000 -T other_module -e other_module.sh &
echo $! >> pids
xterm -sl 5000 -T ip_mux -e ip_mux.sh &
echo $! >> pids
xterm -sl 5000 -T udp_module -e udp_module.sh &
echo $! >> pids
xterm -sl 5000 -T tcp_module -e tcp_module.sh &
echo $! >> pids




