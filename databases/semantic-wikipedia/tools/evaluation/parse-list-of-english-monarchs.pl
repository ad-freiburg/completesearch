#!/usr/bin/perl
use strict;
use warnings;

if ( $#ARGV != 2 ) {
	print
"Usage: perl parse-list-of-english-monarchs.pl <ExtracedListsXml> <Query> <OutputName>\n";
	exit 1;
}

print "Parsing List of English monarchs ... ";

my $count = 0;

# open output with append
open OUTPUT, ">>", $ARGV[2], or die $!;

# get contents of the XML.
open XML, "$ARGV[0]";
my $xml = do { local $/; <XML> };

# Select the correct list part.
if ( $xml =~ /\<title\>List of English monarchs(.*?)<\/page>/s ) {
	my $list = $1;

# Select the part from "==House of Mercia==" to "==Timeline of English Monarchs==".
	if ( $list =~
		/(.*)==House of Mercia==(.*?)==Timeline of English Monarchs==(.*)/s )
	{
		my $current = $2;

# Always select the first wikipedia link that is the the first column of a row
		while ( $current =~ /\|\-(.*?)\n(.*?)\[\[(.*?)[\|\]]/gs ) {

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
		print
"Failed to find an area between ==House of Mercia== and ==Timeline of English Monarchs==!\n";
	}

}
else {
	print "Couldn't find the list in the Xml file!\n";
}

# Close handles and be verbose to avoid errors when using this parser that APPENDS to a output!
close XML;
close OUTPUT;
print "Done. Added $count testcases to $ARGV[2].\n";

