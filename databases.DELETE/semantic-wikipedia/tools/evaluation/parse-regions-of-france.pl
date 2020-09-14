#!/usr/bin/perl
use strict;
use warnings;

if ( $#ARGV != 2 ) {
	print
"Usage: perl parse-regions-of-france.pl <ExtracedListsXml> <Query> <OutputName>\n";
	exit 1;
}

print "Parsing Regions of France now ... ";

my $count = 0;

# open output with append
open OUTPUT, ">>", $ARGV[2], or die $!;

# get contents of the XML.
open XML, "$ARGV[0]";
my $xml = do { local $/; <XML> };

# Select the correct list part.
if ( $xml =~ /\<title\>Regions of France(.*?)<\/page>/s ) {
	my $list = $1;

# Select the part from "==Regions and their capitals== ... Notes" to "==See also==".
	if ( $list =~
		/(.*)\==Regions and their capitals==(.*?)Notes(.*?)==See also==(.*)/s )
	{
		my $current = $3;

# Always select the first wikipedia link that is the the seconds column of a row
		while ( $current =~ /\|\-(.*?)\n(.*?)\n(.*?)\[\[(.*?)[\|\]]/gs ) {

			# Eliminate entries that dont have their own wikipedia article
			if ( $4 !~ /#/ ) {

				# Increase the counter
				$count++;

				# Replace spaces by underscores
				my $entity = $4;
				$entity =~ s/ /_/g;

				# Append to the output in the form query<TAB>entity<NEWLINE>
				print OUTPUT "$ARGV[1]\t$entity\n";
			}
		}
	}
	else {
		print "Unexpected format of the list! \n";
		print
"Failed to find an area between ==Regions and their capitals== ... Notes and ==See also==!\n";
	}

}
else {
	print "Couldn't find the list in the Xml file!\n";
}

# Close handles and be verbose to avoid errors when using this parser that APPENDS to a output!
close XML;
close OUTPUT;
print "Done. Added $count testcases to $ARGV[2].\n";

