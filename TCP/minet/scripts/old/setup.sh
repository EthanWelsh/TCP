#! /bin/bash

# TEMP
# --------------------------------------------------
# Assume that we always run the setup script from the same
# directory where the binary modules can be found.
#
export PATH=`pwd`:$PATH
#
# --------------------------------------------------
#

# TEMP
# --------------------------------------------------
# Get the IP address automatically. This is ugly, but useful because I don't
# have to keep changing it manually every time I want to test it in the VMs.
# 
export MINET_ETHERNETDEVICE="eth0"
export MINET_IPADDR=`ifconfig $MINET_ETHERNETDEVICE | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}'`
if [ ! -n $MINET_IPADDR ]; then
	echo "*************************************************"
	echo "Warning: Failed to fetch IP address automatically"
	echo "*************************************************"
fi
#
# --------------------------------------------------
#

#export PATH=$HOME/minet/minet-development:$PATH

echo "Setting MINET environment vars"
#export MINET_ETHERNETDEVICE="eth0"
#export MINET_IPADDR="10.10.0.254"
export MINET_ETHERNETADDR=`./get_addr.pl $MINET_ETHERNET_DEVICE`
export MINET_DEBUGLEVEL="10"
# log xterm gdb
export MINET_DISPLAY=xterm
export MINET_MODULES="monitor device_driver ethernet_mux arp_module ip_module other_module ip_mux icmp_module udp_module tcp_module ipother_module sock_module socklib_module"
#export MINET_MONITOR="ip_module tcp_module"
export MINET_MONITOR="device_driver ethernet_mux arp_module other_module ip_module ip_mux icmp_module udp_module tcp_module ipother_module sock_module socklib_module"
export MINET_MONITORTYPE=text
export MINET_MSS=256
export MINET_MIP=512
export MINET_MTU=500

case "foo$LD_LIBRARY_PATH" in
  foo) export LD_LIBRARY_PATH=. ;;
  foo?*) export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. ;;
esac

echo "Done! Vars are follows:"

printenv | grep MINET


if [ -e fifos ]; then
    rm -rf fifos
fi

echo "Making fifos for communication between stack components in ./fifos"

mkdir fifos
    
mkfifo fifos/ether2mux 
mkfifo fifos/mux2ether
    
mkfifo fifos/mux2arp
mkfifo fifos/arp2mux
    
mkfifo fifos/mux2ip
mkfifo fifos/ip2mux
    
mkfifo fifos/mux2other
mkfifo fifos/other2mux
    
mkfifo fifos/ip2arp
mkfifo fifos/arp2ip
    
mkfifo fifos/ip2ipmux
mkfifo fifos/ipmux2ip
    
mkfifo fifos/udp2ipmux
mkfifo fifos/ipmux2udp

mkfifo fifos/tcp2ipmux
mkfifo fifos/ipmux2tcp

mkfifo fifos/icmp2ipmux
mkfifo fifos/ipmux2icmp

mkfifo fifos/other2ipmux
mkfifo fifos/ipmux2other

mkfifo fifos/udp2sock
mkfifo fifos/sock2udp

mkfifo fifos/tcp2sock
mkfifo fifos/sock2tcp

mkfifo fifos/icmp2sock
mkfifo fifos/sock2icmp

mkfifo fifos/ipother2sock
mkfifo fifos/sock2ipother

mkfifo fifos/app2sock
mkfifo fifos/sock2app

mkfifo fifos/sock2socklib
mkfifo fifos/socklib2sock

mkfifo fifos/reader2mon
mkfifo fifos/writer2mon
mkfifo fifos/ether2mon
mkfifo fifos/ethermux2mon
mkfifo fifos/arp2mon
mkfifo fifos/ip2mon
mkfifo fifos/other2mon
mkfifo fifos/ipmux2mon
mkfifo fifos/udp2mon
mkfifo fifos/tcp2mon
mkfifo fifos/icmp2mon
mkfifo fifos/ipother2mon
mkfifo fifos/sock2mon
mkfifo fifos/socklib2mon
mkfifo fifos/app2mon

echo "Done!"
