<?php
// Functions and variables for time measurement


/**
 * The global array which holds timestamps combined with a (descriptive) name.
 *
 * An entry looks like: "name|timestamp"
 */
$times = array(microtime_float());


/*
 * Time in microseconds (replicates PHP 5 behaviour)
 *
 * @return float time in microseconds
 */
function microtime_float()
{
  list($usec, $sec) = explode(" ", microtime());
  return (float)$usec + (float)$sec;
}


/**
 * Save the current time as timestamp with name $name in the global array $times
 *
 * @param string $name name with which the timestamp is saved
 */
function saveTimestamp($name)
{
  global $times;
  $times[] = $name . "|" . (floor((microtime_float() - $times[0]) * 10000) / 10);
}
?>
