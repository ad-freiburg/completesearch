#!/usr/bin/perl
use strict;
use warnings;

if ( $#ARGV != 2 ) {
	print
"Usage: perl parse-list-of-treaties.pl <ExtracedListsXml> <Query> <OutputName>\n";
	exit 1;
}

print "Parsing List of treaties now ... ";

my $count = 0; 
# open output with append
open OUTPUT, ">>", $ARGV[2], or die $!;

# get contents of the XML.
open XML, "$ARGV[0]";
my $xml = do { local $/; <XML> };

# Select the correct list part.
if ( $xml =~ /(.*)List of treaties(.*?)<\/page>/s ) {
	my $list = $2;

	# Select the part from ==Before 1200 AD== to ==Footnotes==.
	if ( $list =~ /(.*)==Before 1200 AD==(.*?)==Footnotes==(.*)/s ) {
		my $current = $2;
		# Always select the first wikipedia link that in a row, 
		# that is not in a line that has word chars before the link.
		while ( $current =~ /\|\-(.*?)\n(\W*?)\[\[(.*?)[\|\]]/gs ) {
		# Eliminate entries that dont have their own wikipedia article
			if ( $3 !~ /#/ ) {

				# Increase the counter
				$count++;

				# Replace spaces by underscores
				my $entity = $3;
				$entity =~ s/ /_/g;

				# Append to the output in the form query<TAB>entity<NEWLINE>
				print OUTPUT "$ARGV[1]\t$entity\n";
			}
		}
	}
	else {
		print "Unexpected format of the list! \n";
		print "Failed to find an area between ==Before 1200 AD== and ==Footnotes==!\n";
	}

}
else {
	print "Couldn't find the list in the Xml file!\n";
}

# Close handles and be verbose to avoid errors when using this parser that APPENDS to a output!
close XML;
close OUTPUT;
print "Done. Added $count testcases to $ARGV[2].\n";

