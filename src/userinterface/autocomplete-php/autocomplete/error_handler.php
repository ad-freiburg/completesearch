<?php

// Use output buffering to enable that only the error messages are shown
ob_start();

// Check whether to use an own error handling
//if ($config->use_own_error_handling) {
 	// Set the system error handling to our own handler
 	// This prevent for example error message shown in the HTML code
set_error_handler("my_error_handler");
//}

if (!isset($AC)) {
	$logfile = @fopen($config->error_log, "a");
}
else {
	$logfile = &$AC->log->error_log_file;
//	$logfile = &$AC->settings->error_log_file;
}

// Error handler function
function my_error_handler ($errno, $errstr, $errfile, $errline)
{
	global $logfile;

	$date = date("d.m.Y H:i:s");

	// NEW 08Oct06 (Holger): use error log file name from config
	if (! $logfile) {
		@send_email("kontakt@tetzlaff.org", "MPI", "Fatal Error in 'error_handler.php': error log file could not be opened\n");
	}

	// Note: in the E_USER_X cases it makes no sense to deliver errline and errfile
	//   because these cases are triggered manually (without system information about line or file)
	switch ($errno) {
		case E_USER_ERROR:
			if ($logfile) {
				@fwrite($logfile, "$date: My error [$errno] $errstr\n");
//				echo "<br><b>My error [$errno] $errstr</b>";
			}
			break;

		case E_USER_WARNING:
			if ($logfile) {
				@fwrite($logfile, "$date: My warning [$errno] $errstr\n");
			}
			break;

		case E_USER_NOTICE:
			if ($logfile) {
				@fwrite($logfile, "$date: My notice [$errno] $errstr\n");
			}
			break;

		case E_COMPILE_ERROR:
		case E_PARSE:
		case E_CORE_ERROR:
		case E_ERROR:
			if ($logfile) {
				@fwrite($logfile, "$date: PHP error [$errno] $errstr<br />\n");
				echo "<br><b>PHP error [$errno] $errstr</b>";
			}
			@send_email("kontakt@tetzlaff.org", "MPI", "$date: Fatal Error: [$errno] $errstr in line $errline in $errfile<br />\n");
			exit();

		case E_WARNING:
		case E_CORE_WARNING:
		case E_COMPILE_WARNING:
			if ($logfile) {
				@fwrite($logfile, "$date: PHP warning [$errno] in line $errline of $errfile: $errstr\n");
			}
			break;

		case E_NOTICE:
		case E_STRICT:
			if ($logfile) {
				@fwrite($logfile, "$date: PHP notice [$errno] in line $errline of $errfile: $errstr\n");
  		}
			break;

		default:
			if ($logfile) {
				@fwrite($logfile, "$date: Unkown error type: [$errno] $errstr in line $errline in $errfile\n");
			}
	}
}


function set_php_log_level ($level)
{
  switch ($level) {
  	case 1:
  	case 2:
  		error_reporting(E_ERROR);
  		break;

  	case 3:
  		error_reporting(E_WARNING);
  		break;

  	case 4:
  		error_reporting(E_NOTICE);
  		break;

  	default:
  		error_reporting(E_ALL); // All errors and warnings, as supported, except of level E_STRICT
//  		error_reporting(E_STRICT);  // PHP >= 5: Run-time notices. Enable to have PHP suggest changes to your code which will ensure the best interoperability and forward compatibility of your code
  		break;
  }
}


function send_email ($empfänger, $betreff, $text, $absender = "system")
{
//	mail ($empfänger, "Autocomplete: $betreff", $text, "From: system");
}


?>
