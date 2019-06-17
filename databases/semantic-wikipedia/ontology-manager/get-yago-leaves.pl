#!/usr/bin/perl
use strict;
use warnings;
my @cols;
my $replacement;
my $lineBuffer;
my %wikicategoryMap = ();
my $key;

if ($#ARGV != 3){
	print "Usage: perl get-yago-leaves.pl <ArticleExtrator> <WordNetLinks> <ConceptLinker> <OutputName>\n";
	exit 1;
}
`touch $ARGV[3]; rm $ARGV[3]; touch $ARGV[3]`;

# Read the concept linker map
print "Reading the info from ConceptLinker now...\n";
open CL, $ARGV[2] or die $!;
while(<CL>)
{
	# Split the line at the tab.
	@cols = split(/\t/, $_);
	
	# second col ($cols[1]) is the key, third col ($cols[2]) the value.
	$wikicategoryMap{$cols[1]} = $cols[2];
}
close(CL);
print "done.\n";

# foreach $key ( keys %wikicategoryMap ) {
# print "$key: $wikicategoryMap{$key} \n";
# } 

open OUTPUT, ">", "tmp_output", or die $!;

print "Building the yago-leaves file now...\n";
# Get the union of the facts and drop YAGO-fact-ID and confidence.
`cut -f2,3 $ARGV[0] $ARGV[1] > tmp_union`;

# Open the file.
open UNION, "tmp_union" or die $!;
# Go through the lines. 
while(<UNION>)
{
	# For the right column check if it is a wikicategory.
	# Split the line at the tab.
	@cols = split(/\t/, $_);
	
	# Simply forward the first colum to the output and add a tab again.
	$lineBuffer = $cols[0] . "\t";
	
	# Chop off the newline.
	chop $cols[1];
	# Second col is $cols[1] now. 	
	if (($cols[1] =~ m/wikicategory/))
	{
		# If it is a wikicategory, replace it by the representative wordnet category.
		print OUTPUT $lineBuffer . $wikicategoryMap{$cols[1]} . "\n" if exists $wikicategoryMap{$cols[1]};
		# If there is none, just drop the whole line.	
	} else 
	{	
		# If it's no wikicategory, leave it as it is.
		print OUTPUT $lineBuffer . $cols[1] . "\n";
	}
}

# Cleanup.
`rm tmp_union`;

close(UNION);
print "done.\n";

close(OUTPUT);
print "Sorting and eliminating duplicates now...\n";
`sort -u tmp_output > $ARGV[3]`;
`rm tmp_output`;
print "done.\n"
