#!/usr/bin/perl
use strict;
use warnings;
my @cols;
my %map = ();

if ( $#ARGV != 2 ) {
	print
	  "Usage: perl flag-yago-facts.pl <ArticleExtractor> <YagoFacts> <OutputName>\n";
	exit 1;
}
`touch $ARGV[2]; rm $ARGV[2]; touch $ARGV[2]`;

# Read the paths into a map <name_number -> path>.
# Open the paths file.
print "Read all the articles into a map...\n";
open AE, $ARGV[0] or die $!;

# Read line by line
while (<AE>) {

	# Split each line into its three columns (name, number, path).
	@cols = split( /\t/, $_ );
	$map{lc($cols[1])} = "_concrete__";
}

# Close file handle.
close(AE);
print "done.\n";

# Open output file handle.
open OUTPUT, ">", $ARGV[2], or die $!;

# Open the normal yago facts now
open FACTS, $ARGV[1] or die $!;

# Go through leaves file line by line.
while (<FACTS>) {
	# Split the line at the tab.
	@cols = split( /\t/, $_ );
	my $addString = "";
	if (exists($map{$cols[0]})) {
		$addString = $map{$cols[0]};	
	}
	chop($_);
	print OUTPUT $_ . $addString . "\n";
}
close(FACTS);

# Close output file handle.
close(OUTPUT);
