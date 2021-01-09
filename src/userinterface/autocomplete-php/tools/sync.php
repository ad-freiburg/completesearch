<?php

//$url = "http://geek2:8080/time-server";
$url = "http://geek2.ag1.mpi-sb.mpg.de/markus/time.php";
//$url = "http://search.mpi-sb.mpg.de/markus/tools/time.php";
$global_dc = 0;
//$global_dc = 1000000;
$global_diff = 0.21;

echo "\n\nErmittle Zeitdifferenz zwischen localhost und Server ...";
/*
for ($i=0; $i<10; $i++)
{
  $here1 = microtime();
  $r = file($url);
  $here2 = microtime();
  $there = $r[0];

  $h1 = microtime_float($here1);
  $t = microtime_float($there);
  $h2 = microtime_float($here2);

  $dc = $h2 - $h1;
  $diff = $h1 - $t;
  if ($dc < $global_dc) {
    $global_dc = $dc;
    $global_diff = $diff;
  }


  echo "\n";
  echo "$h1 sec\n";
  echo "$t sec\n";
  echo "$h2 sec\n";

  echo "\n";
  echo "Dauer: " . $dc . " sec\n";
  echo "Differenz: " . $diff . " sec";
  echo "\n";
  echo "\n";

}
*/
echo "\nBester Versuch: Differenz: $global_diff  (+/- $global_dc)";


// Nun eine schlechte Verbindung suchen
echo "\n\nSuche nun schlechte Laufzeit ...";

for ($i=0; $i<100; $i++)
{
  $here1 = microtime();
  $r = file($url);    // Serverzeit
  $here2 = microtime();
  $there = $r[0];
  $h1 = microtime_float($here1);
  $t = microtime_float($there);
  $h2 = microtime_float($here2);

  $dc = $h2 - $h1;

  if ($dc > 1) {
    echo "\nSchlechte Laufzeit gefunden, Dauer: $dc";
//    echo "\nZeit hier: $h1, dort: $t";
    $max = $t + $global_diff - $h1 + $global_dc;
    $min = $t + $global_diff - $h1 - $global_dc;

    echo "\n\nDauer für PHP-Initialisierung: <= " . $max;
    if ($min > 0) echo "\n\nDauer für PHP-Initialisierung: >= " . $min;
//    echo "\n\nDauer für PHP-Initialisierung: " . ($t + $global_diff - $h1) . " (+/- $global_dc)";
    break;
  }
}

echo "\n";

function microtime_float($time)
{
  list($usec, $sec) = explode(" ", $time);
  return (float)$usec + (float)$sec;
}

?>