#!/usr/bin/perl -w

=head1 DESCRIPTION

perl table.pl --- format text from STDIN as ascii table

Converts \t characters in so many spaces that at chunks of a fixed size (default
20) have a nice human readable form.

=cut

use strict;

my $BATCH_SIZE = 50;

while (1)
{
  exit if eof STDIN;
  my @field_widths = ();
  my @lines = ();
  # read next BATCH_SIZE lines
  foreach (1..$BATCH_SIZE)
  {
    my $line = <>;
    last if not $line;
    $line =~ s/\s*$//;
    my @fields = split("\t",$line);
    foreach my $i (0..$#fields) {
      if ( $#field_widths < $i || length $fields[$i] > $field_widths[$i]) {
        $field_widths[$i] = length $fields[$i]; } }
    push @lines, $line;
  }
  my $format_string = "";
  foreach (@field_widths) { $format_string .= "%-${_}s   "; }
  #print " --> \"$format_string\"\n" and next;
  foreach my $line (@lines) { printf "$format_string\n", split("\t",$line); }
}
