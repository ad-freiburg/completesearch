#!/usr/bin/perl
my @columns = ("S","P","N","B");
my %line;
my $n = 1;

while (<>)
{
  if (($_ eq "\n" || $_ eq " \n" ) && (scalar keys %line) ge 1)
  {
   outputSentence($line);
   %line = ();
   next;
  }
  my @l = split;
  for my $i (0..$#l)
  {
    push (@{$line{$columns[$i]}}, $l[$i]);
  }
}


if ((scalar keys %line) ge 1)
{
 outputSentence($line);
 %line = ();
}



sub outputSentence
{
  my $line = shift;
  my $output = "";
  foreach $key (@columns)
  {
    $output = $output.$key.$n."\t";
    my @arr = @{$line{$key}};
    foreach $l (0..$#arr)
    {
      $output .= @arr[$l].($l eq $#arr ? "": " ");
    }
    $output .= "\n";
  }
  $output .= "C$n\tXX\n";
  $output .= "\n";
  $n = $n +1;
  print $output;
}
