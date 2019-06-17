#!/usr/bin/perl
my @columns=("S","D","C","O","P","N","B");
my %line;
my $sentenceNumber = 1;
while (<>)
{
  if ($_=~/^C\d.*/)
  {
    next;
  }
  if ($_ eq "\n" && (scalar keys %line) ge 1)
  {
   outputSentence($line);
   %line = ();
   next;
  }
  my @l = split;
  my $head = $l[0];
  $head=~/^(\w)\d.*/;
  $head=$1;
  if ($head ne "")
  {
    @{$line{$head}} = @l[1..$#l];
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
    #$output = $output.$key.($key eq "B" ? "" : "\t");
  }
  #$output .= "\n";
  for my $i (0..$#{$line{$columns[0]}})
  {
    push(@{$line{"D"}}, $sentenceNumber);
    push(@{$line{"C"}}, 0);
    push(@{$line{"O"}}, 1);
  }
  $sentenceNumber += 1;
  for my $i (0..$#{$line{$columns[0]}})
  {
    foreach $key (@columns)
    {
     $value = @{$line{$key}}[$i];
     if ($value eq ""){
       $value = "*";
     }
     $output = $output.$value.($key eq @columns[$#columns] ? "" : "\t");
    }
    $output .= "\n";
  }
  $output .= "\n";
  print $output;
}

