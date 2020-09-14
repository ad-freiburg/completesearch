#!/usr/bin/perl
use strict;
use warnings;

if ( $#ARGV != 2 ) {
	print
"Usage: perl parse-list-of-computer-scientists.pl <ExtracedListsXml> <Query> <OutputName>\n";
	exit 1;
}

print "Parsing List of computer scientists now ... ";

my $count = 0;

# open output with append
open OUTPUT, ">>", $ARGV[2], or die $!;

# get contents of the XML.
open XML, "$ARGV[0]";
my $xml = do { local $/; <XML> };

# Select the correct list part.
if ( $xml =~ /(.*)List of computer scientists(.*?)<\/page>/s ) {
	my $list = $2;

	# Select the part from ==A== to ==See also==.
	if ( $list =~ /(.*)==A==(.*?)==See also==(.*)/s ) {
		my @lines = split( /\n/, $2 );

		# Go through selected text line by line.
		foreach (@lines) {

			# If the line is a list entry (starting with a "*")
			# Select the content of the double-brackets
			#(wikipedia link) right after the * bullet plus deal
			# with wikipedia syntax (i.e. renaming, hiding, etc)

			if ( $_ =~ /^\*(\s*)\[\[(.*?)[\|\]]/ ) {

				# Increase the counter
				$count++;

				# Replace spaces by underscores
				my $entity = $2;
				$entity =~ s/ /_/g;
				# Append to the output in the form query<TAB>entity<NEWLINE>
				print OUTPUT "$ARGV[1]\t$entity\n";
			}    # otherwise ignore the line.
		}
	}
	else {
		print "Unexpected format of the list! \n";
		print "Failed to find an area between ==A== and ==See also==!\n";
	}

}
else {
	print "Couldn't find the list in the Xml file!\n";
}

# Close handles and be verbose to avoid errors when using this parser that APPENDS to a output!
close XML;
close OUTPUT;
print "Done. Added $count testcases to $ARGV[2].\n";

