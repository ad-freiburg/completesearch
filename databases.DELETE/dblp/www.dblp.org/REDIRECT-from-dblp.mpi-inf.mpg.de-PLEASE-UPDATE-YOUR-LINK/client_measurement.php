<?php
  // get measurements from URL
  $query  = isset($_POST["query"])  ? $_POST["query"]  : "[NO QUERY]";
  $types  = isset($_POST["types"])  ? $_POST["types"]  : "[NO TYPES]";
  $time_D = isset($_POST["time_D"]) ? $_POST["time_D"] : -1;
  $time_E = isset($_POST["time_E"]) ? $_POST["time_E"] : -1;

  // get IP address (again, consider getting it from javascript)
  $ip = isset($_SERVER["HTTP_X_FORWARDED_FOR"])                                                                                                                
          ?   $_SERVER["HTTP_X_FORWARDED_FOR"] . " [P]"                                                                                                        
          :  (isset($_SERVER['REMOTE_HOST'])                                                                                                                   
                ?   $_SERVER['REMOTE_HOST']                                                                                                                    
                :   $_SERVER['REMOTE_ADDR']);

  // print to log file
  $line = sprintf("%s      | %-15s | %-42s | %12s | %-20s | %5s msecs / %5s msecs | %s\n", 
                  date("d.m.Y H:i:s"),
                  "dblpclient",
                  $query,
                  $types,
                  $ip,
                  $time_E < 1000*1000 ? $time_E : "inf",
                  $time_D < 1000*1000 ? $time_D : "inf",
                  str_repeat("#", $time_E < 1000*1000 ? min($time_E / 1000, 10) : 0)
                );
//  $log = fopen("/var/opt/completesearch/log/completesearch.access_log", "a");
  $log = fopen("/var/log/dblp/access.log", "a");
  if($log != false) { fwrite($log, $line); fclose($log); }

  // also return something (will become contents of iframe)
  /*
  echo "Query : \"" . $query . "\"<br>\n";
  echo "Types : \"" . $types . "\"<br>\n";
  echo "Time D: " . $time_D . " msec<br>\n";
  echo "Time E: " . $time_E . " msec<br>\n";
  */
?>
