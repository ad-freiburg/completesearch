#!/usr/bin/perl
use strict;
use warnings;
my $lastFactsSize;
my $factsSize;
my $countCommand;
my $ret;
my $i;

$countCommand = "wc -l $ARGV[1] | sed -e 's/^[ \t]*//' | cut -d ' ' -f1";

if ($#ARGV != 1){
	print "Usage: perl get-wordnet-facts.pl <WordnetLinksFile> <OutputFile>\n";
	exit 1;
}

`cut -f2,3 $ARGV[0] | sort -k2,2 > $ARGV[1]`;
`cut -f2,3 $ARGV[0] | sort -k1,1 > tmp_joinFacts`;
$lastFactsSize = 0;
$factsSize = `$countCommand`;

print "\nBuilding the transitive deductive closure of the wordnet categories...\n"; 
while($factsSize > $lastFactsSize){
	`cp $ARGV[1] tmp_facts`;
	$lastFactsSize = $factsSize;
	`join -t "	" -1 2 -2 1 -o "1.1 2.2" tmp_facts tmp_joinFacts >> tmp_facts`;
	`sort tmp_facts -k2,2 | uniq > $ARGV[1]`;
	$factsSize = `$countCommand`;
	print "Performed a join. This join added " . ($factsSize-$lastFactsSize) . " facts. \n"; 
}
`rm tmp_facts`;
`rm tmp_joinFacts`;
print "construction of closure done.\n";