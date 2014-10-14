#!/usr/bin/perl -w

$java = "/usr/local/jdk/bin/java";

$file = "monitor.stderr.log";

print STDERR "Sorting\n";

system "./sort_monitor_output.pl > sorted_monitor.out";

print "Displaying\n";

$cmd="$java MinetTimeline sorted_monitor.out 2>/dev/null 1>/dev/null";

system $cmd;

