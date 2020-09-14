#!/usr/bin/perl
use strict;
use warnings;

if ( $#ARGV != 0 ) {
        print "Wrong number of arguments\n";
        exit 1;
}

my $nextnumber = 1;
my $numTokens = 0;

open GT,$ARGV[0], or die $!;
while(<GT>)
{
  if($_=~/[SPNC](\d+)\t.*/)
  {
    if ($1 >= $nextnumber)
    {
      $nextnumber = $1+1;
    }
  }
}
close GT;

open GT,">>",$ARGV[0], or die $!;

my $newsentence = "";

while(<STDIN>)
{
  $_=~s/([SPNC])(\t)(.*)/$1$nextnumber$2$3/g;
  $newsentence .= $_;
  my @tmp = split(/ /,$3);
  $numTokens = $#tmp;
}

print "Adding:\n$newsentence";

if ($newsentence ne "")
{
  print GT "\n";
  print GT $newsentence;
  print GT "B$nextnumber\t".("* " x ($numTokens))."*\n";
  print GT "C$nextnumber\tXX\n";
  print GT "C$nextnumber\tXX\n";
}

close GT;
