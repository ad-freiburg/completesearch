#!/usr/bin/perl
use strict;
use warnings;
my @cols;
my @parts;
my $newCount;
my $oldCount;

if ( $#ARGV != 2 ) {
	print
"Usage: perl get-yago-leaves.pl <WordNetLinks> <ConceptLinker> <OutputName>\n";
	exit 1;
}

#################################################
# --------------- PREPARATIONS ------------------
#################################################

`touch $ARGV[2]; rm $ARGV[2]; touch $ARGV[2]`;

# Remove some categories to make the resulting paths more clever.
my $removeCategories = "| grep -v causal_agent";

# Build a join-partner (WordNetLinks sorted by right hand side).
# Columns with yagoActor are ignored.
`cut -f2,3 $ARGV[0] | grep -v yagoActor $removeCategories > tmp_pairs`;

# Find all categories without a parent
`cut -f1 tmp_pairs | sort -u > tmp_left;
 cut -f3 $ARGV[0] $ARGV[1] | sort -u > tmp_right; 
 join -v2 tmp_left tmp_right | grep -v wordnet_entity_ | sort -u > tmp_missing`;

# Remember the correct identifier for concept "entity".
my $entity = `grep wordnet_entity_ tmp_right | sort -u`;

# Remove temporary files
`rm tmp_left; rm tmp_right`;

# Write a part <missing> - wordnet_entity_ ... for each into join file.
open TP, ">>", "tmp_pairs" or die $!;
open MIS, "tmp_missing" or die $!;
while (<MIS>) {
	chop $_;
	print TP "$_\t$entity\n";
}
close MIS;
close TP;

# Sort the join partners
`sort -k2,2  tmp_pairs | grep -v '^\$' > tmp_join_partner`;

# Remove temporary files
`rm tmp_missing`;
`rm tmp_pairs`;

#################################################
# --------------- START OF LOGIC ----------------
#################################################

# Start with entity
`cut -f3 $ARGV[0] | grep wordnet_entity_ | uniq > tmp_output`;

my $countCommand =
  "wc -l tmp_output_sorted | sed -e 's/^[ \t]*//' | cut -d ' ' -f1";

# Sort output by left hand side.
`sort -k1,1 tmp_output > tmp_output_sorted`;
$oldCount = 0;
$newCount = `$countCommand`;

# Repeat until nothing new:
while ( $newCount > $oldCount ) {
	print "About to perform a join...\n ";
	$oldCount = $newCount;

# Join with join-partner. Write lhs of join partner on the lhs of the whole, current output.
`join -t "	" -1 2 -2 1 -o "1.1 0 2.2 2.3 2.4 2.5 2.6 2.7 2.8 2.9 2.10 2.11 2.12 2.13 2.14 2.15 2.16 2.17 2.18 2.19 2.20 2.21 2.22 2.23" tmp_join_partner tmp_output_sorted > tmp_join_result`;

	# Union last output and the new one (we also need sub-paths).
	`cat tmp_join_result >> tmp_output`;

	# Eliminate duplicates and sort.
	`sort -u tmp_output > tmp_output_sorted`;
	$newCount = `$countCommand`;
	print "Performed a join. This join added "
	  . ( $newCount - $oldCount )
	  . " facts. \n";
}
print "Building the paths done. Still messy, needs proper formatting.\n";

# Properly format the output now.
print "Creating properly formatted output now...\n";

open OUTPUT, ">", $ARGV[2], or die $!;
open TMP, "tmp_output_sorted" or die $!;

# Go through temp output line by line.
while (<TMP>) {

	# Split the column at the tab.
	@cols = split( /\t/, $_ );

	# Remove the newline from that last col.
	chop $cols[$#cols];

# Look at left-most element to determine the key name & number (the number is kept for joins with the leaves).
	my $current = $cols[0];

	# Stript off wordnet_ and split the name from the number.
	@parts = split( /_/, $current );
	shift @parts;
	my $number = pop @parts;

	# Print name and number to the output.
	foreach (@parts) {
		$_ = lc($_);
	}
	print OUTPUT join( '_', @parts ) . "\t$number\t";

	# Start from the back.
	@cols = reverse @cols;

	# For each column.
	my $lineBuffer = "";
	foreach my $col (@cols) {

		# Ignore "empty" columns.
		if ( $col =~ m/\S/ ) {

			# Split each column at the underscore.
			@parts = split( /_/, $col );

			# Strip off wordnet_ and the number.
			shift @parts;
			pop @parts;

			# Write the name to an output buffer, followed by a colon.
			foreach (@parts) {
				$_ = lc($_);
			}
			$lineBuffer = $lineBuffer . join( '_', @parts ) . ':';
		}
	}

	# Remove the last colon and write a newline instead.
	chop $lineBuffer;
	$lineBuffer = $lineBuffer . "\n";

	# Write the buffer to the output.
	print OUTPUT $lineBuffer;
}

# Clean temporary files and close file handles.
close(OUTPUT);
`rm tmp_join_partner`;
`rm tmp_join_result`;
`rm tmp_output_sorted`;
`cat $ARGV[2] > tmp_output`;

#################################################
# --------------- POSTPROCESSING ----------------
#################################################

print
"Post-processing the paths now in order to optimize them and remove unnecessary paths...\n";
open PATHS, "tmp_output" or die $!;
open OUTPUT, ">", $ARGV[2], or die $!;
my $entityName = "__init";
my $number     = -1;
my @entityPaths;
while (<PATHS>) {

	# Split the line
	@cols = split( /\t/, $_ );

	# Remove the newline from that last col.
	chop $cols[$#cols];
	my $currentEntity = $cols[0];
	my $currentNumber = $cols[1];

	# For a new entity, process the old one and setup everything for the next.
	if ( !( $number == $currentNumber ) ) {

		# Do the processing
		my @sorted = sort {
			my $countA = ( $a =~ tr/:// );
			my $countB = ( $b =~ tr/:// );
			if ( $countA == $countB ) {
				return -( $a cmp $b );
			}
			return ( $countA <=> $countB );
		} @entityPaths;
		
		# Only write paths that add at least one new fact. The sorting
		# ensures that everything can still be found since paths
		# up to some point are identical independet of the final leaf.
		my %seen = ();
		for my $path (@sorted) {
			my $doneSomething = 0;
			my @facts = split( /:/, $path );
			for my $fact (@facts) {
				if ( !$seen{$fact} ) {
					$seen{$fact} = 1;
					$doneSomething = 1;
				}
			}
			if ($doneSomething) {
				print OUTPUT "$entityName\t$number\t$path\n";
			}
		}

		# Setup the next entity
		$entityName   = $currentEntity;
		$number       = $currentNumber;
		$#entityPaths = -1;
	}

	# For the current entity just append to the paths array
	push( @entityPaths, $cols[2] );
}

# Finally process the last entity!
my @sorted = sort {
	my $countA = ( $a =~ tr/:// );
	my $countB = ( $b =~ tr/:// );
	if ( $countA == $countB ) {
		return -( $a cmp $b );
	}
	return ( $countA <=> $countB );
} @entityPaths;
my %seen = ();
for my $path (@sorted) {
	my $doneSomething = 0;
	my @facts = split( /:/, $path );
	for my $fact (@facts) {
		if ( !$seen{$fact} ) {
			$seen{$fact} = 1;
			$doneSomething = 1;
		}
	}
	if ($doneSomething) {
		print OUTPUT "$entityName\t$number\t$path\n";
	}
}
close OUTPUT;
close PATHS;
`rm tmp_output`;
print "Done creating $ARGV[2].\n";
