<?php
include_once("result.php");
include_once("log.php");

/**
 * PROVIDE THE AC OBJECT (either copy from session or create from scratch using values from $config).
 * Needs:	$config->session_name to resume or create session,
 * 			$config->autocomplete_path to load AC class declaration
 * 			and $config if we have to cretae AC from scratch.
 */

// The name of this script (used to identify log message)
$path_info = pathinfo(__FILE__);
define("SCRIPT_AC_INIT", $path_info["basename"]);

// NEW Markus / 29-06-09: use a log object which is independant from AC object (not member of AC class)
// to provide logging, too, when AC object is not availabe (before AC object is created, for example)
$log = new Log();
$log->level = $config->log_level;
// NEW 29.11.06 (Markus): Define the different log levels (are used in javascript part, too)
//$log->levels = array ("FATAL" => 1, "ERROR" => 2, "WARNING" => 3, "INFO" => 4, "DEBUG" => 5);
// NEW 14Aug12 (baumgari): Check if request_id is set. This is obviously not 
// done before the session has started and some query was sent (so in pressing 
// the reset button)
if (isset($query->request_id))
  $log->write($log, $log->levels['DEBUG'], SCRIPT_AC_INIT, $query->request_id, __FUNCTION__, __LINE__, $log->targets["SESSION"], "\LOG object created \$log");

// The AC class declaration
require ("AC.php");


// Create a unique session name from the application name unless one is already provided (by autocomplete_config.php)
if (! isset($config->session_name)) {
	$config->session_name = $config->application_name;
}


// NEW 25.01.07 (Markus): for PHP > 5.1 we get an warning if we use date() without setting timezone
// Set the default timezone to use. Available since PHP 5.1
if ((float)phpversion() >= 5.1) {
	date_default_timezone_set('Europe/Berlin');
}

$language_changed = false;

// Create the new AC object for storing result and changements of this query
//  Note: initialize so less as possible to minimize the amount of member which are updated in a later call of update($_SESSION['AC'], $NAC, true);
$NAC = new AC();

// NEW 14Aug12 (baumgari): Check if request_id is set. This is obviously not 
// done before the session has started and some query was sent (so in pressing 
// the reset button)
if (isset($query->request_id))
  $log->write("Session started", $log->levels['DEBUG'], SCRIPT_AC_INIT, $request_id, __FUNCTION__, __LINE__);

//$NAC->query = new Query();
$NAC->result = new Result();


// Start session
// As we are using cookie-based sessions, we must call session_name() and session_start() before anything is output to the browser
session_name($config->session_name);
session_cache_expire(24 * 60); // in hours (here one day)
session_start();


// NEW 03-03-08 (Markus)
if (array_key_exists("reset", $_GET))
{
  // Unset all of the session variables.
  $_SESSION = array();

  // If it's desired to kill the session, also delete the session cookie.
  // Note: This will destroy the session, and not just the session data!
  if (isset($_COOKIE[$config->session_name])) {
    setcookie($config->session_name, '', time() - 42000, '/');
  }

  // Finally, destroy the session.
  session_destroy();
}

//
// session_unset();
//
// NOTE 1: use this "session_unset()" only for debugging purposes: it will reset the AC
// object (useful to avoid having to close the browser after changes in the AC class definition). If
// active, back button etc. will *not* work
//
// NOTE 2: very strange things can happen with the session management. For example, if the AC object
// somehow gets into a bad state (e.g., variables in request object without values), there will be
// very strange errors everywhere, until the session is reset.


// DEBUGGING 03-07-07 (Markus): sometimes the AC object is not of type AC but stdClass, I don't know why
// In PHP >= 5 we should use instanceof (we will get a warning otherwise)
if (array_key_exists("AC", $_SESSION))
{
	//if (isset($_SESSION["AC"]) && ! $_SESSION["AC"] instanceof AC) {  // PHP >= 5
	if (isset($_SESSION["AC"]) && ! @is_a($_SESSION["AC"], "AC")) {  // PHP < 5
		trigger_error('_SESSION[AC] is not of AC type, I will recreate it', E_USER_WARNING);
	}
}


//
// CASE 1: COPY AC OBJECT FROM SESSION
//
if (array_key_exists("AC", $_SESSION))
{
	// In PHP >= 5 we should use instanceof (we will get a warning otherwise)
	//if (isset($_SESSION["AC"]) && ($_SESSION["AC"] instanceof AC))  // PHP >= 5
	if (isset($_SESSION["AC"]) && is_a($_SESSION["AC"], "AC"))  // PHP < 5
	{
		$log->write("Restore AC object from session", $log->levels['DEBUG'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);

		include_once("times.php"); // functions and variables for time measurement
	
		// Clone AC object to commit session as soon as possible
    if ((float)phpversion() >= 5) {
		  $AC = clone ($_SESSION['AC']);	// PHP 5
    }
    else {
		  $AC = unserialize(serialize($_SESSION['AC']));
    }
    
    // TODO Markus / 29-06-09
    $AC->log = & $log;
    // $log->level = & $AC->settings->log_level;
    
		// Commit session
		session_commit();
		$log->write("Session committed", $log->levels['DEBUG'], SCRIPT_AC_INIT, $AC->query->request_id, __FUNCTION__, __LINE__);
		saveTimestamp('after: first session_start/commit()');
	
		// Check whether the version of the appication changed (mean that it's version is newer than the version of the saved session)
	  //	if (version_compare($AC->version, VERSION) == -1) {
	  //	  echo "Your saved session ($AC->version) is obsolete, because a newer version (" . VERSION . ") of this application is released";
	  //	  echo "<br>Sorry, but we have to reset your application ...";
	  //	  session_name($session_name);
	  //	  session_start();
	  //	  session_unset();
	  //	  session_destroy();
	  //
	  //	  echo "<br><a href='$index_url'>Back to search page</a>";
	  //	  exit;
	  //	}
	
		// Open the log files and store their handles.
		// This have to be done every time (even if we can continue the current session because the handles are closed with end of the script)
		$AC->log->error_log_file = fopen($AC->settings->error_log, "a")
			or print "ERROR opening error log " . $AC->default_settings->error_log . " while resum session" and exit;
		$AC->log->access_log_file = fopen($AC->settings->access_log, "a")
			or $AC->log->write("ERROR opening " . $AC->settings->access_log . " while resume session", $AC->log->levels['ERROR'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);
	
	
		// SET LANGUAGE PARAMETER AND COOKIE
		//
		//	It's used for language change in both modes: with *and* without javascript
		//	Note: this calls *must* happen before the first html output, because of a call to setcookie() unless ob_start is used (output_buffering)
		//
		$tmp = $AC->settings->language;
		$AC->set_parameter_and_cookie_from_url($AC->settings->language, $AC->settings->varnames_abbreviations['language']);
		if (! isset($NAC->settings)) {
			$NAC->settings = new Settings();
		}
		$NAC->settings->language = $AC->settings->language;
	
		$language_changed = ($AC->settings->language != $tmp);
	}
}


//
// CASE 2: INITIALIZE AC OBJECT (from user config + non-configurable default values)
//
else
{
	// If the session is expired and we came from ajax.php
	// we first have to read configuration from autocomplete_config.php
	if (!isset($config->server_hostname) /* or any of the other config vars */)
	{
		// Session is expired and we have no values for $config (means we came from ajax.php) except of thoses ones passed by the AJAx call itself
		// So we have to read configuration from autocomplete_config.php
		include($_SERVER['DOCUMENT_ROOT'] . $config->index_path . "autocomplete_config.php");

		$expired = true;
	}
	else $expired = false;

	$AC = new AC();
	// Initialize all members representing objects with a corresponding class object
	$AC->initialize();


	// A. PREPARING FOR ERROR HANDLING AND PARAMETER NAMING CONVENTIONS
	//

  // The following parameters must be set so early as possible because they are used for error handling or parameter naming conventions
  //
	// Mapping of variable names (AC members) to the abreviations used in the URL's and cookies
	if (isset($config->varnames_abbreviations)) {
		$AC->default_settings->varnames_abbreviations = $config->varnames_abbreviations;
	}

	// The members concerning logging must be set so early as possible (because to be able to use the method AC->log())
	//

	// NEW 29.11.06 (Markus): Define the different log levels (are used in javascript part, too)
//	$AC->log->levels = array ("FATAL" => 1, "ERROR" => 2, "WARNING" => 3, "INFO" => 4, "DEBUG" => 5);

    // File name of the access log
	if (isset($config->access_log)) {
		$AC->default_settings->access_log = $config->access_log;
	}

	// File name of the error log
	if (isset($config->error_log)) {
		$AC->default_settings->error_log = $config->error_log;
	}

	// If $config provides a log level use it as default value in $AC->default_settings.
	// If not the default is this one provided by the initialize method of $AC->settings
	if (isset($config->log_level)) {
		$AC->default_settings->log_level = $config->log_level;
	}
	else {
	  $config->log_level = $AC->default_settings->log_level;
	}

	// NEW 23.04.07 (Markus): Now set the level of the $AC->log to this log level
	// NEW 03.10.08 / Markus: if log level is not an user preference it must not be set from cookie!
	if (array_key_exists("log_level", $config->user_preferences)) {
    $AC->set_parameter_from_cookie($AC->log->level, $AC->default_settings->varnames_abbreviations["log_level"], $AC->default_settings->log_level);
	}
	else {
    $AC->log->level = $config->log_level;
//    $AC->set_parameter($AC->log->level, $AC->default_settings->varnames_abbreviations["log_level"], $config->log_level);
	}
	
  // Set the log level of the error handler (function set_php_log_level is part of error_handler.php)
  set_php_log_level($AC->log->level);

	// Open the log files and store their handles
	$AC->log->error_log_file = fopen($AC->default_settings->error_log, "a")
		or print "ERROR opening error log " . $AC->default_settings->error_log . " while creating new AC" and exit;
	$AC->log->access_log_file = fopen($AC->default_settings->access_log, "a")
		or $AC->log->write("ERROR opening access log " . $AC->settings->access_log . " while creating new AC", $AC->log->levels['ERROR'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);



	//	B. SET $AC->default_settings
	//

	//	1. In a first step all members of $AC->default_settings are set by a general procedure
	//		Some of them are modified in the second step
	//

	//	$AC->default_settings contains default values for the most $AC->settings members.
	//	These ones can be overridden by values defined by $config (in autocomplete_config.php)
	//	So replace all $AC->default_settings members by existing $config members
	foreach (get_object_vars($AC->settings) as $name => $value)
	{
	  if (isset($config->$name))
	  {
	    $AC->default_settings->$name = $config->$name;
	    $AC->log->write("settings->$name is set to " . $config->$name . " (provided by \$config)", $AC->log->levels['DEBUG'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);
	  }
	}

	//	2. In the second step some members of $AC->default_settings are modified, extracted from PHP variables or in other way
	//

	// NEW 10-03-08 (Markus): It must be possible to define every of these both separately by $config
	//	(for example, in relay.php we define a value for index_page to override the name of the relay script)
	// If at least one of index_path or index_page are not set by $config build them from the value of $_SERVER['PHP_SELF']
	if (! (isset($AC->default_settings->index_path) && isset($AC->default_settings->index_page)))
	{
		// Determine the file name of the index page of the application and the path to it
		preg_match("/^(.*)\/(.*)$/", $_SERVER['PHP_SELF'], $matches);
		// But change only those parameters which are not set by $config
		if (! (isset($AC->default_settings->index_path))) {
			$AC->default_settings->index_path = $matches[1] . "/";
		}
		if (! (isset($AC->default_settings->index_page))) {
			$AC->default_settings->index_page = $matches[2];
		}
	}

	// The URL of the application folder (relative to web server's document root)
	if (! isset($AC->default_settings->index_url)) {
		$AC->default_settings->index_url = get_full_url($AC->default_settings->index_path);
	}

	// The URL of the autocomplete folder: if not already set by $config derive it from $AC->default_settings->autocomplete_path
	if (! isset($AC->default_settings->autocomplete_url)) {
		$AC->default_settings->autocomplete_url = get_full_url($AC->default_settings->autocomplete_path);
	}

	// URL to the stylesheet file, relative to the autocomplete folder
	$AC->default_settings->css_file = $AC->default_settings->index_url . $AC->default_settings->css_file;

	// We have to apply the specified encoding
	$AC->default_settings->capitals = $AC->charset_encode($AC->default_settings->capitals, $AC->default_settings->encoding);

	// Append a '/' to the autocomplete path, in case user did not add one
  if (substr($AC->default_settings->autocomplete_path, -1, 1) != "/") {
   	$AC->default_settings->autocomplete_path .= "/";
  }

	// NEW 27.03.07 (Markus): determine the port number from the corresponding file in directory "/var/opt/autocomplete/ports/"
	// The file has the name <service_name>.port and contains only the port number
	if (is_numeric($AC->default_settings->server_port))	{
		$AC->default_settings->service_name = "-";
	}
	else
	{
		$tmp = "/var/opt/completesearch/ports/" . $AC->default_settings->server_port . ".port";
		if ( ($r = ltrim(rtrim(file_get_contents($tmp)))) != "" && is_numeric($r))
		{
			$AC->default_settings->service_name = strtoupper($AC->default_settings->server_port);
			$AC->default_settings->server_port = $r;
		}
		else echo "** wrong service name or port number: " . $AC->default_settings->server_port . "**";
	}


	// 	C. SET $AC->settings
	//

	//	1.	Now set those members of $AC->settings which can be set by cookies
	//

	//	Those ones which are defined in $AC->default_settings->user_preferences
	foreach ($AC->default_settings->user_preferences as $pref)
	{
		if (! array_key_exists($pref, get_class_vars('Settings')))
		{
				$AC->log->write("Parameter in user_preferences is not member of class 'Settings' (while saving AC->default_settings): $pref", $AC->log->levels['WARNING'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);
				continue;
		}
		// NEW 05-08-07 (Markus)
		$AC->set_parameter_from_cookie($AC->settings->$pref, $AC->default_settings->varnames_abbreviations[$pref], $AC->default_settings->$pref);
	}

	// Language is a special user preference (not part of user_preferences but changeable by user by a special link)
	$AC->set_parameter($AC->settings->language, $AC->default_settings->varnames_abbreviations['language'], $AC->default_settings->language);


	//	2.	Set all $AC->settings members by their corresponding $AC->default_settings members except if they are already set
	// 		(by a cookie, for example)
	foreach (get_object_vars($AC->settings) as $name => $value)
	{
		// If this parameter is not yet defined (by a cookie, for example) set it from the corresponding AC->default_settings member
		if (! isset($AC->settings->$name))
		{
			if (isset($AC->default_settings->$name)) {
				$AC->settings->$name = $AC->default_settings->$name;
			}
			else {
				$AC->log->write("Parameter $name (AC->settings) is nether set by \$config nor by \$default_settings", $AC->log->levels['WARNING'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);
			}
		}
	}


	//	D.	SET $AC->query FROM URL OR $AC->settings
	// 		NOTE: Because of cookie handling, this calls have to happen before first output is written (except when we use ob_start())!
	//

	//	1.	First declare some special parameters (which are not declared in AC->settings)
	//
	// CHANGED 24-04-09 (Markus): this is now in the constructor
//	$AC->query->first_hit = 1;
//  $AC->query->first_hit_shown = 1;

	//	2.	Now set the remaining members of the AC->query object from AC->settings;
	//		(Is it necessary to get it from URL? Yes, at least for query_types when we come from ajax.php (parallel requests))
	foreach (get_object_vars($AC->query) as $name => $value)
	{
		// Set only those members which have an corresponding member in $AC->settings
		if (isset($AC->settings->$name))
		{
      // NEW 06-08-07 (Markus): Cookies should not changed by this call (what happened with AC->set_parameter)
			$AC->set_parameter_from_url($AC->query->$name, $AC->settings->varnames_abbreviations[$name], $AC->settings->$name);
		}
	}

	// NEW 22.05.07 (Markus)
	// Check whether the multibyte string extension mbstring is installed in cases that utf-8 should be used as encoding
	if (strtolower($AC->settings->encoding) == "utf-8" && ! function_exists("mb_strtolower"))
	{
		echo "For use of multibyte string encodings the mbstring extension is needed which is probably not installed.";
		echo "<br />Please change the encoding in the autocomplete_config.php or install the mbstring extension.";
    trigger_error("For use of multibyte string encodings the mbstring extension is needed which is probably not installed. Please change the encoding in the autocomplete_config.php or install the mbstring extension.", E_USER_ERROR);
		exit;
	}


	//	E.	SOME INITIALIZATION OF STATE VARIABLES AND HANDLES AND INCLUDING OF TRANSLATIONS
	//

	// State of autocomplete processing
	$AC->state = "start";

	// NEW 22-03-08 (Markus): Member javascript has not be taken from $config yet
	if (isset($config->javascript)) {
	  $AC->javascript = $config->javascript;
	}

	// Include the translations for the text patterns
	include get_full_path($AC->settings->autocomplete_path) . "text.php";

    // NEW 15-07-08 (Markus): be sure that $config->custom_text_file_name is set
    //  The error control operator @ is necessary because if $config->custom_text_file_name
    //  is not defined (in autocomplete_config.php) isset() would result in a notice
//	$AC->log->write("custom_text_file_name: " . $AC->settings->custom_text_file_name, $AC->log->levels['INFO'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);
	if (@isset($AC->settings->custom_text_file_name)
	     && @$AC->settings->custom_text_file_name != "")
	{
	  $custom_text_file_name = preg_match("|^/|", $AC->settings->custom_text_file_name)
      	  ? $AC->settings->custom_text_file_name
      	  : get_full_path($AC->default_settings->index_path) . $AC->settings->custom_text_file_name;
	  if ($custom_text_file_name != "" && is_readable($custom_text_file_name)) {
//    	$AC->log->write("custom_text_file_name is set: " . $custom_text_file_name, $AC->log->levels['INFO'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);
	    include $custom_text_file_name;
	  }
	}
//	$AC->log->write("AC->text: " . print_r($AC->text, true), $AC->log->levels['INFO'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);

	// NEW 14-11-2007 (Markus): Text pattern can now be overridden by new $config->text parameter
	if (isset($config->text) && is_array($config->text)) {
	 $AC->text = array_merge($AC->text, $config->text);
	}

	// If the session was expired and we recreated it write this to the log file
	if ($expired) {
		$AC->log->write($AC->get_text("session_expired_and_recreated"), $AC->log->levels['INFO'], SCRIPT_AC_INIT, "", __FUNCTION__, __LINE__);
	}

	// settings->max_completions_show is now defined by the first member of the settings->top_hits array (new "top X" concept)
	if (sizeof($AC->settings->top_hits) > 0) {
	  $AC->settings->max_completions_show = $AC->settings->top_hits[0];
	}
	else {
	  // Which should never happen
	  $AC->settings->max_completions_show = 4;
	}

	// If provided in $config set the transformation array (it's not part of settings because it's only for PHP not for Javascript)
//  $AC->hit_transformation = array();
//  if (isset($config->hit_transformation) && is_array($config->hit_transformation)) {
//    $AC->hit_transformation = $config->hit_transformation;
//  }
	
	//	F.	STORE THE CREATED AC OBJECT IN THE SESSION VARIABLE
	//

	$_SESSION["AC"] = $AC;
	session_commit();
	
  // NEW Markus / 29-06-09: new debug concept
  $_SESSION["DEBUG"] = array();
  $_SESSION["ERROR"] = array();
//	$AC->log->write(print_r($AC, true), $AC->log->levels['DEBUG'], SCRIPT_AC_INIT);
}

	
// Check whether transformations are provided and load them if so
$file = $_SERVER['DOCUMENT_ROOT'] . "/" . $AC->settings->index_path . "transformations.php";
if (is_readable($file)) {
  include $file;
}

?>
