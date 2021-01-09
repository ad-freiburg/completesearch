<?php

/**************************************************************************************************
Mandatory settings
*/

// URL of site should be tested
//$url = "http://localhost/MPI/autocomplete-php/";
$url = "http://search.mpi-inf.mpg.de/markus.tests/";

// Path to the access.log file (for testing logging, summary line, for instance)
//$logPath = "C:\Dokumente und Einstellungen\www\MPI\autocomplete-php\access.log";
$logPath = $url . "access.log";

/**
	 * Array of browser details
	 *
	 * First parameter is the abbreviation used above to select the browser for this test sequence
	 * Second one is the name (can be changed by user)
	 * Third one is the selenium name (should not be changed) and optional the execution path.
	 * 	The last one is not necessary if the path to the browser executable is in the Windows PATH variable
	 * Fourth one is the abbreviation used in the log file to identify the browser used for a test
	 *
	 */
$browserDefs = array(
  "s" => new Browser("s", "Safari", "*safari C:\Programme\Web\Safari\Safari.exe", "SF"),
  "f" => new Browser("f", "Mozilla Firefox", "*firefox", "FF"),
  "o" => new Browser("o", "Opera", "*opera C:\Programme\Opera\Opera.exe", "OP"),
  "i" => new Browser("i", "Internet Explorer", "*iexplore", "IE")
//  "i" => new Browser("i", "Internet Explorer", "*custom C:\Programme\Internet Explorer\iexplore.exe", "IE")
);


/**************************************************************************************************
Default settings (don't have to be changed)
*/

// Selection of browser(s) to use for this test sequence (default: all browsers)
//  (f)irefox
//  (i)nternet explorer
//  (o)pera
$browser = "ofi";

// Duration of the "pause" command (realized in PHP by "sleep()")
$waitForExecution = 1.5;

?>