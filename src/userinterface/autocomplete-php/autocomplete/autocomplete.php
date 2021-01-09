<?php

/**
 * AUTOCOMPLETE.PHP (on the way to becoming identical to ajax.php)
 *
 * All javascript variables are set from PHP session object AC.
 * Perform a AC->process() if a new query is given.
 * Needs: $config->autocomplete_path
 *
 */

// The name of this script (used to identify log message)
//define("SCRIPT_AUTOCOMPLETE", "autocomplete");
$path_info = pathinfo(__FILE__);
define("SCRIPT_AUTOCOMPLETE", $path_info["basename"]);


include_once($_SERVER['DOCUMENT_ROOT'] . "/" . $config->autocomplete_path . "times.php"); // functions and variables for time measurement; set the start timestamp $times[0]

// NEW Markus / 27-06-09: new debug concept
$__debug = array();
$__error = array();

// Provide own error handling and set the system error handling to our handling
include_once($_SERVER['DOCUMENT_ROOT'] . "/" . $config->autocomplete_path . "error_handler.php");
include_once($_SERVER['DOCUMENT_ROOT'] . "/" . $config->autocomplete_path . "helper.php");
include_once($_SERVER['DOCUMENT_ROOT'] . "/" . $config->autocomplete_path . "generate_javascript.php");


/**
 * 1. GET CURRENT AC OBJECT
 */
saveTimestamp('before: include(AC_init.php)');
include_once($_SERVER['DOCUMENT_ROOT'] . "/" . $config->autocomplete_path . "/AC_init.php");
saveTimestamp('after: include(AC_init.php)');


$AC->log->write("AT THE BEGINNING OF autocomplete.php : \"" .  $AC->query->query_string . "\"", $AC->log->levels['DEBUG'], SCRIPT_AUTOCOMPLETE, "", __FUNCTION__, __LINE__, $AC->log->targets["FILE"]);

echo '<link rel=stylesheet type="text/css" href="' . get_full_url($AC->settings->autocomplete_path) . 'logging.css">';


// NEW (Markus): Check whether cookies are enabled
if (isset($_COOKIE["cookie_test"]) && $_COOKIE["cookie_test"] == "enabled")
{
	// NEW 26.02.07 (Markus)
	if (! $AC->cookies)
	{
		$AC->cookies = true;
		$NAC->cookies = true;
    $AC->log->write("Cookies are enabled", $AC->log->levels['DEBUG'], SCRIPT_AUTOCOMPLETE, "", __FUNCTION__, __LINE__);
	}
}
else {
	// NEW 26.02.07 (Markus)
	if ($AC->cookies)
	{
		$AC->cookies = false;
		$NAC->cookies = false;
	}
	// Try to set a test cookie to see with the next request whethter the cookie was set and send
	setcookie("cookie_test", "enabled");
}

// Take the query history from the AC object
// Note: It's necessary to set the history member in $NAC because of the session AC object update
//  (only what changed in $NAC is also changed in the session AC object).
//  To set $NAC->history as alias (reference) to $AC->history is sufficiant for the update mechanism to recognize that history is changed.
$NAC->history = &$AC->history;


// NEW 21.03.07 (Markus): try without the following code
// We don't know whether javascript is enabled or not, so assume the worst case
//if ($AC->javascript)
//{
//	$AC->javascript = false;
//	$NAC->javascript = false;
//}


/**
 * 2. SET QUERY (from url)
 */

// NEW 26-02-08 (Markus): URL parameter name changed from "autocomplete_query" to "query"
if (array_key_exists("query", $_GET)) {
  $AC->set_parameter_from_url($AC->query->query_string, "query");
}
else {
  $AC->set_parameter_from_url($AC->query->query_string, "");
}

// NEW 14-08-08 / Markus: if the query parameters (the part after "qp=") is empty set $query_parameters to the default query string representation)
$query_parameters = variable("qp") == "" ? $AC->getDefaultQueryParametersAsString() : variable("qp");

$AC->log->write ("BEGIN request" . " (query #" . $AC->query->id . "): '" . $AC->query->query_string . "' (type " . $AC->query->query_types . ") fh: " . $AC->query->first_hit, $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);
$AC->log->write ($AC->query, $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__, "Starting with ");


/**
 * 3. PROCESS QUERY
 */
//if ($AC->state == "changed")
{
	$AC->log->write ("State: **{$AC->state}**  " . " (query #" . $AC->query->id . "): '" . $AC->query->query_string . "' (parameters " . $query_parameters . ")", $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);

	
	$NAC->result = new Result();

	if ($AC->query->query_string != $AC->last_query_string)
	{
	  // Markus 27-06-09: why here "reset" but for NAC above "new Result()"?
		$AC->result->reset();
		$AC->query->first_hit = 1;
		$AC->first_hit = 1;
		$AC->last_hit = 1;
	}

//	echo "qp=".$query_parameters;
  // NEW Markus / 07-01-09
	preg_match_all("/([" . $AC->settings->query_types . "])(\d+)\.(\d+)\.?(\d*)/", $query_parameters, $matches);
	
	// For every matched type in the query parameters add a separate query  
	for ($i = 0; $i < sizeof($matches[0]); $i++)
	{
	  // TODO welches ist die bessere Möglichkeit?
//		$q = unserialize(serialize($AC->query));
    $q = new Query();
    $q->initialize($AC);	
    $q->query_string = $AC->query->query_string; 
		// So klappt es ohne JS nach Reset in Opera und Google Chrome nicht:
//    $q->query_string = $AC->last_query_string; 

	  $q->query_types = $matches[1][$i];
	  $q->query_index = $matches[2][$i];
	  $q->max_completions_show = $matches[3][$i];
	  
	  // Parameter 4 for first_hit is optional
	  if ($matches[4][$i]) {
	  	$q->first_hit = $matches[4][$i];
  	  $q->first_hit_shown = $matches[4][$i];
  	  // NEW Markus / 06-01-09: For a H query (hits) we have to set the first_hit member of AC additionally
  	  if ($q->query_types == "H") {
	      $AC->first_hit = $matches[4][$i];
	    }
	  }
	  else {
	  	$q->first_hit = 1;
  	  $q->first_hit_shown = 1;
  	  // NEW Markus / 06-01-09: For a H query (hits) we have to set the first_hit member of AC additionally
  	  if ($q->query_types == "H") {
	      $AC->first_hit = 1;
	    }
	  }
	  
		$AC->add_query($q);
	}

	saveTimestamp('before: process()');
	$AC->process($NAC);
	saveTimestamp('after: process()');

	$NAC->state = "changed";

	// If the query string ist empty we get the count of total documents; store it in the AC object
  if ($AC->query->query_string == ""){
    $NAC->documents_count = $NAC->result->H_boxes[1]["total"];
  }
}
/*
else if ($AC->state == "start")
{
  if (! isset($AC->documents_count))
  {
    // Compute a empty query to get the number of documents
    $AC->log->write("State: **{$AC->state}** compute an empty query to get the number of documents", $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);

    $q = new Query();
    $q->initialize($AC);	 
    $q->query_types = "H";
    $q->query_string = "*";
    $q->hits_per_page_while_typing = 0;

    $AC->add_query($q);
    
    // If facets (F boxes) should be computed for empty query, too, add a F query
    if ($AC->settings->show_facets_for_empty_query)
    {
      $q = new Query();
      $q->initialize($AC);	 
      $q->query_types = "F";

      $AC->add_query($q);
    }
    
//		$AC->query->query_string = "";
//		$AC->add_query($AC->query);

    $AC->log->write(print_r($AC->queries, true), $AC->log->levels['DEBUG'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);

    $AC->process($NAC);
	  
    $NAC->documents_count = $NAC->result->H_boxes[1]["total"];
		$AC->log->write("Number of documents: $NAC->documents_count", $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);
    $NAC->state = "changed";
  }
}
*/
		
$AC->log->write("Number of documents: $NAC->documents_count", $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);

// If AC object was changed before update it now
if ($NAC->state == "changed")
{
  // Update the AC object because we need it for the index.php result div's
  $AC->log->write ("Updating AC->object->result:", $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);
  update($AC->result, $NAC->result);

  /**
 	 * WRITE MODIFIED PARTS OF NAC OBJECT BACK TO SESSION AC OBJECT
 	 */
  // Set the session name to the application name
  session_name($AC->settings->session_name);
  saveTimestamp('before: final session start/commit()');

  // Initialize session
  session_start();

  
  $AC->log->write ("Updating sessions's AC->object:", $AC->log->levels['DEBUG'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);
  update($_SESSION['AC'], $NAC);

  // NEW Markus / 27-06-09: new debug concept
  $_SESSION["ERROR"] = $__error;
  $_SESSION["DEBUG"] = $__debug;
  
  session_commit();
  saveTimestamp('after: final session start/commit()');

  $NAC->result->time_C = round((microtime_float() - $times[0]) * 1000, 2);	// time for processing in ms

  $AC->log->write_stats($AC, $NAC, SCRIPT_AUTOCOMPLETE, $AC->query->request_id);

  // Add a newline in access.log
  $AC->log->write("END request" . " (query #" . $AC->query->id . ") " . "(" . floor($NAC->result->time_C) . " ms)", $AC->log->levels['INFO'], SCRIPT_AUTOCOMPLETE, $AC->query->request_id, __FUNCTION__, __LINE__);
  $AC->log->write("");
}


/**
 * 4. "COPY" RESULT FROM PHP TO JAVASCRIPT (code will be eval'd on client)
 */
  if (! isset($config->javascript) || $config->javascript)
  {
?>
  <!-- Include the javascript parts of autocomplete functionality -->
  <script type="text/javascript" src="<?php echo get_full_url($AC->settings->autocomplete_path); ?>logging.js"></script>
  <script type="text/javascript" src="<?php echo ($AC->settings->autocomplete_url); ?>autocomplete.js"></script>
<!--  <script type="text/javascript" src="<?php echo get_full_url($AC->settings->autocomplete_path); ?>autocomplete.js"></script>-->
  <script type="text/javascript" src="<?php echo get_full_url($AC->settings->autocomplete_path); ?>utils.js"></script>
  <script type="text/javascript" src="<?php echo get_full_url($AC->settings->autocomplete_path); ?>history.js"></script>
  <script type="text/javascript">
  <!--
    var AC = new AC_Class("AC");

<?php
    echo variable_in_javascript($AC->settings, array_keys(get_object_vars($AC->settings)), "\tAC.");
  
    // The following leads to errors because values of AC->settings are overridden
    // If we need members of query we should do this explicitly
    // echo variable_in_javascript($AC->query, array_keys(get_object_vars($AC->query)), "\tAC.");
  
    // NEW 29.10.06 (Markus)
    // Ok, we do it explicitly for first_hit, because we need to know about it when we come from options or back from an external link
    // Note: this is a redeclaration, the previous value set by $AC->settings is overridden
    echo "\tAC.first_hit = " . javascript_rhs($AC->query->first_hit) . ";";
    // NEW 06-12-07 (Markus)
    echo "\tAC.first_hit_shown = " . javascript_rhs($AC->query->first_hit_shown) . ";";
  
    echo "\tAC.javascript = " . javascript_rhs($AC->javascript) . ";";
    echo "\tAC.cookies = " . javascript_rhs($AC->cookies) . ";";
  
    // If we don't do the following the query_id counter begins at zero with every call of index.php (refresh or return)
    // It's not so nice but is it an problem?
    echo "\n\tAC.query_id=" . javascript_rhs($AC->query->id) . ";";
    echo "\n\tAC.documents_count=" . javascript_rhs($AC->documents_count) . ";";
  
    echo "\n";
    // NEW 16-07-07 (Markus): deliever the history to javascript
    if (isset($AC->history))
      echo variable_in_javascript($AC->history, array_keys(get_object_vars($AC->history)), "\tAC.history.");
  
    // The log object containing current log level, possible levels etc.
    echo "\tAC.log = new Object();\n";
    echo variable_in_javascript($AC->log, array_keys(get_object_vars($AC->log)), "\tAC.log.");
  
    echo "\tAC.text = new Object();";
    // Copy the AC->js_text array to javascript
    foreach ($AC->js_text as $key => $value) echo "AC.text['$key'] = " . javascript_rhs($AC->get_text($key)) . ";\n\t";
  
    echo "\n\tAC.note = new Object();";
    // Copy the AC->note array to javascript
    foreach ($AC->note as $key => $value) echo "\n\tAC.note.$key = \"$value\";";
  
    echo "\n";
    // NEW 02.11.06 (Markus): copy all members of AC->result to javascript AC.result object
    // This is used for navigating through completions, for example.
    if (isset($AC->result)) {
      echo variable_in_javascript($AC->result, array_keys(get_object_vars($AC->result)), "\tAC.result.");
    }
?>

    AC.result.completion_index = AC.result.completion_start_index;
    //-->
    </script>
<?php
    }
?>
