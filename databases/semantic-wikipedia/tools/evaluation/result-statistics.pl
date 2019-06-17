#!/usr/bin/perl
use strict;
use warnings;

if ( $#ARGV != 0 ) {
	print "Usage: result-staticstics.pl <results file>\n";
	exit 1;
}

open RESULT, "$ARGV[0]";

my $currentQuery = "none";
my $nofLines     = 0;
my $nofTrue      = 0;
my $nofFalse     = 0;
my $nofMissing   = 0;

my $totalLines   = 0;
my $totalTrue    = 0;
my $totalFalse   = 0;
my $totalMissing = 0;

while (<RESULT>) {

	# Split the line at the tab.
	my @cols = split( /\t/, $_ );

	my $query = $cols[0];
	my $res   = $cols[2];

	# handle the first query
	if ( $currentQuery =~ /none/ ) {
		$currentQuery = $query;
	}

	# if this is a new query, write statistics for the last one
	if ( !( $currentQuery eq $query ) ) {


		my $fMeasure;
		my $precision;
		my $recall;
		if ($nofTrue + $nofFalse == 0){
			$precision = 0;
		} else {
			$precision  = $nofTrue / ( $nofTrue + $nofFalse );
		}
		if ($nofTrue + $nofMissing == 0){
			$recall = 0;
		} else {
			$recall  = $nofTrue / ( $nofTrue + $nofMissing );
		}

		if ($precision + $recall == 0){
			$fMeasure = 0;
		} else{
			$fMeasure = 2 * ( $precision * $recall ) / ( $precision + $recall );
		}
		print "Query: $currentQuery\n";
		print "\n";
		print "\t Lines: $nofLines\n";
		print "\t True: $nofTrue\n";
		print "\t False: $nofFalse\n";
		print "\t Missing: $nofMissing\n";
		print "\n";
		print "\t Precision: $precision\n";
		print "\t Recall: $recall\n";
		print "\t F-Measure: $fMeasure\n";
		print "\n";
		print "\n";

		# update totalCounter
		$totalLines   += $nofLines;
		$totalTrue    += $nofTrue;
		$totalFalse   += $nofFalse;
		$totalMissing += $nofMissing;

		# reset local counters
		$currentQuery = $query;
		$nofLines     = 0;
		$nofTrue      = 0;
		$nofFalse     = 0;
		$nofMissing   = 0;
	}

	# increase counters
	$nofLines++;
	if ( $res =~ /TRUE/ )    { $nofTrue++; }
	if ( $res =~ /FALSE/ )   { $nofFalse++; }
	if ( $res =~ /MISSING/ ) { $nofMissing++; }
}

# Process the last, remaining query

my $precision = $nofTrue / ( $nofTrue + $nofFalse );
my $recall    = $nofTrue / ( $nofTrue + $nofMissing );
my $fMeasure = 2 * ( $precision * $recall ) / ( $precision + $recall );

print "Query: $currentQuery\n";
print "\n";
print "\t Lines: $nofLines\n";
print "\t True: $nofTrue\n";
print "\t False: $nofFalse\n";
print "\t Missing: $nofMissing\n";
print "\n";
print "\t Precision: $precision\n";
print "\t Recall: $recall\n";
print "\t F-Measure: $fMeasure\n";
print "\n";
print "\n";

# update totalCounter
$totalLines   += $nofLines;
$totalTrue    += $nofTrue;
$totalFalse   += $nofFalse;
$totalMissing += $nofMissing;

# Write total numbers
$precision = $totalTrue / ( $totalTrue + $totalFalse );
$recall    = $totalTrue / ( $totalTrue + $totalMissing );
$fMeasure  = 2 * ( $precision * $recall ) / ( $precision + $recall );

print "Total numbers:\n";
print "\n";
print "\t Lines: $totalLines\n";
print "\t True: $totalTrue\n";
print "\t False: $totalFalse\n";
print "\t Missing: $totalMissing\n";
print "\n";
print "\t Precision: $precision\n";
print "\t Recall: $recall\n";
print "\t F-Measure: $fMeasure\n";
print "\n";
print "\n";

close RESULT;
