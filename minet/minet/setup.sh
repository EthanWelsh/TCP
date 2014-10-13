#! /bin/bash

# boilerplate junk
IFS=

# configuration values #

NIC="eth1"
MAC_ADDR=`./scripts/get_addr.pl ${NIC}`

IP_ADDR=`/sbin/ifconfig ${NIC} | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}'`
if [ ! -n $MINET_IPADDR ]; then
	echo "*************************************************"
	echo "Warning: Failed to fetch IP address automatically"
	echo "*************************************************"

	exit
fi

#
# CS 1652 / TELCOM 2310: 
#     Set the default IP Address here to have it remain consistent across setup.sh invocations
IP_ADDR=""


FIFO_DIR="./fifos"

MSS=256
MIP=512
MTU=500


DEBUG_LEVEL=10
DISPLAY=xterm
MONITOR_TYPE=text


MODS="monitor \
device_driver \
ethernet_mux \
arp_module \
ip_module \
other_module \
ip_mux \
icmp_module \
udp_module \
tcp_module \
ipother_module \
sock_module \
socklib_module"

MONITORED_MODS="device_driver \
ethernet_mux \
arp_module \
other_module \
ip_module \
ip_mux \
icmp_module \
udp_module \
tcp_module \
ipother_module \
sock_module \
socklib_module"


##########################################
####   End Configuration Parameters   ####
##########################################

if [ -z $1 ]; then
    CFG_FILE="./minet.cfg"
fi

echo "" > ${CFG_FILE}


write_cfg() {
    echo $* >> ${CFG_FILE}
}




write_cfg MINET_ETHERNETDEVICE=\"${NIC}\"
write_cfg MINET_IPADDR=\"${IP_ADDR}\"
write_cfg MINET_ETHERNETADDR=\"${MAC_ADDR}\"
write_cfg MINET_DEBUGLEVEL=\"${DEBUG_LEVEL}\"
write_cfg MINET_DISPLAY=\"${DISPLAY}\"
write_cfg MINET_MODULES=\"${MODS}\"
write_cfg MINET_MONITOR=\"${MONITORED_MODS}\"
write_cfg MINET_MONITORTYPE=\"${MONITOR_TYPE}\"
write_cfg MINET_MSS=${MSS}
write_cfg MINET_MIP=${MIP}
write_cfg MINET_MTU=${MTU}


echo "Configuration Written to \"${CFG_FILE}\":"
cat ${CFG_FILE}

if [ -e $FIFO_DIR ]; then
    rm -rf $FIFO_DIR
fi

echo "Making fifos for communication between stack components in ./fifos"

mkdir -p $FIFO_DIR
    

mkfifo $FIFO_DIR/ether2mux 
mkfifo $FIFO_DIR/mux2ether
    
mkfifo $FIFO_DIR/mux2arp
mkfifo $FIFO_DIR/arp2mux
    
mkfifo $FIFO_DIR/mux2ip
mkfifo $FIFO_DIR/ip2mux
    
mkfifo $FIFO_DIR/mux2other
mkfifo $FIFO_DIR/other2mux
    
mkfifo $FIFO_DIR/ip2arp
mkfifo $FIFO_DIR/arp2ip
    
mkfifo $FIFO_DIR/ip2ipmux
mkfifo $FIFO_DIR/ipmux2ip
    
mkfifo $FIFO_DIR/udp2ipmux
mkfifo $FIFO_DIR/ipmux2udp

mkfifo $FIFO_DIR/tcp2ipmux
mkfifo $FIFO_DIR/ipmux2tcp

mkfifo $FIFO_DIR/icmp2ipmux
mkfifo $FIFO_DIR/ipmux2icmp

mkfifo $FIFO_DIR/other2ipmux
mkfifo $FIFO_DIR/ipmux2other

mkfifo $FIFO_DIR/udp2sock
mkfifo $FIFO_DIR/sock2udp

mkfifo $FIFO_DIR/tcp2sock
mkfifo $FIFO_DIR/sock2tcp

mkfifo $FIFO_DIR/icmp2sock
mkfifo $FIFO_DIR/sock2icmp

mkfifo $FIFO_DIR/ipother2sock
mkfifo $FIFO_DIR/sock2ipother

mkfifo $FIFO_DIR/app2sock
mkfifo $FIFO_DIR/sock2app

mkfifo $FIFO_DIR/sock2socklib
mkfifo $FIFO_DIR/socklib2sock

mkfifo $FIFO_DIR/reader2mon
mkfifo $FIFO_DIR/writer2mon
mkfifo $FIFO_DIR/ether2mon
mkfifo $FIFO_DIR/ethermux2mon
mkfifo $FIFO_DIR/arp2mon
mkfifo $FIFO_DIR/ip2mon
mkfifo $FIFO_DIR/other2mon
mkfifo $FIFO_DIR/ipmux2mon
mkfifo $FIFO_DIR/udp2mon
mkfifo $FIFO_DIR/tcp2mon
mkfifo $FIFO_DIR/icmp2mon
mkfifo $FIFO_DIR/ipother2mon
mkfifo $FIFO_DIR/sock2mon
mkfifo $FIFO_DIR/socklib2mon
mkfifo $FIFO_DIR/app2mon

echo "Done!"
