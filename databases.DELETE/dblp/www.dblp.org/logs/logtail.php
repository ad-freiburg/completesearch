<?
error_reporting(E_ALL);
// logtail.php
function readLog($file, $numLines)
{
  $cmd = "wc -l " . $file . " | cut -d \" \" -f1";
  exec("$cmd 2>&1", $lines);
  if (intval($numLines) == 0) $linesToPrint = intval(30);
  else if (intval($numLines) >= intval($lines[0]))
    $linesToPrint = intval($numLines) - intval($lines[0]);
  else $linesToPrint = intval(0);
  if (in_array(0, $lines, true)) echo ("$lines[0]\n");
  else echo ("0\n");
  
  $cmd = "tail -q -n " . intval($linesToPrint) . " " . $file;
  exec("$cmd 2>&1", $output);
  foreach($output as $outputline)
  {
    echo ("$outputline\n");
  }
}

readLog($_REQUEST['path'], $_REQUEST['lines']);
?>
