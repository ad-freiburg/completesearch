#!/usr/bin/perl
use strict;
use warnings;
my @cols;
my %map = ();

if ( $#ARGV != 2 ) {
	print
	  "Usage: perl get-yago-facts.pl <yago-leaves> <yago-paths> <OutputName>\n";
	exit 1;
}
`touch $ARGV[2]; rm $ARGV[2]; touch $ARGV[2]`;

# Read the paths into a map <name_number -> path>.
# Open the paths file.
print "Reading the paths into a map...\n";
open PATHS, $ARGV[1] or die $!;

# Read line by line
while (<PATHS>) {

	# Split each line into its three columns (name, number, path).
	@cols = split( /\t/, $_ );

	# Insert the pair <"wordnet"_name_number, path> into the map.
	push @{ $map{"wordnet_$cols[0]_$cols[1]"} }, $cols[2];
}

# Close file handle.
close(PATHS);
print "done.\n";

# Now create the yago facts map.
print "Creating the yago-facts file now...\n";

# Open output file handle.
open OUTPUT, ">", $ARGV[2], or die $!;

# Open the leaves file
open LEAVES, $ARGV[0] or die $!;

# Go through leaves file line by line.
while (<LEAVES>) {

	# Split the line at the tab.
	@cols = split( /\t/, $_ );

	# Remove the newline from the second col.
	chop $cols[1];

	# Check if something is found for this leaf.
	if ( exists $map{ lc( $cols[1] ) } ) {

		# If there are one or more paths, output the entity plus the path
		# for each path. For entities that are wordnet categories, remove
		# the wordnet prefix and the number.
		my $entity = $cols[0];
		if ( $entity =~ m/wordnet_/ ) {

			# Remove wordnet_ and the number
			my @parts = split( /_/, $entity );
			shift @parts;
			$#parts = $#parts - 1;
			$entity = join( "_", @parts );
		}
		foreach my $path ( @{ $map{ lc( $cols[1] ) } } ) {
			$path =~ s/_//g;
			print OUTPUT lc($entity) . "\t$path";
		}
	}
	else {

		# If nothing is found, output some text.
		print "Couldn't find a path for $cols[1]!\n";
	}
}

# Close leaves file.
close(LEAVES);

# Close output file handle.
close(OUTPUT);
`cat $ARGV[2] > tmp_output`;
#################################################
# --------------- POSTPROCESSING ----------------
#################################################

print "Post-processing the facts now in order to remove unnecessary facts...\n";
open FACTS, "tmp_output" or die $!;
open OUTPUT, ">", $ARGV[2], or die $!;
my $entityName = "__init";
my @entityPaths;
while (<FACTS>) {

	# Split the line
	@cols = split( /\t/, $_ );

	# Remove the newline from that last col.
	chop $cols[$#cols];
	my $currentEntity = $cols[0];

	# For a new entity, process the old one and setup everything for the next.
	if ( !( $entityName eq $currentEntity ) ) {

		# Do the processing
		my @sorted = sort {
			my $countA = ( $a =~ tr/:// );
			my $countB = ( $b =~ tr/:// );
			if ( $countA == $countB ) {
				return $a cmp $b;
			}
			return ( $countB <=> $countA );
		} @entityPaths;

		# Only write paths that are no prefix of another path!
		my $pathNum = 0;
		for my $path (@sorted) {
			my $isAPrefix = 0;
			for ( my $i = 0 ; $i < $pathNum ; $i++ ) {
				$isAPrefix = $isAPrefix || ( $sorted[$i] =~ m/$path/ );
			}
			$pathNum++;
			if ( !$isAPrefix ) {
				print OUTPUT "$entityName\t$path\n";
			}
		}

		# Setup the next entity
		$entityName   = $currentEntity;
		$#entityPaths = -1;
	}

	# For the current entity just append to the paths array
	push( @entityPaths, $cols[1] );
}

# Finally process the last entity!
my @sorted = sort {
	my $countA = ( $a =~ tr/:// );
	my $countB = ( $b =~ tr/:// );
	if ( $countA == $countB ) {
		return $a cmp $b;
	}
	return ( $countB <=> $countA );
} @entityPaths;

# Only write paths that are no prefix of another path!
my $pathNum = 0;
for my $path (@sorted) {
	my $isAPrefix = 0;
	for ( my $i = 0 ; $i < $pathNum ; $i++ ) {
		$isAPrefix = $isAPrefix || ( $sorted[$i] =~ m/$path/ );
	}
	$pathNum++;
	if ( !$isAPrefix ) {
		print OUTPUT "$entityName\t$path\n";
	}
}
close OUTPUT;
close FACTS;
`rm tmp_output`;

print "Done constructing $ARGV[2].\n"
