#!/usr/bin/perl -w

$#ARGV == 7 || die "make_shim.pl uppername lowername upperclass lowerclass fromupperfifo toupperfifo fromlowerfifo tolowerfifo\n";


($uppername,
 $lowername,
 $upperclass,
 $lowerclass,
 $fromupperfifo,
 $toupperfifo,
 $fromlowerfifo,
 $tolowerfifo) = @ARGV;

print END ;
#include "config.h"
#include "shim.h"

int main()
{
  RunShim<$upperclass,$lowerclass>($uppername,
				   $lowername,
				   $toupperfifo,
				   $fromupperfifo, 
				   $tolowerfifo, 
				   $fromlowerfifo);
};
END;
