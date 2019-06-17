#!/usr/bin/perl
use strict;
use warnings;
my @cols;
my @words;

if ($#ARGV != 3){
	print "Usage: perl get-yago-facts.pl <WordNetClosure> <ArticleExtrator> <IsAExtractor> <OutputName>\n";
	exit 1;
}
`touch $ARGV[3]; rm $ARGV[3]; touch $ARGV[3]`;

open OUTPUT, ">>", $ARGV[3], or die $!;
print "Processing the wordnet-closure...\t";
open CLOSURE, $ARGV[0] or die $!;
# For each line and each column just cut off wordnet_ and the _number in the end.
while (<CLOSURE>)
{
	# Split the line at the tab.
	@cols = split(/\t/, $_);
	
	# First col.
	# Strip the leading wordnet_ and the number.
	@words = split(/_/, $cols[0]);
	shift @words;
	$#words = $#words - 1;
	foreach(@words)
	{
		$_ = lc($_);	
	}
	print OUTPUT join("_", @words) . "\t";
		
	# Second col.	
	@words = split(/_/, $cols[1]);
	shift @words;
	$#words = $#words - 1;
	foreach(@words)
	{
		$_ = lc($_);	
	}
	print OUTPUT join("_", @words) . "\n";
} 
close(CLOSURE);
print "done.\n";

# ArticleExtractor
print "Processing Aritcle Extractor...\t";
open AE, $ARGV[1] or die $!;
# For each line and each column just cut off wordnet_ and the _number in the end.
while (<AE>)
{
	# Split the line at the tab.
	@cols = split(/\t/, lc($_));
	
	# Check if it is a wikicategory or not. Ignore wikicats.
	if (1- ($cols[2] =~ m/wikicategory/))
	{
		# Seond col.
		$cols[1] = lc($cols[1]);
		print OUTPUT "$cols[1]\t";
		
		# Third col.
		# Strip the leading wordnet_ and the number.	
		@words = split(/_/, $cols[2]);
		shift @words;
		$#words = $#words - 1;
		foreach(@words)
		{
			$_ = lc($_);	
		}
		print OUTPUT join("_", @words) . "\n";	
	}	
	
} 
close(AE);
print "done.\n";

# IsAExtractor
print "Processing IsA Extractor...\t";
open IAE, $ARGV[2] or die $!;
# For each line and each column just cut off wordnet_ and the _number in the end.
while (<IAE>)
{
	# Split the line at the tab.
	@cols = split(/\t/, lc($_));

	# Seond col.
	$cols[1] = lc($cols[1]);
	print OUTPUT "$cols[1]\t";
	
	# Third col.
	# Strip the leading wordnet_ and the number.	
	@words = split(/_/, $cols[2]);
	shift @words;
	$#words = $#words - 1;
	foreach(@words)
	{
		$_ = lc($_);	
	}
	print OUTPUT join("_", @words) . "\n";		
} 
close(AE);
print "done.\n";

close(OUTPUT);