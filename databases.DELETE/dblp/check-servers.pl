#!/usr/bin/perl -w

=head1 DESCRIPTION

Usage: perl check-servers.pl

Check whether servers (specified via URLs) are up.

=cut

use strict;

my $curl_cmd = "/usr/bin/curl -Ls"; # -L means follow redirects; -s means silent
my $log_filename = "logs/check-servers.log";
my $to_email = "Ina Baumgarten <baumgari\@informatik.uni-freiburg.de>, Hannah Bast <bast\@informatik.uni-freiburg.de>";
my $from_email = "CompleteSearch <dontreply\@informatik.uni-freiburg.de>";
my @urls = (
            "http://dblp.org/search",
            "http://test.dblp.org/search"
            #"http://search.mpi-inf.mpg.de/cgi-bin/autocomplete.pl?database=mpi-webpages",
            #"http://search.mpi-inf.mpg.de/cgi-bin/autocomplete.pl?database=homeopathy",
            #"http://search.mpi-inf.mpg.de/wikipedia_en",
            #"http://search.mpi-inf.mpg.de/wikipedia_de",
            #"http://search.mpi-inf.mpg.de/library",
            #"http://search.mpi-inf.mpg.de/linux-manpages",
            #"http://search.mpi-inf.mpg.de/phpdoc",
            #"http://search.mpi-inf.mpg.de/wikipedia",
            #"http://search.mpi-inf.mpg.de/robust",
            #"http://www.homeonet.org/homeopathy",
            #"http://search.mpi-inf.mpg.de/dblp",
            #"http://search.mpi-inf.mpg.de/dblp-facets",
            #"http://search.mpi-inf.mpg.de/dblp-plus",
            #"http://search.mpi-inf.mpg.de/ester",
            #"http://geek2.ag1.mpi-sb.mpg.de/wikipedia_en",
	   );

my @urls_facets = (
                    "http://www.dblp.org/db/indices/a-tree/p/Poor:H=_Vincent.html",
                    "http://test.dblp.org/db/indices/a-tree/p/Poor:H=_Vincent.html",
                    "http://www.informatik.uni-trier.de/~ley/db/indices/a-tree/p/Poor:H=_Vincent.html"
                  );

my @urls_api = (
                 "http://www.dblp.org/search/api/?q=test*",
                 "http://test.dblp.org/search/api/?q=test*"
               );

my @secs_between_trials = (10, 20, 30);

# 1. OPEN LOG FILE (create if it doesn't exist yet)
open(LOG, (-f $log_filename ? ">>" : ">").$log_filename) or die "$@$!";
print LOG "\n"."CHECKING SERVERS ".`date`."\n";

# 2. CHECK URLS - mainpage
my $nof_trials = $#secs_between_trials + 2;
foreach my $url (@urls)
{ 
  my $cmd = "$curl_cmd $url | egrep \"div.*(subtitle|parameters)\"";
  my $nof_hits;
  print LOG "Command: $cmd\n";
  for (my $trial = 1; $trial <= $nof_trials; $trial++)
  {
    if ($trial > 1) 
    { 
      my $secs = $secs_between_trials[$trial - 2]; 
      print LOG "waiting $secs ".($secs == 1 ? "second" : "seconds")." ...\n";
      sleep $secs_between_trials[$trial - 2]; 
    }
    my $info = `$cmd`;
    $info =~ s/^\s*(.*?)\s*$/$1\n/;
    print LOG "Trial $trial: $info";
    $nof_hits = $url =~ /cgi-bin/
                          ? ($info =~ /<br>.*?<br>(.*?)<br>.*?<br>/ ? $1 : 0)
                          : ($info =~ /searching in (.*?) documents/ ? $1 : 0);
    last if $nof_hits > 0;
  }
  if ($nof_hits > 0)
  {
    print LOG "$nof_hits HITS -> OK\n";
  }
  else
  {
    my $app_name = ($url =~ /([\\=][\w_-]+)$/ ? $1 : $url);
    print LOG "NOT OK, SENDING EMAIL TO $to_email\n";
    open(MAIL, "| /usr/lib/sendmail -t");
    print MAIL "From: $from_email\n";
    print MAIL "To: $to_email\n";
    print MAIL "Subject: CHECK FAILED for \"$app_name\"\n";
    print "Subject: CHECK FAILED for \"$app_name\"\n";
    print MAIL "Full URL: $url\n";
    print MAIL "Tried $nof_trials times, waiting ", join(", ", @secs_between_trials), " seconds inbetween\n";
    print MAIL "\n";
    print MAIL "automatic email sent by \"$0\", on ".`date`;
    close(MAIL);
  }
  print LOG "\n";
}

# 3. CHECK URLS - facets on static dblp sites
# !!! Not working since the javascript isn't executed and therefore not in the
# result of curl.
=begin GHOSTCODE
$nof_trials = $#secs_between_trials + 2;
foreach my $url (@urls_facets)
{ 
  my $info = "";
  my $cmd = "$curl_cmd $url | egrep 'name=\"box_F\".*href=\".*\"'";
  print LOG "Command: $cmd\n";
  for (my $trial = 1; $trial <= $nof_trials; $trial++)
  {
    if ($trial > 1) 
    { 
      my $secs = $secs_between_trials[$trial - 2]; 
      print LOG "waiting $secs ".($secs == 1 ? "second" : "seconds")." ...\n";
      sleep $secs_between_trials[$trial - 2]; 
    }
    $info = `$cmd`;
    print LOG "Trial $trial: $info";
    last if $info ne "";
  }
  if ($info ne "")
  {
    print LOG "Facets OK -> OK\n";
  }
  else
  {
    my $app_name = ($url =~ /([\\=][\w_-]+)$/ ? $1 : $url);
    print LOG "NOT OK, SENDING EMAIL TO $to_email\n";
    #open(MAIL, "| /usr/lib/sendmail -t");
    #print MAIL "From: $from_email\n";
    #print MAIL "To: $to_email\n";
    #print MAIL "Subject: CHECK FAILED for \"$app_name\"\n";
    print "Subject: FACETS CHECK FAILED for \"$app_name\"\n";
    #print MAIL "Full URL: $url\n";
    #print MAIL "Tried $nof_trials times, waiting ", 
    #join(", ", @secs_between_trials), " seconds inbetween\n";
               #print MAIL "\n";
               #print MAIL "automatic email sent by \"$0\", on ".`date`;
               #close(MAIL);
  }
  print LOG "\n";
}
=end GHOSTCODE
=cut

# 4. CHECK URLS - search api
$nof_trials = $#secs_between_trials + 2;
foreach my $url (@urls_api)
{ 
  my $info = "";
  my $cmd = "$curl_cmd $url | egrep '<status.*</status>'";
  print LOG "Command: $cmd\n";
  for (my $trial = 1; $trial <= $nof_trials; $trial++)
  {
    if ($trial > 1) 
    { 
      my $secs = $secs_between_trials[$trial - 2]; 
      print LOG "waiting $secs ".($secs == 1 ? "second" : "seconds")." ...\n";
      sleep $secs_between_trials[$trial - 2]; 
    }
    $info = `$cmd`;
    print LOG "Trial $trial: $info";
    $info =~ /<status code=\".*\">(\w*)<\/status>/;
    $info = $1;
    last if $info eq "OK";
  }
  if ($info eq "OK")
  {
    print LOG "Search api OK -> OK\n";
  }
  else
  {
    my $app_name = $url;
    print LOG "NOT OK, SENDING EMAIL TO $to_email\n";
    open(MAIL, "| /usr/lib/sendmail -t");
    print MAIL "From: $from_email\n";
    print MAIL "To: $to_email\n";
    print MAIL "Subject: SEARCH API - CHECK FAILED for \"$app_name\"\n";
    print MAIL "Full URL: $url\n";
    print MAIL "Tried $nof_trials times, waiting ", join(", ", @secs_between_trials), " seconds inbetween\n";
    print MAIL "\n";
    print MAIL "automatic email sent by \"$0\", on ".`date`;
    close(MAIL);
  }
  print LOG "\n";
}
