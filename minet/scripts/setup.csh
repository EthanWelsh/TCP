#! /bin/csh

echo "Setting MINET environment vars"
setenv MINET_IPADDR "10.10.2.0"
setenv MINET_ETHERNETDEVICE "eth0"
setenv MINET_ETHERNETADDR `./get_addr.pl $MINET_ETHERNETDEVICE`
setenv MINET_READER "/home/Minet/execs/reader"
setenv MINET_WRITER "/home/Minet/execs/writer"
setenv MINET_READERBUFFER "100"
setenv MINET_WRITERBUFFER "100"
setenv MINET_DEBUGLEVEL "2"
# log xterm gdb
setenv MINET_DISPLAY xterm
setenv MINET_MODULES "monitor reader writer device_driver ethernet_mux arp_module ip_module other_module ip_mux icmp_module udp_module tcp_module ipother_module sock_module socklib_module"
setenv MINET_MONITOR "ethernet_mux arp_module other_module ip_module ip_mux icmp_module udp_module tcp_module ipother_module sock_module socklib_module"
setenv MINET_MSS 256
setenv MINET_MTU 500

echo "Done!  Vars are as follows:"

printenv | grep MINET


if ( -e fifos ) then
    rm -rf fifos
endif

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

mkfifo fifos/app2sock
mkfifo fifos/sock2app

mkfifo fifos/ipother2sock
mkfifo fifos/sock2ipother

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
