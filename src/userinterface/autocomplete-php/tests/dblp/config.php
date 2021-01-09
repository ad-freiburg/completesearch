<?php

/**************************************************************************************************
Mandatory settings
*/

// URL of site should be tested
$url = "http://dblp.mpi-inf.mpg.de/dblp-mirror/"; // the real DBLP
//$url = "http://geek2.ag1.mpi-sb.mpg.de/markus/dblp-mirror/";  // Markus' test DBLP

// Path to the access.log file (for testing logging, summary line, for instance)
//$logPath = "C:\Dokumente und Einstellungen\www\MPI\autocomplete-php\access.log";
//$logPath = "D:\Projekte\Web\MPI\autocomplete-php\access.log";
$logPath = "http://geek2.ag1.mpi-sb.mpg.de/markus/dblp-mirror/access.log";

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
  "f" => new Browser("f", "Mozilla Firefox", "*firefox", "FF"),
  "o" => new Browser("o", "Opera", "*opera C:\Programme\Web\Opera\Opera.exe", "OP"),
  "i" => new Browser("i", "Internet Explorer", "*iexplore C:\Programme\Internet Explorer\iexplore.exe", "IE")
  //"f" => new Browser("f", "Mozilla Firefox", "*firefox C:\Programme\Web\Mozilla\Firefox\firefox.exe", "FF"),
  //"i" => new Browser("i", "Internet Explorer", "*iexplore", "IE")
);




/**************************************************************************************************
Default settings (don't have to be changed)
*/

// Selection of browser(s) to use for this test sequence (default: all browsers)
//  (f)irefox
//  (i)nternet explorer
//  (o)pera
$browser = "f";

// Duration of the "pause" command (realized in PHP by "sleep()")
$waitForExecution = 2.5;

?>