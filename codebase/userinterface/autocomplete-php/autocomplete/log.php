<?php

class Log
{
  var $level;	// the current level
  var $levels;	// all possible levels
  var $target;	// the current target to log to
  var $targets;	// all possible targets to log to
  var $function_name; // filter for name of the function which is reporting a log message
  var $access_log_file;
  var $error_log_file;
  var $session_name;


  // Constructor
  function Log ()
//  function Log (& $ac)
  {
    $this->level = 2;
    $this->target = 1;
    $this->levels = array ("FATAL" => 1, "ERROR" => 2, "WARNING" => 3, "INFO" => 4, "DEBUG" => 5);
    $this->targets = array("FILE" => 1, "SESSION" => 2, "FILE_SESSION" => 3);
    // Important: don't delete this line: it's necessary to "link" (synchronize) the member "level" to AC->settings->log_level
//    $this->level = & $ac->settings->log_level;
//    $this->function_name = & $ac->settings->log_function_name;
//    $this->session_name = & $ac->settings->session_name;
  }


  function write_stats (& $ac, & $nac, $script_name = "", $request_id = "")
  {
    // NOTE: The '===' is essential because '==' would result in true if $request_id = 0
    $msg_header = date("d.m.Y H:i:s") . ($request_id === "" ? "     " : sprintf(" [%2d]", $request_id));

    //    if (isset($nac->result->time_C)) $nac->result->time_C = $nac->result->time_A + $nac->result->time_B;

    $msg_body = sprintf("| %-15s | %-42s | %-8s %3s | %-20s | %4d +%4d +%4d msecs | %s",
    // $msg_body = sprintf("| %-15s | %-42s | %-8s %3s | %-20s | %5d msecs (%2d%% %2d%% %2d%%) | %s",
      $ac->settings->session_name,
      $ac->query->query_string,
      "C" . $ac->query->max_completions_show . "H" . $ac->query->hits_per_page_while_typing . "F" . $ac->query->first_hit,
      $ac->query->query_types,
      
      // NEW 02Nov07 (Holger): resolve IP behind proxy (e.g., for DBLP)
      isset($_SERVER["HTTP_X_FORWARDED_FOR"]) 
        ?   $_SERVER["HTTP_X_FORWARDED_FOR"] . " [P]"
        :  (isset($_SERVER['REMOTE_HOST'])
              ?   $_SERVER['REMOTE_HOST'] 
              :   $_SERVER['REMOTE_ADDR']),
             // $nac->result->time_C,
             $nac->result->time_A,
             ($nac->result->time_B - $nac->result->time_A),
             ($nac->result->time_C - $nac->result->time_B),
             // $nac->result->time_C,
             // 100 * $nac->result->time_A / $nac->result->time_C,
             // 100 * ($nac->result->time_B - $nac->result->time_A) / $nac->result->time_C,
             // 100 * ($nac->result->time_C - $nac->result->time_B) / $nac->result->time_C,
            $nac->result->error_message_title == ""
              ? str_repeat("*", min($nac->result->time_C / 1000 , 5))
              : "ERROR: " . substr($nac->result->error_message,0,25) . (strlen($nac->result->error_message) > 25 ? "..." : ""),
            true);

    fwrite($this->access_log_file, "\n$msg_header $msg_body" . ($script_name != "" ? " [" . $script_name . ".php]" : ""));

    if (0 && $ac->settings->session_name == "dblpfacets")
    {
      global $times;
      $head = sprintf("\n$msg_header | %-15s | %-40s | ", 
                        $ac->settings->session_name . "_T",
                        $ac->query->query_string
                      );
      foreach($times as $time)
      {
        fwrite($this->access_log_file, $head . $time);
      }
    }
  }


  /**
   * Store a log message into the log file.
   * $level and $function_name are used as filters. For example, if function name filter is set ($function_name != "")
   * only log messages from within function with this name are stored
   *
   * @param string $text
   * @param integer $level
   * @param string $script_name Name of the script (file name) which is reporting the log message
   * @param integer $request_id
   * @param string $function_name Name of function (or method) which is reporting the log message
   */
  function write ($text, $level = 0, $script_name = "", $request_id = "", $function_name = "", $line = "", $target = "", $description = "")
  {
    // NEW Markus / 27-06-09: new debug concept
    global $__error, $__debug;
    
    // Special case: $text == "" --> insert a newline
    if ($text == "" && $this->access_log_file > 0)
    {
      fwrite($this->access_log_file, "\n");
      return;
    }

    // Default for the target parameter
    if (empty($target)) {
      $target = $this->targets["FILE_SESSION"];
    }
    
    // If function name filter is set (log_function_name != "") 
    //  only log messages from within function with same name are stored
    if ($level > $this->level 
      || $this->function_name != "" && $this->function_name != $function_name) return;

    // NEW 25-07-07 (Markus)
    // Now distinguish between error handling and access reporting
    switch ($level)
    {
      // Errors and warnings deliver to the PHP error handler ...
      case $this->levels['FATAL']:
      case $this->levels['ERROR']:
        // NEW Markus 03-07-09: now we use the same format like the PHP error handler
        trigger_error(
            (empty($line) ? "" : " in line $line")
            . (empty($script_name) ? "" : " of $script_name")
            . (empty($function_name) ? "" : " [$function_name]")
            . ": $text",
            E_USER_ERROR);

        // NEW Markus / 27-06-09: new debug concept
        $__error['ERROR'][$script_name][$function_name]["[$line] $description"] = $text;

        break;
        
      case $this->levels['WARNING']:
        // NEW Markus 03-07-09: now we use the same format like the PHP error handler
        trigger_error(
            (empty($line) ? "" : " in line $line")
            . (empty($script_name) ? "" : " of $script_name")
            . (empty($function_name) ? "" : " [$function_name]")
            . ": $text",
            E_USER_WARNING);

        // NEW Markus / 27-06-09: new debug concept
        $__error['WARNING'][$script_name][$function_name]["[$line] $description"] = $text;

        break;
        
        // ... all others write to the access.log file
      default:
        
        if ($this->target & $target == $this->targets['FILE'])
        {
            // NOTE: The '===' is essential because '==' would result in true if $request_id = 0
            $msg_header = date("d.m.Y H:i:s") . ' [' . $this->session_name . '] ' . ($request_id === "" ? "     " : sprintf(" [%2d]", $request_id));

            if ($this->level > 0 && $this->access_log_file > 0) {
              fwrite($this->access_log_file, "\n$msg_header $text"
              . (empty($script_name) ? "" : " [" . $script_name)
              . (empty($line) ? "" : " line $line")
              . (empty($function_name) ? "" : ", $function_name(...)")
              . (empty($script_name) ? "" : "]"));
            }
        }

        // Paranthesis are correct, here a bitwise AND is used
        if (($this->target & $target) == $this->targets['SESSION'] 
            || ($this->target & $target) == $this->targets['FILE_SESSION'])
        {
            // NEW Markus / 27-06-09: new debug concept
            if (empty($description)) {
              $description = "Message";
            }
            if (empty($function_name)) {
              $__debug["Request #$request_id"][$script_name]["[$line] $description"] = $text;
            }
            else {
              $__debug["Request #$request_id"][$script_name][$function_name]["[$line] $description"] = $text;
            }
        }
        
        break;
    }
  }
}

?>
