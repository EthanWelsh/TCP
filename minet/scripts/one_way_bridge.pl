#!/usr/bin/perl -w

#
# Bridges local to remote network.  Intended to be 
# run on the local network.  Assumption is that bridge
# is in the locations noted below and must run as root
#

$localbridge="./bridge";
$remotebridge="./bridge";

$sleepsecs=30;

$#ARGV>=3 or die "usage: one_way_bridge.pl remote_account local_device remote_device addresses+\n";

$account=shift;
$localdev=shift;
$remotedev=shift;
foreach $arg (@ARGV) {
  chomp($arg);
  $addrs.=" ".$arg;
}

$cmd="(sleep $sleepsecs; export MINET_ETHERNETADDR=junk; $localbridge $localdev local $addrs) | ssh $account \"(export MINET_ETHERNETADDR=junk; $remotebridge $remotedev remote $addrs >/dev/null )\"";

print "Executing '$cmd'\nPlease type password within $sleepsecs seconds\n";

system $cmd;



