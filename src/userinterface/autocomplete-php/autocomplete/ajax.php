<?php

/**
 * AJAX.PHP (on the way to becoming identical to autocomplete.php)
 */
// Necessary?
header("content-type: text/html; charset=utf-8");

// The name of this script (used to identify log message)
$path_info = pathinfo(__FILE__);
define("SCRIPT_AJAX", $path_info["basename"]);

include_once("times.php"); // functions and variables for time measurement; set the start timestamp $times[0]

// FOR TESTING ONLY!! This provocates that the AC object must be created from scratch
//session_name($_POST['name']);
//session_start();
//unset ($_SESSION['AC']);
//session_commit();

// Provide own error handling and set the system error handling to our handling
include_once("error_handler.php");

/**
 * THE FOLLOWING VALUES WE HAVE TO KNOW TO RESUME THE SESSION OR TO CRATE AC OBJECT FROM SCRATCH (reading configuration from the autocomplete_config.php)
 */
// Path to the error log file as first to provide error handling as soon as possible
$config->error_log = isset($_POST['log']) ?  $_POST['log'] : $_GET['log'];
$config->log_level = isset($_POST['ll']) ?  $_POST['ll'] : $_GET['ll'];

set_php_log_level($config->log_level);

//// NEW Markus / 27-06-09: new debug concept
$__debug = array();
$__error = array();

// Now the remaining $config variables
$config->index_path = isset($_POST['path']) ?  $_POST['path'] : $_GET['path'];
$config->index_page = isset($_POST['page']) ?  $_POST['page'] : $_GET['page'];

// The following we need in case we can't resume our session and therefor have to create the AC from scratch
// (reading configuration from the autocomplete_config.php)
$config->session_name = isset($_POST['name']) ?  $_POST['name'] : $_GET['name'];


include_once("helper.php");
include_once("generate_javascript.php");

$request_id = (integer) variable("rid");

/**
 * 1. GET CURRENT AC OBJECT
 */
saveTimestamp('before: include(AC_init.php)');
require("AC_init.php");
saveTimestamp('after: include(AC_init.php)');


//echo "<br>AC: " . print_r($AC, true) . "<br><br";

// Clean output buffer to prevent empty lines to be sent to client
ob_clean();

// This script is called via AJAX, so we can be sure that javascript is activated
$AC->javascript = true;
$NAC->javascript = true;

// Initialize query and request members because we will change their values by the next processing
$NAC->query = new Query();
$NAC->result = new Result();

// Take the query history from the AC object
// Note: It's necessary to set the history member in $NAC because of the session AC object update
//  (only whar changed in $NAC is also changed in the session AC object).
//  To set $NAC->history as alias (reference) to $AC->history is sufficiant for the update mechanism to recognize that history is changed.
// Markus 27-06-09: not longer used
//$NAC->history = &$AC->history;

$NAC->result->time_C = 0; // timer for processing of this script


/**
 * 2. SET QUERY (from url)
 */

// First some special parameters
$AC->set_parameter_from_url($AC->query->first_hit, "fh", null, true);
// TODO Do we need this?
//$NAC->query->first_hit = $AC->query->first_hit;

// NEW 31.01.07 (Markus): don't save request_id longer in $AC->query->id, because since parallel processing of request they both are different
$AC->query->request_id = (integer) variable("rid");
$NAC->query->request_id = $AC->query->request_id;

// NEW 08Dez11 (Ina): Store start_completion_index and completion_index, since else it gets lost.
$NAC->result->completion_start_index = -1;
$NAC->result->completion_index = -1;

// NEW Markus / 27-06-09: new debug concept
$last_qid = $AC->query->id;

$AC->set_parameter_from_url($AC->query->id, "qid", null, true);
//$AC->set_parameter_from_url($AC->query->port, "prt");

$AC->query->query_string = variable('query');


// NEW 29-02-08 (Markus): set $query_index; if no value is provided set $query_index to "" (means all indexes)
$tmp = variable('qi');
$AC->query->query_index = is_null($tmp) ? "" : $tmp;

$NAC->query->first_hit_shown = (integer) variable("fhs");
// NEW 11-12-07 (Markus)
$AC->first_hit = (integer) variable("fhs");
$NAC->first_hit = (integer) variable("fhs");

// NEW 06-03-08 (Markus)
$inactivity = variable("inactivity");

// NEW 17-07-07 (Markus): Iterate now over varnames_abbreviations instead of user_preferences
// Set all query members which are declared in $AC->settings->varnames_abbreviations and delievered with $_POST
foreach ($AC->settings->varnames_abbreviations as $parameter => $abbreviation)
{
	$value = &$_POST[$abbreviation];
	if (isset($value))
	{
		$AC->log->write("Set from POST: \$AC->query->$parameter = " . $value, $AC->log->levels['DEBUG'], SCRIPT_AJAX, $request_id, __FUNCTION__, __LINE__);
		$AC->query->$parameter = $value;
	}
}

$AC->log->write("BEGIN request #" . $request_id . " (query #" . $AC->query->id .
    "): '" . $AC->query->query_string . "' (type " . $AC->query->query_types . ")", $AC->log->levels['INFO'], SCRIPT_AJAX, $request_id, __FUNCTION__, __LINE__);

// NEW 06-03-08 (Markus)
//if (isset($inactivity)) $AC->log->write("inactivity: $inactivity", $AC->log->levels['INFO'], SCRIPT_AJAX, $request_id);


// NEW 24.05.07 (Markus): if the encoding of the collection is utf-8 but the content type of the request header is not 
//  (request was not sent utf-8-encoded) encode query string explicit
//  NOTE 05-10-08: This don't work for Firefox 3, it says allways "utf-8" even for not-utf-8-encoded requests
//if (strtolower($AC->settings->encoding) == "utf-8" && strchr(strtolower($_SERVER['CONTENT_TYPE']), "utf-8") === false) {
if (strtolower($AC->settings->encoding) == "utf-8") {
	$AC->query->query_string = utf8_encode($AC->query->query_string);
//	$AC->log->write("ENCODING is set to UTF-0 but the content type of the request header is not", $AC->log->levels['INFO'], SCRIPT_AJAX);
}
//else	$AC->log->write("ENCODING is ok", $AC->log->levels['INFO'], SCRIPT_AJAX);

// Convert query to encoding specified in autocomplete_config (encoding of the query string must be the same as for the collection)
//$AC->query->query_string = $AC->charset_encode ($AC->query->query_string);

//$AC->log->write("query #" . $AC->query->id . ", request #" . $request_id . " to compute: '" . $AC->query->query_string . "' (type " . $AC->query->query_types . ")", $AC->log->levels['INFO'], SCRIPT_AJAX);
//$AC->log->write("Query to compute: " . $AC->query->query_string . "(#" . $AC->query->id . ")", $AC->log->levels['INFO']);

//$AC->log->write("CONTENT_TYPE: " . $_SERVER['CONTENT_TYPE']);


/**
 * 3. PROCESS QUERY
 */
$AC->add_query($AC->query);


saveTimestamp('before: process()');
$AC->process($NAC);
saveTimestamp('after: process()');



/**
 * 4. "COPY" RESULT FROM PHP TO JAVASCRIPT (code will be eval'd on client)
 */

// VERY IMPORTANT! request_id must be the first line of response text to assign the response to the right request
echo "request_id=" . javascript_rhs($request_id) . ";\n";
//echo "query_string=" . javascript_rhs($AC->query->query_string) . ";\n";


// NEW 20Oct06 (Holger): used to call with "true" (=urldecode), because javascript would report an error otherwise. But urldecode
// does not deal properly with UTF8 strings. The error was only because of the newlines in $AC->result->facet_boxes, which I now removed.
// However, iso-8859-1 mode we *must* urlencode, otherwise we get a utf8 string on the javascript side and a lot of inverted question marks
// TODO: I still don't understand this last phenomeno -> don't leave it like that, find it out!!
//echo variable_in_javascript($NAC->result, $NAC->result->computation_mask, "", $AC->settings->encoding == "utf-8" ? false : true); //true);  // true = urlencode values, essential!

// Save new query string in the query history array
//$AC->history_navigate($AC->query->query_string, $inactivity);


/**
 * 5. WRITE MODIFIED MEMBERS OF AC OBJECT BACK TO SESSION AC OBJECT
 */
saveTimestamp('before: final session start/commit()');
// Set the session name to the application name
session_name($config->session_name);

// Initialze session
session_start();

$AC->log->write("Session started", $AC->log->levels['DEBUG'], SCRIPT_AC_INIT, $AC->query->request_id, __FUNCTION__, __LINE__);


if ($_SESSION['AC']->query->id <= $NAC->query->id)
{
	// NEW 31.01.07 (Markus): write only changed members of AC object back
	update($_SESSION['AC'], $NAC, true);

	$AC->log->write("Wrote back to session: query_id=" . $_SESSION['AC']->query->id . " <= current query id=" . $NAC->query->id . ", query_string=" . $_SESSION['AC']->query->query_string, $AC->log->levels['DEBUG'], SCRIPT_AJAX, $AC->query->request_id, __FUNCTION__, __LINE__);
}
else {
//	echo("ajax.php: session not wrote back: current session id=" . $_SESSION['AC']->query->id . " > current query id=" . $NAC->query->id);
	$AC->log->write("Session not wrote back: current session id=" . $_SESSION['AC']->query->id . " > current query id=" . $NAC->query->id, $AC->log->levels['INFO'], SCRIPT_AJAX, $AC->query->request_id, __FUNCTION__, __LINE__);
}


//echo variable_in_javascript($AC->history, array_keys(get_object_vars($AC->history)), "\thistory.");
//echo variable_in_javascript2($AC, "history", "", false);

// Copy the AC->js_text array to javascript
// This handling is a little bit special because we send data which not belongs to the AC result object but to the AC object directly
if ($language_changed) {
	foreach ($AC->js_text as $key => $value) echo "AC.text['$key'] = " . javascript_rhs($AC->get_text($key)) . ";\n\t";
}

// NEW Markus / 27-06-09: new debug concept
//$AC->log->write("$last_qid == " . $AC->query->id, $AC->log->levels['INFO'], SCRIPT_AJAX, $AC->query->request_id, __FUNCTION__, __LINE__);
//if ($last_qid == $AC->query->id)
if (($log->target == $log->targets['SESSION'] || $log->target == $log->targets['FILE_SESSION'])
//if (($log->target == $this->targets['SESSION'] || $log->target == $this->targets['FILE_SESSION'])
      && $last_qid == $AC->query->id)
{
  $_SESSION["DEBUG"] = array_merge($_SESSION["DEBUG"], $__debug);
  $_SESSION["ERROR"] = array_merge($_SESSION["ERROR"], $__error);
}
else {
  $_SESSION["DEBUG"] = $__debug;
  $_SESSION["ERROR"] = $__error;
}

session_commit();
$AC->log->write("Session committed", $AC->log->levels['DEBUG'], SCRIPT_AC_INIT, $AC->query->request_id, __FUNCTION__, __LINE__);
saveTimestamp('after: final session start/commit()');

echo "navigation_mode=" . javascript_rhs(variable("navigation_mode"), true) . ";\n";

$NAC->result->time_C = round((microtime_float() - $times[0]) * 1000, 2);	// time for processing in ms
echo "time_C=" . javascript_rhs($NAC->result->time_C, true) . ";\n";
saveTimestamp('end of: ajax.php');


echo "times=new Array();";
foreach ($times as $p => $v)
{
	echo "\ntimes[$p]='$v';";
}


/**
 * 6. WRITE ONE-LINE SUMMARY TO ACCESS LOG
 */
$AC->log->write_stats($AC, $NAC);


$AC->log->write("END request #" . $request_id . " (" . floor($NAC->result->time_C) . " ms)", $AC->log->levels['INFO'], SCRIPT_AJAX, $request_id, __FUNCTION__, __LINE__);
$AC->log->write("");
?>
