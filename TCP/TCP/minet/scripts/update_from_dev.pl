#!/usr/bin/perl -w

use FileHandle;


STDIN->autoflush(1);
STDOUT->autoflush(1);

$cp="scp";
$minet_dev="/home/pdinda/minet-development";
$minet_ho="/home/pdinda/minet-netclass-w02";
$minet_execs="godzilla:/home1/pdinda/netclass-execs";

@srcs=( 
"app.cc",
"arp.cc",
"arp.h",
"arp_module.cc",
"bitsource.cc",
"bitsource.h",
"buffer.cc",
"buffer.h",
"config.cc",
"config.h",
"constate.cc",
"constate.h",
"debug.cc",
"debug.h",
"device_driver.cc",
"error.cc",
"error.h",
"ethernet.cc",
"ethernet.h",
"ethernet_mux.cc",
"header.h",
"headertrailer.cc",
"headertrailer.h",
"http_client.cc",
"http_server1.cc",
"http_server2.cc",
"http_server3.cc",
"icmp_app.cc",
"icmp.cc",
"icmp.h",
"icmp_module.cc",
# "ip.cc",                   - will give out stub
"ip.h",
# "ip_module.cc",            - will give out stub
# "ip_module_diffusion.cc",  - experiemntal
# "ip_module_routing.cc",    - experimental
"ip_mux.cc",
"ipother_module.cc",
"Minet.cc",
"Minet.h",
"minet_socket.cc",
"minet_socket.h",
"monitor.cc",
"Monitor.cc",
"Monitor.h",
"other_module.cc",
"packet_buffer.h",
"packet.cc",
"packet.h",
"packet_queue.cc",
"packet_queue.h",
"raw_ethernet_packet_buffer.cc",
"raw_ethernet_packet_buffer.h",
"raw_ethernet_packet.cc",
"raw_ethernet_packet.h",
"reader.cc",             
# "route.cc",               - experimental
# "route.h",                - experimental
"sock.h",
"sockint.cc",
"sockint.h",
"sock_mod_structs.cc",
"sock_mod_structs.h",
"sock_module.cc",
"sock_test_app.cc",
"sock_test_tcp.cc",
"tcp.cc",
"tcp_client.cc",
"tcp.h",
# "tcp_module.cc",          - will give out a stub
# "tcp_module_stub.cc",     - junk
"tcp_server.cc",
"test_arp.cc",
"test_bitsource.cc",
"test_raw_ethernet_packet_buffer.cc",
"test_reader.cc",
"test_writer.cc",
"udp.cc",
"udp_client.cc",
"udp.h",
"udp_module.cc",
"udp_server.cc",
"util.cc",
"util.h",
"writer.cc" 
);

@scripts=
(
"app.sh",
"arp_module.sh",
"device_driver.sh",
"ethernet_mux.sh",
"fixup.sh",              
"get_addr.pl",
# "go2.sh",                 don't need this
"go.sh",
"icmp_module.sh",
"ip_module.sh",
"ip_mux.sh",
"ipother_module.sh",
#"local.sh",               don't need this
"monitor.sh",
"other_module.sh",
"run_module.sh",
"setup_exp.sh",
#"setup.sh",               will customize
#"setup.csh",              will customize
"sock_module.sh",
"stop.sh",
"tcp_module.sh",
"test_arp.sh",
"udp_module.sh",
"xterm_pause.sh",
"kill_matching.pl",
"sort_monitor_output.pl"
);


@gui=
(
"down-link.gif",
"left-link.gif",
"libjmm.so",
"mmonitor.jar",
"right-link.gif",
"up-link.gif"
);

@execs=
(
"tcp_module",
"ip.o",
"ip_module",
"http_client",
"http_server1",
"http_server2",
"http_server3"
);

@readerwriter=
("reader",
 "writer");

print "This script updates the handout directory $minet_ho\n";
print "and $minet_execs\n";
print "with files from $minet_dev, OVERWRITING FILES.\n";

print "If you want to do this, please enter 'yes' : ";

$prompt=<STDIN>; chomp($prompt);

if (!($prompt eq "yes")) {
  print "HALTED.\n";
  exit 0;
}

print "Would you like to be prompted for each file? [y/n] ";
$prompt=<STDIN>; chomp($prompt);
if (($prompt eq "y" )) {
  $prompt=1;
} else {
  $prompt=0;
}

foreach $file (@srcs, @scripts, @gui) {
  UpdateFile("$minet_dev/$file","$minet_ho/$file",$prompt);
}

print "Do you want to copy over the executables: \n  ".join(" ",@execs)." ? [y/n] ";
$doexecs=<STDIN>; chomp($doexecs);
if ($doexecs eq "y") {
  $cmd="$cp ";
  foreach $file (@execs) {
    $cmd.="$minet_dev/$file ";
  }
  $cmd.="$minet_execs";
#  print "$cmd\n";
  system $cmd;
}

print "Do you want to copy over reader and writer? [y/n] ";
$doreaderwriter=<STDIN>; chomp($doreaderwriter);
if ($doreaderwriter eq "y") {
  $cmd="$cp ";
  foreach $file (@readerwriter) {
    $cmd.="$minet_dev/$file ";
  }
  $cmd.="$minet_execs";
#  print "$cmd";
  system $cmd;
}

print "DONE!\n\n";
print "Don't forget to commit if you're using a repository!\n";





sub UpdateFile {
  my $frompath=shift;
  my $topath=shift;
  my $prompt=shift;
  my ($fromtime, $totime);
  
  $fromtime=GetTime($frompath);
  if (-e $topath) { 
    $totime=GetTime($topath);
  } else {
    $totime=0;
  }

  if ($fromtime>$totime) { 
    $cmd="$cp $frompath $topath";
    print "UPDATING $topath (time=$totime) with $frompath (time=$fromtime)";
    if ($prompt) { 
      print " execute? (y|n) :";
      my $test=<STDIN>; chomp($test);
      if ($test eq "y") {
#	print "$cmd\n";
	system $cmd;
      } else {
	print "SKIPPED\n";
      }
    } else {
#      print "\n$cmd\n";
      system "$cmd"; print "\n";
      
    }
  } else {
    print "skipping $topath (time=$totime) with $frompath (time=$fromtime)\n";
  }
}

sub GetTime {
  my $file=shift;
  my @junk = stat $file;
  return $junk[9];
}

