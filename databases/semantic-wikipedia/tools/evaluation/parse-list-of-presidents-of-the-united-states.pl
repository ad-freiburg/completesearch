#!/usr/bin/perl
use strict;
use warnings;

if ( $#ARGV != 2 ) {
	print
"Usage: perl parse-list-of-prsidents-of-the-united-states.pl <ExtracedListsXml> <Query> <OutputName>\n";
	exit 1;
}

print "Parsing List of Presidents of the United States ... ";

my $count = 0;

# open output with append
open OUTPUT, ">>", $ARGV[2], or die $!;

# get contents of the XML.
open XML, "$ARGV[0]";
my $xml = do { local $/; <XML> };

# Select the correct list part.
if ( $xml =~ /\<title\>List of Presidents of the United States(.*?)<\/page>/s ) {
	my $list = $1;

# Select the part from "! [[Vice President of the United States|Vice President]]" to "==Notes==".
	if ( $list =~
		/(.*)\! \[\[Vice President of the United States\|Vice President\]\](.*?)==Notes==(.*)/s )
	{
		my $current = $2;
# Always select the first wikipedia link that is the the third column of a row
		while ( $current =~ /\|\-(.*?)\n\|[^-]([^\n]*?)\n\|[^-]([^\n]*?)\n\|([^\n]*?)\[\[(.*?)[\|\]]/gs ) {

			# Eliminate entries that dont have their own wikipedia article
			if ( $5 !~ /#/ ) {

				# Increase the counter
				$count++;

				# Replace spaces by underscores
				my $entity = $5;
				$entity =~ s/ /_/g;

				# Append to the output in the form query<TAB>entity<NEWLINE>
				print OUTPUT "$ARGV[1]\t$entity\n";
			}
		}
	}
	else {
		print "Unexpected format of the list! \n";
		print
"Failed to find an area between ! [[Vice President of the United States|Vice President]] and ==Notes==!\n";
	}

}
else {
	print "Couldn't find the list in the Xml file!\n";
}

# Close handles and be verbose to avoid errors when using this parser that APPENDS to a output!
close XML;
close OUTPUT;
print "Done. Added $count testcases to $ARGV[2].\n";

