#!/usr/bin/env perl

$match=$ARGV[0];

$CMD = "ps x";

open(FOO, "$CMD |");

while (<FOO>) {
    chomp;
    $line = $_;
    @fields = split;
    $cmd = join(' ',@fields[4 .. $#fields]);
    if ($cmd =~ $match) {
	$CMD = "kill $fields[0]";
	print STDERR "KILL $line\n";
	system "$CMD";
    }
}
