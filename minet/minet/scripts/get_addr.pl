#!/usr/bin/perl -w

if ($#ARGV<0) {
   $eth="eth0"
} else {
   $eth=$ARGV[0];
}

$s=`/sbin/ifconfig $eth`;
$s=~/.*HWaddr\s+(\S*)/m;
print "$1";
