<?php

/**
 * Created on 21.02.2006
 * Author: Markus Tetzlaff
 *
 */

// The name of this script (used to identify log message)
$path_info = pathinfo(__FILE__);
define("SCRIPT_AC", $path_info["basename"]);

include_once("settings.php");
include_once("query.php");
include_once("result.php");
//include_once("log.php");  // This include is now in AC_init.php
include_once("hit.php");
include_once("completion.php");
include_once("xml_parser.php");

/**
 * Class for all autocomplete functionality
 *
 */
class AC
{
	// The settings: one default settings and the currently used settings
	var $default_settings;
	var $settings;

	// Query parameters: are passed to the autocomplete server with the query
	var $query;
	// NEW 24-02-08 (Markus)
	var $queries;

	// Result, contains all information we got from autocomplete server
	var $result;

	// Logging functionality
	var $log;

	var $history;

	var $version;            // Version of the completion server application
	var $text;               // Array for the text patterns defined in text.php
	var $js_text;            // Array for the text patterns used in the javascript part (defined in text.php)
	var $state;              // State of autocompletion computation, "changed" for example
	var $cookies;            // Type boolean; true = cookies enabled, false = cookies disabled
	var $javascript;         // Type boolean; true = javascript enabled, false = javascript disabled
	var $note;               // This is an array whcih determine whether the user is informed about certain things like disabled javascript functionality
	var $user_agent;         // User agent of request (information about client/browser)
	var $first_hit;          // The first hit is shown (this is different from the first_hit of the query object)
	var $last_hit;           // The last hit is shown (this is different from the last_hit of the query object)
	var $last_query_string;  // The search string of the last query
	var $documents_count;    // The number of all documents in the database
	var $errors;
        
        // NEW 29 Okt 2010 (Björn): 
        // Added a member that remembers translated Queries
        var $translated_query;


	/**
	 * Constructor for AC class
	 *
	 * @return AC
	 */
	function AC()
	{
	}


	/**
	 * Initialize the AC object
	 *
	 */
	function initialize()
	{
    //$this->version = VERSION;
		// Because we don't know at the beginning whether javascript is enabled or not, we assume the worst case
		$this->javascript = false;
		
		$this->note = array();
		$this->note["javascript"] = 1;
		$this->note["quirks_mode"] = 1;
		
		$this->default_settings = new Settings();
		$this->default_settings->initialize();
		$this->settings = new Settings();
//		$this->settings->initialize();
		$this->query = new Query();
		// NEW 24-02-08 (Markus)
		$this->queries = array();
		$this->result = new Result();
		$this->log = new Log($this);
		// Error codes
		$this->errors = array("OPEN_SOCKET_FAILED" => 1, "SOCKET_TIMEOUT" => 2);
//		$this->history = new History();
		// CHANGED 26-02-08 (Markus)
		$this->first_hit = 1;
                //		$this->last_query_string = "undefined";
                
                // NEW 29 Okt 2010 (Björn):
                // Also initializes the new member that rembers query 
                // translations.
                $this->translated_query = array();
                $this->translated_query['original'] = "";
                $this->translated_query['translated'] = "";
}


	// NEW 24-02-08 (Markus)
	function add_query($query)
	{
    $this->log->write("Add query: ", $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
    $this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$query");
		$this->queries[] = $query;
	}

/*	
	function clear_queries()
	{
	  $this->queries = array();
	  $this->log->write("Queries cleared", $this->log->levels['DEBUG'], SCRIPT_AC, null, __FUNCTION__, __LINE__);
//	  $this->log->write("Queries cleared", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	}
*/
	
  //
	// METHODS FOR PROCESSING A QUERY (Holger moved them from below)
  //
  //

	/**
	 * Process query for each letter of query_types string, call corresponding process function, e.g., process_A(), process_B(), ...
	 * Every call of process() changes the current AC object (which is stored in the session);
	 * NAC object holds these changements
	 *
	 * @param AC $nac the instance of the AC object which contains all changements in compairison to the session AC object.
	 */
	function process(& $nac)
        {
                $this->log->write("Process called: " . print_r($this, true), $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"], "\$this");

    // Unset error message due to errors of past calls
		$nac->result->error_message = "";

		if ($this->log->level >= $this->log->levels['WARNING'] && count($this->queries) == 0) {
		  $this->log->write("Method " . __FUNCTION__ . "() is called without any query (there should be one query at least)", $this->log->levels['WARNING'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
		}

		$nac->query = unserialize(serialize($this->query));
		// sort ($this->queries);
		// NEW 24-02-08 (Markus)
		foreach ($this->queries as $query)
		{
		  for ($i = 0; $i < strlen($query->query_types); $i++)
		  {
		    $method_name = 'process_' . $query->query_types[$i];
		    $this->log->write("PROCESS METHOD : $method_name", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
		    if (method_exists($this, $method_name))
		    {
		      // TMP: time measurement
		      saveTimestamp('before: ' . $method_name . "()");
		      eval('$this->' . $method_name . '($query, $nac);');
		      // TMP: time measurement
		      saveTimestamp('after: ' . $method_name . "()");

          $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "[" . $query->query_types[$i] . "] \$nac");
		    }
		    else {
		      $this->log->write("INVALID PROCESS METHOD : $method_name", $this->log->levels['WARNING'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
		    }
		  }
		}
		
		// If we have got an error message but no error message title is given we use the standard error message title text
		if ($nac->result->error_message != "" && $nac->result->error_message_title == "") {
			$nac->result->error_message_title = $this->get_text("error_message_title");
		}
		else {
			// Update some members according to successful processing
			$nac->last_query_string = $nac->query->query_string;
			$this->state = "ok";
		}
	}

	

  /**
   * Processing function H: find hits for query
   *
   * @param Query $ac_query
   * @param AC $nac
   */
	function process_H($ac_query, & $nac)
	{
    $this->log->write($ac_query, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$ac_query");

    // We have to work on a copy of ac_query; since PHP5 $ac_query is passed by reference, so we have to do this explicitly
    $query = unserialize(serialize($ac_query));
    
    
    // NEW 29 Okt 2010 (Björn): Use the translated query if there is one
    // NEW 14Aug12 (baumgari): On reset the index 'translation' doesn't exist 
    // and leads to a php error. Therefore check its existence first.
    if ($this->translated_query['original'] == $query->query_string && array_key_exists('translation', $this->translated_query)) {
      $query->query_string = $this->translated_query['translation'];
      // $this->log->write("Use translation instead!" , $this->log->levels['INFO'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"], "\$ac_query"); 
    }

    $this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$query");
	  $this->log->write("Query H : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);

	  // For testing only: provocate an error by using an wrong port
	  //		if (preg_match("|fux$|", $query->query_string))
	  //		{
	  //			$tmp_port = $this->settings->server_port;
	  //			$this->settings->server_port = "19011";
	  //		}

	  // This array declares all members of result which should be computed
	  $compute = array("hits_computed", "options_title");

		// NEW 12-02-08 / 10-04-08 (Markus): new query concept, hit box is treated as other boxes too, means that hits_per_page_while_typing is determined by max_completions_show
    //$query->hits_per_page_while_typing = $query->max_completions_show;
		// NEW 16-07-08 (Markus): this new query concept (the line above) leads to an error when computing non-javascript query
		//  (where all query types are computed within **one** request): there we can not use parameter mcs for W/F/C boxes and H box at the same time
		//  So we give hits_per_page_while_typings priority against max_completions_show
		//echo "hpp={$query->hits_per_page_while_typing}, mcs={$query->max_completions_show}";
//		if (! isset($query->hits_per_page_while_typing)) {
		  $query->hits_per_page_while_typing = $query->max_completions_show;
//		}

		// Don't compute completions
    $query->max_completions_show = 0;

		// NEW 17Nov10 (Hannah): For HomeoVim, append ct:datum:*, so that we can sort by date.
		if ($this->settings->session_name == "homeovim")
		{
		  if ($query->query_string != "") $query->query_string .= " ";
		  $query->query_string .= "ct:datum:*";
		}

	  // NEW 11May07 (Holger): deal with synonyms here
	  $SEP = $this->settings->separators;
	  if ($this->settings->synonym_mode == 1)
	  {
	    $query->query_string = preg_replace_callback("|([^$SEP]+)~|", array($this, "rewrite_tildas"), $query->query_string);
	  }
	  else if ($this->settings->synonym_mode == 2)
	  {
	    $query->query_string = preg_replace_callback("|([^$SEP]+)|", array($this, "rewrite_tildas"), $query->query_string);
	  }

	  // Process the query
	  $result = $this->process_single($query);

	  /* BEGIN Set result
	  */
	  // Set all result member of the AC object (declared to be computed above) to the corresponding values of the temporary $result
	  foreach ($compute as $computed)
	  {
	    $nac->result->$computed = $result->$computed;
	  }

	  // NEW 12-12-07 (Markus): the index of result::last_hit begins with zero, the one of AC::last_index with 1
	  $nac->last_hit = $result->last_hit;

	  // NEW 11-04-07 (Markus)
	  if ($result->error_message != "") {
	    //		$nac->result->error_message_title = $result->error_message_title;
	    $result->error_message_title = $this->get_text('completions_title_on_error');
	    $result->error_message = $result->error_message;
	  }

	  // The *_box(es) members needs special processing because they are computed by build_*_box(es) from the corresponding result member
    // NEW 03-04-08 (Markus): the result box arrays gets now a numeric key i which indicates that the
    //  result belongs to the i-th result box
    //  (for a query of a type which has only one box (W,C for example) this index is always "1")
	  $nac->result->H_boxes[1] = $this->build_H_boxes($result, $query);
	  // NEW 25Jul07 (Holger): if hit box has already been produced by Y query, don't overwrite it (TODO: this is a quick hack)
	  //    if (!preg_match("|yyy=\"Y\"|", $nac->result->hit_box["body"]))
	  //    {
	  //      $nac->result->hit_box = $this->build_hit_box($result, $query);
	  //    }

	  // TODO Here we need a merge
	  // Add times, size and error_message of this call to the overall values for this process() call
	  $nac->result->time_A += $result->time_A;
	  $nac->result->time_B += $result->time_B;
	  $nac->result->response_size += $result->response_size;

	  // Indicator whether continue processing in case of errors
	  $nac->result->do_continue = $result->do_continue;
	  /* END Set result
	  */

	  // For testing only: restore manipulated port
	  // $this->settings->server_port = $tmp_port;

    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
	}


  /**
   * Processing function W: find completions for word query
   *
   * @param Query $ac_query
   * @param AC $nac
   */
  function process_W($ac_query, & $nac)
  {
    $this->log->write($ac_query, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$ac_query");

    if (! empty($ac_query->query_string))
    {
      // We have to work on a copy of ac_query; since PHP5 $ac_query is passed by reference, so we have to do this explicitly
      $query = unserialize(serialize($ac_query));

      // Don't compute hits
      $query->hits_per_page_while_typing = 0;
      $query->hits_per_page_on_click = 0;

      $this->log->write("Query W : \"" . $query->query_string . "\", fh=" . $query->first_hit, $this->log->levels['INFO'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['FILE']);
      $this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$query");

      // This array declares all members of result which should be computed
      // TODO is this still used?
      //	  $compute = array("completions", "completion_start_index", "completions_sent");

      // NEW 11May07 (Holger): deal with synonyms here
      $SEP = $this->settings->separators;

      if ($this->settings->synonym_mode == 1)
      {
        $query->query_string = preg_replace_callback("|([^$SEP]+)~|", array($this, "rewrite_tildas"), $query->query_string);
      }
      else if ($this->settings->synonym_mode == 2)
      {
        $query->query_string = preg_replace_callback("|([^$SEP]+)|", array($this, "rewrite_tildas"), $query->query_string);
      }

      // Process the query
      $result = $this->process_single($query);

      /* BEGIN Set result
      */
      // Set all result member of the AC object (declared to be computed above) to the corresponding values of the temporary $result
      // TODO is this still used?
      //	  foreach ($compute as $computed)
      //	  {
      //	    $nac->result->$computed = $result->$computed;
      //	  }

      // NEW 11-04-07 (Markus)
      if ($result->error_message != "") {
        $result->error_message_title = $this->get_text('completions_title_on_error');
        $result->error_message = $result->error_message;
      }

      // The *_box(es) members needs special processing because they are computed by build_*_box(es) from the corresponding result member
      // NEW 17-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
      //  (for a query of a type which has only one box (W,C for example) this index is always "1")
      $nac->result->W_boxes[1] = $this->build_W_boxes($result, $query);

      // NEW 25Jul07 (Holger): if hit box has already been produced by Y query, don;t overwrite it (TODO: this is a quick hack)
      //	  if (!preg_match("|yyy=\"Y\"|", $nac->result->hit_box["body"]))
      //	  {
      //	    $nac->result->hit_box = $this->build_hit_box($result, $query);
      //	  }

      // Add times, size and error_message of this call to the overall values for this process() call
      $nac->result->time_A += $result->time_A;
      $nac->result->time_B += $result->time_B;
      $nac->result->response_size += $result->response_size;

      // NEW 11.10.06 (Markus): indicator whether continue processing in case of errors
      $nac->result->do_continue = $result->do_continue;
      /* END Set result
      */
    }

    else {
      // Even if we did not compute word completions we have to deliver the W boxes array (with empty arrays as elements)
      // to tell the index.php how many W boxes are to construct at beginning or after reset
      $nac->result->W_boxes[1] = array();
      $this->log->write("No request sent to server because of empty query", $this->log->levels['INFO'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__);
    }

    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
  }


  /**
   * Processing function C: find matching category names replace last word w by cn:w)
   *
   * @param Query $ac_query
   * @param AC $nac
   */
	function process_C($ac_query, & $nac)
	{
    // We have to work on a copy of ac_query; since PHP5 $ac_query is passed by reference, so we have to do this explicitly
		$query = unserialize(serialize($ac_query));

		// NEW 20.12.06 (Markus): First check whether the query string has the minimal size
		if ($query->query_string != "" && strlen($query->query_string) < $this->settings->min_query_size)
		{
			$result->subtitle = sprintf ($this->get_text("query_too_short"), $this->settings->min_query_size);
			return array();
		}

		$SEP = $this->settings->separators;
		if (preg_match("|[^$SEP]$|", $query->query_string) && !preg_match("|c[nts]:[^$SEP]+$|", $query->query_string))
		{
			// Prepare the query
			$query->query_string = preg_replace("|([^$SEP]+)$|", "cn:$1", $query->query_string);
			$query->hits_per_page_while_typing = 0;

			// Process the query
			$result = $this->process_single($query);

			/* BEGIN Set result
			*/
			// The *_box(es) members needs special processing because they are computed by build_*_box(es) from the corresponding result member
			// Note: he category box use the same box structure as the completion box
      // NEW 17-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
      //  (for a query of a type which has only one box (W,C for example) this index is always "1")
			$nac->result->C_boxes[1] = $this->build_C_boxes($result, $query);
//			$nac->result->C_boxes = $this->build_C_boxes($result, $query);

			// Add times, size and error_message of this call to the overall values for this process() call
			$nac->result->time_A += $result->time_A;
			$nac->result->time_B += $result->time_B;
			$nac->result->response_size += $result->response_size;
			if ($result->error_message != "") {
				$nac->result->error_message .= $result->error_message . "<br>";
			}
			// NEW 11.10.06 (Markus): indicator whether continue processing in case of errors
			$nac->result->do_continue = $result->do_continue;
			/* END Set result
			*/
		}
		else
		{
			$nac->result->C_boxes[1] = array();
			$this->log->write("QUERY C : nothing to do", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
		}

    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
	}


  /**
   * Processing function F: find refinements for faceted search (just append ct:)
   *
   * @param Query $ac_query
   * @param AC $nac
   */
  function process_F($ac_query, & $nac)
  {
    $this->log->write("QUERY F: " . $ac_query->query_string, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['FILE']);
    $this->log->write($ac_query, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$ac_query");
    
    // We have to work on a copy of ac_query; since PHP5 $ac_query is passed by reference, so we have to do this explicitly
    $query = unserialize(serialize($ac_query));

    // Don't compute hits
    $query->hits_per_page_while_typing = 0;    
    $query->hits_per_page_on_click = 0;    

    $this->log->write("Query F : \"" . $query->query_string . "\", fh=" . $query->first_hit, $this->log->levels['INFO'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['FILE']);
    $this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$query");

    // NEW 20.12.06 (Markus): First check whether the query string has the minimal size
    if ($query->query_string != "" && strlen($query->query_string) < $this->settings->min_query_size)
    {
      $result->subtitle = sprintf ($this->get_text("query_too_short"), $this->settings->min_query_size);
      return;
    }

    // Add a space before ct: only for non-empty query
    // Save this for reuse in every round (was a bug before, kept on appending)
    $query_tmp = ($query->query_string == "" ? "ct:" : $query->query_string . " ct:");

    $i = 1;
    foreach ($this->settings->facets_to_show as $facet)
    {
      // If a query index is given (means don't equal to "") we only do the iteration step belonging to this query index.
      // For all other facettes we continue the loop by going to the next iteration step
      if ($query->query_index != "" && $query->query_index != $i) {
        $i++;
        continue;
      }

      // Proceed with processing only for non-empty query or if facets should be shown for empty query
      if (strlen($query->query_string) > 0 || $this->settings->show_facets_for_empty_query == true)
      {
        // Prepare the query
        $query->query_string =  $query_tmp . $facet . ($facet == "" ? "*" : ":*");

        // $this->log->write("Query F [" . $facet . "] : \"" . $query->query_string . "\" mcs=" . $query->max_completions_show, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id);

				// Process the query.
				// TODO: Why do we have to save these query parameters and restore them?
				// HACK(Hannah 22Mai11): Hard-code sorting for certain facets in HomeoVim Search (for Tjado).
        $max_completions_show_tmp = $query->max_completions_show;
				$how_to_rank_words_tmp = $query;
				if ($facet == "monat" || $facet == "quartal" || $facet == "jahr" || strtoupper($facet) == "YEAR") $query->how_to_rank_words = "3d";
				if ($facet == "monat" || $facet == "quartal" || $facet == "jahr") $query->max_completions_show = 500;
        $result = $this->process_single($query);
        $query->max_completions_show = $max_completions_show_tmp;
				$query->how_to_rank_words = $how_to_rank_words_tmp;

        /* BEGIN Set result
        */
        if ($result->error_message != "")
        {
          $result->error_message_title = $this->get_text('facets_title_on_error');
          $result->error_message = $result->error_message;
        }
        // The result box arrays gets a numeric key i which indicates that the
        //  result belongs to the i-th result box
        //  (for a query of a type which has only one box (W,C for example) this index is always "1")
        //  Note: works only if facets_to_show doesn't equal to "",
        //  e.g. we iterate over an array of predefined facets and build_F_boxes() delivers an array of length = 1

        // NEW 04-03-08 (Markus): To prevent the notice "Only variables should be passed by reference"
        $tmp = $this->build_F_boxes($result, $query, $i);
        $nac->result->F_boxes[$i] = array_pop($tmp);

        // Add times, size and error_message of this call to the overall values for this process() call
        $nac->result->time_A += $result->time_A;
        $nac->result->time_B += $result->time_B;
        $nac->result->response_size += $result->response_size;

        // Indicator whether continue processing in case of errors
        $nac->result->do_continue = $result->do_continue;
        /* END Set result
        */
      }
      else {
        // Even if we did not compute facets we have to deliver the F boxes array (with empty arrays as elements)
        // to tell the index.php how many F boxes are to construct at beginning or after reset
        $nac->result->F_boxes[$i] = array();
//        $this->log->write("QUERY F : no request sent to server because of empty query", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
        $this->log->write("No request sent to server because of empty query", $this->log->levels['INFO'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__);
      }
      $i++;
    }
  
    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
  }


	/**
   * Processing function J: deal with *open* join queries like "ct:auth[ct:sigmod ct:sigi"
	 *
	 * @param Query $ac_query
	 * @param AC $nac
	 */
	function process_J($ac_query, & $nac)
	{
		// We have to work on a copy of ac_query
		$query = unserialize(serialize($ac_query));

		// NEW 20.12.06 (Markus): First check whether the query string has the minimal size
		if ($query->query_string != "" && strlen($query->query_string) < $this->settings->min_query_size)
		{
			$result->subtitle = sprintf ($this->get_text("query_too_short"), $this->settings->min_query_size);
			return;
		}

		// This array declares all members of result which should be computed
		$compute = array("join_box");

		// First, same test as in query->query_for_completion_server
		// TODO: avoid doing it twice
		$pos_beg = strrpos($query->query_string, "[");
		$pos_end = strrpos($query->query_string, "]");

		// If open join bracket, just close it
		if ($pos_beg !== false && ($pos_end === false || $pos_end < $pos_beg))
		{
			// Prepare the query
			$SEP = "\\[\\]. ";
			$query->query_string .= "]";	//$this->query->query_string = preg_replace("/([^$SEP]+)$/", "$1] $1", $this->query->query_string);
			$query->hits_per_page_while_typing = 0;
			//$query->max_completions_show = 5;

			$this->log->write("Query J : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);

			// Process the query
//			usleep(100000);
			$result = $this->process_single($query);

			/* BEGIN Set result
			*/
			// Set all result member of the AC object (declared to be computed above) to the corresponding values of the temporary $result
			foreach ($compute as $computed)
			{
				$nac->result->$computed = $result->$computed;
			}

			// The *_box(es) members needs special processing because they are computed by build_*_box(es) from the completion member
			$join_attr = preg_match("/([^$SEP]+)\[[^[]+$/", $query->query_string, $matches) ? $matches[1] : "join attr.";
			// Note: the join box use the same box structure as the completion box
			$nac->result->J_boxes = preg_replace('/( "[^"]*" )/', " \"" . $join_attr . "\" ", $this->build_J_boxes($result, $query));

			// Add times, size and error_message of this call to the overall values for this process() call
			$nac->result->time_A += $result->time_A;
			$nac->result->time_B += $result->time_B;
			$nac->result->response_size += $result->response_size;
			$nac->result->error_message_title = $result->error_message_title;
			if ($result->error_message != "") {
				$nac->result->error_message .= $result->error_message . "<br>";
			}
			// NEW 11.10.06 (Markus): indicator whether continue processing in case of errors
			$nac->result->do_continue = $result->do_continue;
			/* END Set result
			*/
		}
		else
		{
			$this->log->write("Query J : nothing to do", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
		}

    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
	}


	/**
	 * Processing function Y: yago query
	 * TODO currently (mis)using category_box !!!
	 *
	 * @param Queery $ac_query
	 * @param AC $nac
	 */
  function process_Y($ac_query, & $nac, $mode = "ONE_HIT_PER_COMPLETION")
	{
	  // We have to work on a copy of ac_query
	  $query = unserialize(serialize($ac_query));

	  $compute = array();

	  $query_words = preg_split("|([ .]+)|", $query->query_string, -1, PREG_SPLIT_DELIM_CAPTURE);
	  $hits_per_page_while_typing_original = $query->hits_per_page_while_typing;
	  $max_completions_show_original = $query->max_completions_show;
	  $query_string_original = $query->query_string;
	  /*
	  // this replaced every prefix by a join query if it was the beginning of a unique class name

	  for ($i = 0; $i < sizeof($query_words); $i += 2)
	  {
	  $query->query_string = "ch:" . $query_words[$i];
	  $query->hits_per_page_while_typing = 0;
	  $query->max_completions_show = 1;
	  $this->log->write("Query Y1 : \"" . $query->query_string . "\"", $this->log->levels['INFO']);
	  $result = $this->process_single($query);
	  if ($result->completions_sent >= 1)
	  {
	  $ch_parts = explode(":", $result->completions[1]);
	  $sub_category = $ch_parts[1];
	  $super_category = $ch_parts[2];
	  $query_words[$i] = $sub_category == $super_category
	  ? "ce:" . $super_category
	  : "[ce:" . $super_category . ":*#type:" . $sub_category . ".ce:" . $super_category . ":*]";
	  //$query_words[$i] = "[ce:" . $category . ":*#type:" . $query_words[$i]  . ".ce:" . $category . ":*]";
	  }
	  }
	  $query->query_string = implode("", $query_words);
	  */

	  // CHECK WHETHER COMPLETION OF ch:<last_prefix> EXISTS

	  $last_prefix_of_query = array_pop($query_words);
	  $last_prefix_of_query_original = $last_prefix_of_query;
	  $last_separator_of_query = array_pop($query_words);
	  $first_part_of_query = implode("", $query_words);

	  $query->query_string = "ch:" . $last_prefix_of_query;
	  $query->hits_per_page_while_typing = 0;
	  //$query->max_completions_show = 1;
	  $this->log->write("Query Y1 : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	  $result = $this->process_single($query);

		// CHECK WHETHER UNIQUE HIT (either single completion of ch:<last_prefix> or 
		// exact match of ch:<last_prefix>:
		$unique_completion = "";
		if ($result->completions_sent == 1)
	  {
			$unique_completion = $result->completions[0]->string;
		}
		else if ($result->completions_sent > 1) 
		{
			for ($i = 0; $i < sizeof($result->completions); $i++) 
			{
				if (preg_match("/^ch:$last_prefix_of_query:/", $result->completions[$i]->string)) {
					$unique_completion = $result->completions[$i]->string;
			  }
			}
		}
    $this->log->write("Unique completion : \"" . $unique_completion . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
		// If unique match, get query to search for instances
    if ($unique_completion != "") 
	  {
	    $ch_parts = explode(":", $unique_completion);
//	    $ch_parts = explode(":", $result->completions[1]);
	    $sub_category = $ch_parts[1];
	    $super_category = $ch_parts[2];
	    $sub_category_alphnum_only = preg_replace("|_|", "", $sub_category);
	    $super_category_alph_num_only = preg_replace("|[\W_]|", "", $super_category);
	    $last_prefix_of_query = $sub_category == $super_category
    	    ? "ce:" . $super_category_alph_num_only
    	    : "[ce:" . $super_category_alph_num_only . ":*#cr:isa.ce:class:"
    	    . $sub_category_alphnum_only . ".;;.ce:" . $super_category_alph_num_only . ":*]";
	  }

	  // Add times, size and error_message of this call to the overall values for this process() call
	  $nac->result->time_A += $result->time_A;
	  $nac->result->time_B += $result->time_B;
	  $nac->result->response_size += $result->response_size;
	  if ($result->error_message != "") {
	    $nac->result->error_message .= $result->error_message . "<br>";
	  }
	  $nac->result->do_continue = $result->do_continue;

	  // CASE 1: non-empty query + prefix completed to more than one class name -> show alternatives
	  if ($query_string_original != "" && $result->completions_sent > 1 && $unique_completion == "")
	  {
	    $query->first_part_of_query = $first_part_of_query;
	    $query->last_separator_of_query = $last_separator_of_query;
	    $query->last_word_of_query .= " (Y --- NOT UNIQUE)";
	    // NEW 07May07 (Holger): rewrite class names in nicer form
	    //            for ($i = 0; $i < sizeof($result->completions); $i++)
	    //            {
	    //$result->completions[$i] = "[" . $result->completions[$i] . "]";
	    //$result->completions[$i] = preg_replace("|\(|", "[", $result->completions[$i]);
	    //            }

      // NEW 22-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
      //  (for a query of a type which has only one box (W,C for example) this index is always "1")
  	  $nac->result->Y_boxes[1] = $this->build_Y_boxes($result, $query);
//	    $nac->result->Y_boxes = $this->build_Y_boxes($result, $query);
//	    $this->log->write("\"" . $last_prefix . "\" has more than one (" . $result->completions_sent . ") completions", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id);
	  }

	  // CASE 2: prefix completed to a unique class name -> search for instances
	  else if ($query_string_original != "" && $unique_completion != "")
	  {
	    $query->query_string = $first_part_of_query . $last_separator_of_query . $last_prefix_of_query;
	    $query->hits_per_page_while_typing = $hits_per_page_while_typing_original;
	    array_push($compute, "hit_box", "hit_title_box");
	    $query->max_completions_show = $max_completions_show_original;
	    /*
	    $q = count($query_words);
	    $query->query_string = $q <= 1
	    ? $query_words[0] . (preg_match("|^type:|",$query_words[0]) ? "." : "..") . $entity_prefix
	    : implode(" ", array_slice($query_words,0,$q-2))
	    . "[" . $query_words[$q-2] . (preg_match("|^type:|",$query_words[$q-2]) ? "." : "..") . $entity_prefix
	    . "#" . $query_words[$q-1] . (preg_match("|^type:|",$query_words[$q-1]) ? "." : "..") . $entity_prefix . "]";
	    //? "author:[" . $query_words[0] . "]"
	    //: implode(" ", arra_slice($query_words,0,$q-2)) . "author:[" . $query_words[$q-2] . " " . $query_words[$q-1] . "]";
	    $query->hits_per_page_while_typing = 0;
	    */
	    $this->log->write("Query Y2 : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	    $result = $this->process_single($query);

	    for ($i = 0; $i < sizeof($result->completions); $i++)
	    {
	      $result->completions[$i]->string = preg_replace("|ce:([^:]+):([^:]+):([^:]+)|", "cf:$1:" . $sub_category . ":$2:$3", $result->completions[$i]->string);
//	      $result->completions[$i] = preg_replace("|ce:([^:]+):([^:]+):([^:]+)|", "cf:$1:" . $sub_category . ":$2:$3", $result->completions[$i]);
	    }
	    //			foreach ($compute as $computed) eval ('$this->result->' . $computed . ' = $result->' . $computed . ';');

	    $query->first_part_of_query = $first_part_of_query;
	    $query->last_separator_of_query = $last_separator_of_query;
	    $query->last_word_of_query = $last_prefix_of_query_original . ", the CLASS";
	    //$query->last_word_of_query = $query->query_string . " (Y)";
	    //$query->last_word_of_query = "ce:" . $sub_category; //"YAGO QUERY";

      // NEW 22-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
      //  (for a query of a type which has only one box (W,C for example) this index is always "1")
  	  $nac->result->Y_boxes[1] = $this->build_Y_boxes($result, $query);
//	    $nac->result->Y_boxes = $this->build_Y_boxes($result, $query);
	    $this->log->write($nac->result->Y_boxes, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);

      // TODO 22Jul07 (Holger): shouldn't there be a function for this block!?
	    // Add times, size and error_message of this call to the overall values for this process() call
	    $nac->result->time_A += $result->time_A;
	    $nac->result->time_B += $result->time_B;
	    $nac->result->response_size += $result->response_size;
	    if ($result->error_message != "") {
	      $nac->result->error_message .= $result->error_message . "<br>";
	    }
	    $nac->result->do_continue = $result->do_continue;

      // NEW 23Jul07 (Holger): display one hit (for entity:...) per completion
      if ($mode == "ONE_HIT_PER_COMPLETION")
      {
				$query->query_string = $query_string_original;
				// 07-10-08 / Markus hits_box is now H_boxes[1]
				$nac->result->H_boxes[1] = $this->build_H_boxes($result, $query);
				$nac->result->H_boxes[1]["body"] = "";

				for ($i = 0; $i < sizeof($result->completions) && $i <= $hits_per_page_while_typing_original; $i++)
//				for ($i = 1; $i < sizeof($result->completions) && $i <= $hits_per_page_while_typing_original; $i++)
				{
					preg_match("|cf:([^:]+):([^:]+):([^:]+):([^:]+)|", $result->completions[$i]->string, $matches);
//					preg_match("|cf:([^:]+):([^:]+):([^:]+):([^:]+)|", $result->completions[$i], $matches);
					$entity = "entity:" . $matches[3];
					$text = preg_replace("|[_\-]|", " ", $matches[4]);
					$text = preg_replace("|[^\w ]|", "", $text);
					$text = preg_replace("|\b[^ ]{1,3}\b|", "", $text);
						//$text = preg_replace("|\b([^ ]{1,3})\b|", "$1\$", $text);
					$text = preg_replace("| {1,}|", "..", $text);
						//$entity = preg_replace("|cf:([^:]+):([^:]+):([^:]+):([^:]+)|", "entity:$3", $result->completions[$i]);
            // 1. "beatles..musician" -> "beatles cr:*.;.entity:johnlennon john..lennon"
					$query->query_string = $first_part_of_query  != ""
																	 ? $first_part_of_query . "  " . "cr:.;." . $entity . "  " . $text
																	 :                               "cr:.;." . $entity . "  " . $text;
          // 2. "audience..pope..politician" -> "audience..pope..entity:angelamerkel"
          $query->query_string = $first_part_of_query . $last_separator_of_query . $entity; // . " " . $entity;
						//$query->query_string = $entity;
						//$query->query_string = $first_part_of_query ? $first_part_of_query . " " . $entity : $entity;
						//$query->query_string = $first_part_of_query . $last_separator_of_query . $entity . " " . $entity;
					//$this->log->write("Query Y3.".$i.". : \"" . $query->query_string . "\"",
					//                  $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
					$query->max_completions_show = 0;
					$query->hits_per_page_while_typing = 1;
					$tmp = $this->process_single($query);
          // NEW Markus / 12-11-08
					$hit = $this->construct_hits_body($tmp);
//					$hit = preg_match("|(<dl>.*</dl>)|", $tmp->hits, $matches) ? $matches[1] : "";
					//if ($hit == "") $hit = "<h3>[error parsing query ".$query->query_string."]</h3>" . "[result from parser: --&gt;$tmp->hits&lt;--]";

					$nac->result->H_boxes[1]["body"] .= $hit;
				}
				//$nac->result->hit_box["body"] = "<div id=\"hits\">".$nac->result->hit_box["body"]."</div>";
				$nac->result->H_boxes[1]["body"] = "<div id=\"hits\" yyy=\"Y\">".$nac->result->H_boxes[1]["body"]."</div>";
			}

      // NEW 23Jul07 (Holger): remove [[,]],{{,}} from excerpt (hack, should not write them to excerpt in the first place)
      //$nac->result->hit_box["body"] = preg_replace("/(\[\[|\]\]|\{\{|\}\})/", "", $nac->result->hit_box["body"]);

      // NEW 22Jul07 (Holger): produce result via OR of completions (hack to get ranking and highlighting right)
      /*
      $entities = array();
      for ($i = 1; $i < sizeof($result->completions); $i++)
      {
	      $this->log->write("Completion #".$i." : \"" . $entities[$i] . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
        $tmp = preg_replace("|cf:([^:]+):([^:]+):([^:]+):([^:]+)|", "entity:$3", $result->completions[$i]);
        array_push($entities, $tmp);
      }
      $query->query_string = $first_part_of_query . $last_separator_of_query . implode("|", $entities);
	    $this->log->write("Query Y3 : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
	    $result = $this->process_single($query);
	    // NEW 19-07-07 (Markus):
	    $nac->result->hit_box = $this->build_hit_box($result, $query);
      */

	    // Add times, size and error_message of this call to the overall values for this process() call
	    $nac->result->time_A += $result->time_A;
	    $nac->result->time_B += $result->time_B;
	    $nac->result->response_size += $result->response_size;
	    if ($result->error_message != "") {
	      $nac->result->error_message .= $result->error_message . "<br>";
	    }
	    $nac->result->do_continue = $result->do_continue;
	  }

	  // CASE 3: no matching class name at all -> do nothing
	  else
	  {
      // NEW 22-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
      //  (for a query of a type which has only one box (W,C for example) this index is always "1")
  	  $nac->result->Y_boxes[1] = array();
//	    $nac->result->Y_boxes = array();
	    $this->log->write("QUERY Y : nothing to do", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	  }

    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
	}


  /**
   * T : Translation queries. Checks completions of :T:<last prefix> . If there 
   * is a unique completion :T:<last prefix>:<translation>, then replace <last 
   * prefix> by <translation> in query. If there is more than one <translation> 
   * show them in a box. If there is no such <translation>, don't show the box.
   */
  function process_T($ac_query, & $nac)
  {
    // 1. Initialization. Note that we have to work on a copy of ac_query. 
    // $compute is an array of things that are computed.
	  $query = unserialize(serialize($ac_query));
	  //$query = $ac_query;
	  $compute = array();
	  $query_words = preg_split("|([ .]+)|", $query->query_string, -1, PREG_SPLIT_DELIM_CAPTURE);
	  $hits_per_page_while_typing_original = $query->hits_per_page_while_typing;
	  $max_completions_show_original = $query->max_completions_show;
	  $query_string_original = $query->query_string;

	  // 2. Get translations = completions of :T:<last prefix> .
	  $last_prefix_of_query = array_pop($query_words);
	  $last_prefix_of_query_original = $last_prefix_of_query;
	  $last_separator_of_query = array_pop($query_words);
	  $first_part_of_query = implode("", $query_words);
	  $query->query_string = ":t:" . $last_prefix_of_query;
	  $query->hits_per_page_while_typing = 0;
	  $query->max_completions_show = 100;
    $this->log->write("Query T1: \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC,
                      $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
	  $result = $this->process_single($query);
	  $nac->result->time_A += $result->time_A;
	  $nac->result->time_B += $result->time_B;
	  $nac->result->response_size += $result->response_size;
	  if ($result->error_message != "") {
	    $nac->result->error_message .= $result->error_message . "<br>";
	  }
	  $nac->result->do_continue = $result->do_continue;

    // 3. See if there is a UNIQUE translation. That is, either single 
    // completion of :T:<last prefix> or exact match of :T:<last_prefix>: .
		$unique_completion = "";
		if ($result->completions_sent == 1)
	  {
			$unique_completion = $result->completions[0]->string;
		}
		else if ($result->completions_sent > 1) 
		{
      $num_exact_completions = 0;
      $last_exact_completion = "";
			for ($i = 0; $i < sizeof($result->completions); $i++) 
			{
        // $this->log->write("TRANSLATION: \"" . $result->completions[$i]->string . "\"", $this->log->levels['INFO'],
        //                   SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
        if (preg_match("/^:t:$last_prefix_of_query:/", $result->completions[$i]->string))
        {
          ++$num_exact_completions;
			    $last_exact_completion = $result->completions[$i]->string;
        }
			}
      if ($num_exact_completions == 1)
      {
			  $unique_completion = $last_exact_completion;
      }
		}
    if ($unique_completion != "")
    {
      $this->log->write("Unique translation: \"" . $unique_completion . "\"", $this->log->levels['INFO'],
                        SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
    }
    else
    {
      $this->log->write("No unique translation", $this->log->levels['INFO'],
                        SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
    }

		// 4. If unique translation, replace query part by this translation.
    if ($unique_completion != "") 
	  {
      $last_prefix_of_query = preg_replace("|:t:[^:]+:|", "", $unique_completion);
      $last_prefix_of_query .= "*";
	    // $ch_parts = explode(":", $unique_completion);
	    // $sub_category = $ch_parts[1];
	    // $super_category = $ch_parts[2];
	    // $sub_category_alphnum_only = preg_replace("|_|", "", $sub_category);
	    // $super_category_alph_num_only = preg_replace("|[\W_]|", "", $super_category);
	    // $last_prefix_of_query = $sub_category == $super_category
    	//     ? "ce:" . $super_category_alph_num_only
    	//     : "[ce:" . $super_category_alph_num_only . ":*#cr:isa.ce:class:"
    	//     . $sub_category_alphnum_only . ".;;.ce:" . $super_category_alph_num_only . ":*]";
	  }

	  // 5. If more than one translation (and non-empty query), show them in a separate box.
	  if ($query_string_original != "" && $result->completions_sent > 1 && $unique_completion == "")
	  {
	    $query->first_part_of_query = $first_part_of_query;
	    $query->last_separator_of_query = $last_separator_of_query;
	    $query->last_word_of_query .= " (T --- NOT UNIQUE)";
  	  $nac->result->Y_boxes[1] = $this->build_T_boxes($result, $query);
	  }

    // 6. If unique translation, launch translated query and replace search 
    // results of original query.

          else if ($query_string_original != "" && $unique_completion != "")
	  {
	    $query->query_string = $first_part_of_query . $last_separator_of_query . $last_prefix_of_query;
	    $query->hits_per_page_while_typing = $hits_per_page_while_typing_original;
	    array_push($compute, "hit_box", "hit_title_box");
	    // array_push($compute, "hits_computed", "hit_box", "hit_title_box");
	    $query->max_completions_show = $max_completions_show_original;
      $this->log->write("Query T2: \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC,
        $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);

            // NEW 29 Okt 2010 (Björn):
            // Also set the variabled translated_query to indicate that a 
            // translation happend for further reloading!
            $nac->translated_query['original'] = $ac_query->query_string;
            $nac->translated_query['translation'] = $query->query_string;

	    $result = $this->process_single($query);
	    $nac->result->hits_computed = $result->hits_computed;
	    $nac->result->H_boxes[1] = $this->build_H_boxes($result, $query);
	    $nac->query->query_string = $query->query_string;
	    $nac->result->time_A += $result->time_A;
	    $nac->result->time_B += $result->time_B;
	    $nac->result->response_size += $result->response_size;
	    if ($result->error_message != "") {
	      $nac->result->error_message .= $result->error_message . "<br>";
	    }
	    $nac->result->do_continue = $result->do_continue;

	    for ($i = 0; $i < sizeof($result->completions); $i++)
	    {
	      $result->completions[$i]->string = preg_replace("|:e:|", ":f:", $result->completions[$i]->string);
	    }

	    $query->first_part_of_query = $first_part_of_query;
	    $query->last_separator_of_query = $last_separator_of_query;
	    $query->last_word_of_query = $last_prefix_of_query_original . ", the CLASS";
  	  $nac->result->Y_boxes[1] = $this->build_T_boxes($result, $query);
      $this->log->write($nac->result->Y_boxes, $this->log->levels['DEBUG'], SCRIPT_AC,
                        $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
    }

	  // 7. If no translations, do nothing and leave box empty.
	  else
	  {
  	  $nac->result->Y_boxes[1] = array();
      $this->log->write("QUERY T : nothing to do", $this->log->levels['INFO'], SCRIPT_AC,
                        $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
	  }

    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id,
                       __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
  }


	/**
   * NEW 16Aug07 (Holger) P : query for *p*recomputed DBLP facets
   *
   *   NOTE: is called process_P (p for precomputed) but actually produces F boxes (f for facets) !!
   *
   *   TODO 1: reset boxes not needed
   *
   *   TODO 2: block division of hybrid is not good
   *
   */
  function process_P($ac_query, & $nac)
  {
    $this->log->write($nac, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac");
    $this->log->write($ac_query, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$ac_query");

    $query = unserialize(serialize($ac_query));
    $boxes_tmp = array();

    $max_completions_show_original = $query->max_completions_show;

    $query->hits_per_page_while_typing = 1;
    $this->log->write("Query P1 : \"" . $query->query_string . "\"", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['FILE']);
    $result = $this->process_single($query);
    $this->log->write($result, $this->log->levels['DEBUG'], SCRIPT_AC, $ac_query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$result");

    // NEW Markus / 07-07-09: changements to apply new XML response format of CS server
//    $hits = $result->hits;
    $hits = $result->hits[0]->excerpts[0];
    $this->log->write($hits, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$hits");
    $this->log->write("Hits P1  : \"" . $hits . "\"", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['FILE']);
    
    // Example: <facet name="year" items="5">ct:year:2005:2005 (4)<br>ct:year:2006:2006 (4)<br>ct:year:2007:2007 (4)<br>ct:year:2000:2000 (2)<br>ct:year:2008:2008 (2)</facet>
    preg_match_all("|<facet name=\"([^\"]+?)\" items=\"([^\"]+?)\">(.*?)<\/facet>|", $hits, $matches);
//    $this->log->write($matches, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$matches");
    
    for ($i = 0; $i < sizeof($matches[1]); ++$i)
    {
      // Run through the i-th facet
      
      // The response of CS server seems to be not correct: one completion is sent in <completions>-section, the other ones in <hits>-section.
      // So, we clear the completions array (because it contains already this one completion
      // And, in addition, we have to array of completions for every facet
      $result->completions = array();

//      $result->completions_sent = $matches[2][$i];
      $result->completions_total = $matches[2][$i];

      $facets = explode("<br>", $matches[3][$i]);
      // CHANGED 07-07-09 / Markus: the list of entries for every facet ends with "<br>"; 
      // so explode() delievers one element too much (the last string is empty)
//      for ($j = 0; $j < sizeof($facets); $j++)
      for ($j = 0; $j < sizeof($facets) - 1; $j++)
      {
        preg_match("|^(.*?)\s*\((\d+)\)\s*$|", $facets[$j], $m);
        
        // NEW Markus / 07-07-09: changements to apply new XML response format of CS server
        $completion = new Completion();
        $completion->string = $m[1];
        $completion->document_count = $m[2];
        $result->completions[] = $completion;

//        $result->completions[$j + 1] = $m[1];
//        $result->hits_count_per_completion[$j + 1] = $m[2];
      }
      // NEW Markus / 07-07-09
//      $result->completions_sent = $j;
      $this->log->write($result, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "[i:$i] \$result");
      $this->log->write($result->completions, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "[i:$i] \$completions");

      // NEW 19Dec07 (Holger & Markus): remove remaining completions from previous round (was a bug!)
      // CHANGED 07-07-09 / Markus: this is no longer needed (the completions array is cleared at beginning of every iteration)
//      array_splice($result->completions, sizeof($facets) + 1);

      $query->first_part_of_query = $query->query_string;
      $query->query_types = "WHF";

      //$this->log->write("!!!! " . print_r($result->completions, true), $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
      $boxes_tmp = array_merge($boxes_tmp, $this->build_P_boxes($result, $query, $i + 1));
    }

    $nac->result->time_A += $result->time_A;
    $nac->result->time_B += $result->time_B;
    $nac->result->response_size += $result->response_size;
    if ($result->error_message != "") $nac->result->error_message .= $result->error_message . "<br>";
    $nac->result->do_continue = $result->do_continue;

    $nac->result->F_boxes = $boxes_tmp;
    $this->log->write($nac->result->F_boxes, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets['SESSION'], "\$nac->result->F_boxes");
  }


	/**
	 * Processing function R: YAGO QUERY (relation)
	 *
	 * TODO currently (mis)using join completion box
	 *
	 * @param Query $ac_query
	 * @param AC $nac
	 */
	function process_R($ac_query, & $nac)
	{
	  // We have to work on a copy of ac_query
	  $query = unserialize(serialize($ac_query));

	  $this->log->write("Query R : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);

	  $query_words = preg_split("|([ .]+)|", $query->query_string, -1, PREG_SPLIT_DELIM_CAPTURE);
	  $max_completions_show_original = $query->max_completions_show;
	  $query_string_original = $query->query_string;

	  $last_prefix_of_query = array_pop($query_words);
	  $last_separator_of_query = array_pop($query_words);
	  $first_part_of_query = implode("", $query_words);

	  // CHECK WHETHER LAST PREFIX MATCHES A RELATION (like cr:borninyear)

	  if (preg_match("|^cr|", $last_prefix_of_query))
	  {
	    $query->query_string = /* "cr:" . */ $last_prefix_of_query;
	    $query->hits_per_page_while_typing = 0;
	    $this->log->write("Query R1a : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	    $result = $this->process_single($query);
	  }

	  // IF IT DOESN'T, CHECK WHETHER PREVIOUS WORD HAS ANY RELATIONS

	  if (!preg_match("|^cr|", $last_prefix_of_query) || $result->completions_sent == 0)
	  {
	    $query->query_string = $query_string_original . ".cr:";
	    $query->hits_per_page_while_typing = 0;
	    $this->log->write("Query R1b : \"" . $query->query_string . "\"", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	    $result = $this->process_single($query);
	  }

	  // CASE 1: exactly one completion -> display matching triples

	  if ($result->completions_sent == 1)
	  {
	    preg_match("|^cr:([^:]+):([^:]+):([^:]+):([^:]+):([^:]+)$|", $result->completions[1], $matches);
	    $matches[5] = preg_replace("|_|", " ", $matches[5]);
	    //            $this->log("***** " . implode(" ; ", $matches) . " *****");

	    $relation_separator = "." . str_repeat(",", $matches[4] - 1) . ".";
	    // NEW 23Jul07 (Holger): is target type is not class, take entity (hack to remedy a mistake I did during index building)
	    $target_type = $matches[3] == "class" ? "class" : "entity";
	    $query->query_string = $first_part_of_query . ".cr:" . $matches[1] . $relation_separator . "ce:" . $target_type . ":";
	    $this->log->write("QUERY R2 : " . $query->query_string, $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);

	    $result = $this->process_single($query);
	    //			foreach ($compute as $computed) eval ('$this->result->' . $computed . ' = $result->' . $computed . ';');
	    $query->first_part_of_query = $first_part_of_query;
	    $query->last_separator_of_query = $last_separator_of_query;
	    $query->last_word_of_query = strtoupper($matches[5]) . " (R)";

      // NEW 22-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
     //  (for a query of a type which has only one box (W,C for example) this index is always "1")
  	  $nac->result->R_boxes[1] = $this->build_R_boxes($result, $query);
//	    $nac->result->R_boxes = $this->build_R_boxes($result, $query);

	    // Add times, size and error_message of this call to the overall values for this process() call
	    $nac->result->time_A += $result->time_A;
	    $nac->result->time_B += $result->time_B;
	    $nac->result->response_size += $result->response_size;
	    if ($result->error_message != "") {
	      $nac->result->error_message .= $result->error_message . "<br>";
	    }
	    $nac->result->do_continue = $result->do_continue;
	  }

	  // CASE 2: more than one completion -> display options

	  else if ($result->completions_sent > 1)
	  {
	    $query->first_part_of_query = $query_string_original;
	    $query->last_separator_of_query = " ";
	    $query->last_word_of_query = "RELATIONS (R)";
      // NEW 22-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
      //  (for a query of a type which has only one box (W,C for example) this index is always "1")
  	  $nac->result->R_boxes[1] = $this->build_R_boxes($result, $query);
//	    $nac->result->R_boxes = $this->build_R_boxes($result, $query);

	    // Add times, size and error_message of this call to the overall values for this process() call
	    $nac->result->time_A += $result->time_A;
	    $nac->result->time_B += $result->time_B;
	    $nac->result->response_size += $result->response_size;
	    if ($result->error_message != "") $nac->result->error_message .= $result->error_message . "<br>";
	    $nac->result->do_continue = $result->do_continue;
	  }

	  // CASE 3: no completions -> do nothing

	  else
	  {
      // NEW 22-01-07 (Markus): the result box arrays gets now a numeric key i which indicates that the
      //  result belongs to the i-th result box
      //  (for a query of a type which has only one box (W,C for example) this index is always "1")
  	  $nac->result->R_boxes[1] = array();
//	    $nac->result->R_boxes = array();
	    $this->log->write("QUERY R : nothing to do", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	  }
	}


	/**
	 * Process a single query
	 *
	 *	1. normalize query (append *, rewrite join queries, etc.)
	 *	2. sent to completion server
	 *	3. parse answer from server and return it in a Result object
	 *
	 * @param Query $query
	 * @return Result $result An instance of class Result which contains the answer of the CompleteSearch server
	 */
	function process_single (& $query)
	{
    $this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, "\$query");

    $next_hits_url = "";	// The url for the request of the next hits (for example for page up/down)
		$previous_hits_url = "";	// The url for the request of the previous hits (for example for page up/down)

		// Create a result object
//		$result = new Result();

		// First some special cases which require no interaction with the completion server

		// The query is empty
		// Note: the empty query must be processed, too, because we need the number of document which is computed with the empty query
//     if ($query->query_string == "") { return new Result(); }
//     if ($query->query_string == "") { return $result; }

		// The query string is too short
		// CHANGED Markus / 06-04-09: "*" is now used to compute the count of all document (was the empty string before)
		if ($query->query_string != "" && $query->query_string != "*" && strlen($query->query_string) < $this->settings->min_query_size)
		{
		  $result = new Result();
		  // Set text for options title for the case we called process() because language was changed
		  $result->options_title = $this->get_text("options_title");
		  $result->subtitle = sprintf ($this->get_text("query_too_short"), $this->settings->min_query_size) . "!!";
      $this->log->write("Query is too short, abort", $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__);
		  return $result;
		}

/*
		// Provocated errors for testing
		if (strlen($query->query_string) == 3) sleep(5);

		if (preg_match("|fuck\S*$|", $query->query_string))
		{
			$result->set_error_message ("Do you have to use so many cuss words?", "Just one thing, Dude.");
			return $result;
		}
*/

		// NEW 23Jul06: compute last word and first part of query
		$matches = null;
		$SEP = $this->settings->separators;
		preg_match("/^(.*?)([$SEP]*)([^$SEP]+)$/", $query->query_string, $matches);

		// The following we only may do if preg_match was succesful (also not the empty query, for example)
		if (sizeof($matches) > 0)
		{
			$query->first_part_of_query = $matches[1];
			$query->last_separator_of_query = $matches[2];
			$query->last_word_of_query = $matches[3];
			$this->log->write("[first part|last separator|last word] = [" . $query->first_part_of_query . "|"
				. $query->last_separator_of_query . "|"
				. $query->last_word_of_query . "]", $this->log->levels['INFO'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);
		}
		else
		{
			// TODO Have we to set first_part_of_query, last_separator_of_query, last_word_of_query to empty string?
			$this->log->write ("Query was empty, so the following were not computed: first_part_of_query, last_separator_of_query, last_word_of_query", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
		}

		$this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$query");

    // NEW 21Oct11 (Hannah + Ina): GET/?q= ... HTTP/1.0 no longer added in 
    // query_for_completion_server in query.php, see detailed comment there.
    // NEW 25Oct11 (Ina): query_for_completion_server is now splitted in 
    // rewrite_query and get_query_parameters.
    $query_string = "GET /?q=" . $query->rewrite_query() . $query->get_query_parameters($this->javascript) . " HTTP/1.0";
		// $query_string = $query->query_for_completion_server();

		// zum testen
    // $query_string = 'GET /?q=user* ct:year:*&h=0&c=8&f=0&en=3&er=7&rd=1a&rw=1d HTTP/1.0';
		
		$this->log->write("Sending query '$query_string' ...", $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__);

		// Save time to compute duration of processing
		$time_recv_begin = microtime_float();

		$response = "";
		// The result of the cs server is delievered by reference in $response
		$rc = $this->send_to_server($query_string, $response);

		// Check the return code
		if ($rc !== true)
		{
		  // Create a Result object, set it with error information and return it
		  $result = new Result();
			switch ($rc)	// return code
			{
				case $this->errors['OPEN_SOCKET_FAILED']:
					$result->set_error_message ($this->get_text("error_open_socket"));
					$result->subtitle = $this->get_text("subtitle_on_error_open_socket");
					saveTimestamp(" connect() CONNECTERROR");
					break;
				case $this->errors['SOCKET_TIMEOUT']:
					$result->set_error_message ($this->get_text("error_no_server_connection"));
					$result->subtitle = $this->get_text("subtitle_on_no_server_connection");
					saveTimestamp(" connect() TIMEOUT");
					break;
				default:
					$result->set_error_message ($this->get_text("error_server_connection"));
					$result->subtitle = $this->get_text("subtitle_on_error_server_connection");
					saveTimestamp(" connect() CONNECTERROR");
			}

			$result->time_B = -1;	// means not yet set, value unknown
			return $result;
		}

 		$this->log->write($response, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "Answer from CS server");
// 		$this->log->write("Answer from CS server: '$response'", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
//    $response = str_replace("&", "&amp;", $response);

    $result = new Result($response);
 		$this->log->write($result, $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "Result from CS server: \$result");
    
    
    // Set text for options title for the case we called process() because language was changed
 		$result->options_title = $this->get_text("options_title");

		// Time for sending and receiving query to and from completion server in ms
		$result->time_B = round((microtime_float() - $time_recv_begin) * 1000, 2);

		// Set the size of the response of the server
		$result->response_size = strlen($response);
		$this->log->write("Query '" . $query->query_string . "': " . $result->response_size . " characters from completion server in "
			. $result->time_B . " ms received (" . $result->time_A . " ms for computation of completion server)", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);

    /*
		foreach ($lines as $line)
		{
			$size += strlen($line);

			// Check whether the current line contains the report
			$matches = "";
			if (preg_match('/<div id="parameters">(.*?)<br>(.*?)<br>(.*?)<br>(.*?)<br>(.*?)<\/div>/', $line, $matches))
			{
			  // $prefix = $matches[1];
			  // NEW 29.08.06 (Markus): use now a local created result object
			  $result->completions_sent = (int) $matches[2];
			  $result->hits_sent = (int) $matches[3];
				// CHANGED 26-02-08 (Markus)
			  $result->first_hit = (int) $matches[4] + 1;
			  $result->last_hit = (int) $matches[5] + 1;
//			  $result->first_hit = (int) $matches[4];
//			  $result->last_hit = (int) $matches[5];
			}
			else
			{
				if (preg_match('/<div id="completions">(.*)<\/div>/', $line, $matches))
				{
					$result->completions = array();
					$result->hits_count_per_completion = array();

					// As first element of the completions list set the last word of the query
					$result->completions[] = $query->last_word_of_query;
					$result->hits_count_per_completion[] = 0;

					foreach (explode("<br>", $matches[1]) as $completion)
					{
						if ($completion != "...")
						{
						  // NEW 09-08-07 (Markus): retrieve completions and their count in an other way
						  // Matches pattern like xyz (12)
						  // Note: \S --> all not whitespace characters
              if (preg_match('/(\S*).*\((.*)\)/', $completion, $matches) > 0)
              {
                $result->completions[] = $matches[1];
                $result->hits_count_per_completion[] = (int) $matches[2];
              }
//							$t = explode(" (", $completion);
//							$result->completions[] = $t[0];
//							$result->hits_count_per_completion[] = (int) rtrim($t[1], ")");
						}
					}
					// If the first completion is the last word of query set the start index to 1
					// to provide double entries at the beginning of the completions list
					if (count($result->completions) > 1 && $result->completions[0] == $result->completions[1]) {
//					if ($result->completions[0] == $result->completions[1]) {
						$result->completion_start_index = 1;
					}
					else {
						$result->completion_start_index = 0;
					}
				}
				else
				{
//					if (preg_match('/<div id="hits">(.*)<\/div>/', $line, $matches)) {
					if (preg_match('/<div id="hits"><div class="hits"><h1>.*<\/h1>(.*)<\/div><\/div>/', $line, $matches)) {
						$result->hits = $matches[1];
//						$result->hits = $matches[0];
					}
					else
					{
						// Check for data about time spent for this call
						if (preg_match('/<div id="time">(.*)<\/div>/', $line, $matches)) {
							$result->time_A = $matches[1];
						}
					}
				}
			}
		}
		$result->response_size = $size;
		$this->log->write("Query '" . $query->query_string . "': " . $result->response_size . " characters from completion server in "
			. $result->time_B . " ms received (" . $result->time_A . " ms for computation of completion server)", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
    */
		return $result;
	}


	/**
	 * Set $parameter (by reference) and corresponding cookie from POST / GET data, cookie or $default
	 *
	 * In detail:
	 * If an url parameter with name $name is given and it differs from its current value in object AC,
	 * set $parameter and the corresponding cookie to the value of that url parameter.
	 * Otherwise if a cookie with name $name is given, set $parameter to the value of that cookie.
	 * The parameter $is_int indicates that the parameter is to set as an integer.
	 * Note: This calls must happen before the first HTML output is generated (because of the call of setcookie()).
	 *
	 * @param mixed $parameter
	 * @param string $name
	 * @param mixed $default
	 * @param boolean $is_int
	 */
	function set_parameter (&$parameter, $name, $default, $is_int = false)
	{
	  $tmp = variable($name);
	  if (isset ($tmp) && $tmp <> "" && $tmp != $parameter)
	  {
	    $parameter = $tmp;
	    $this->state = "changed";

	    // Set cookie for all pages below the directory where the index page is located; expires in 30 days
	    setcookie ($name, $tmp, time() + $this->settings->cookie_expiration, $this->settings->index_path);
    	$this->log->write ("Set parameter and cookie from URL: $name / set_parameter", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	  }
	  else
	  {
	    if (isset($_COOKIE[$name]))
	    {
	      if ($parameter != $_COOKIE[$name])
	      {
	        $parameter = ($is_int) ? (int) $_COOKIE[$name] : $_COOKIE[$name];
	        $this->state = "changed";
        	$this->log->write ("Set parameter from cookie: $name = $parameter (set_parameter)", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	      }
	    }
	    else
	    {
	      if ($parameter !== $default)
	      {
	        // NEW 08.11.06 (Markus)
	        $parameter = ($is_int) ? (int)$default : $default;
	        $this->state = "changed";
        	$this->log->write ("Set parameter from default: $name = $parameter (set_parameter)", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	      }
	    }
	  }
	}


	/**
	 * Set $parameter (by reference) from cookie or $default
	 * Set parameter referenced by $parameter to the value of the cookie with name $name or to $default
	 *
	 * @param mixed $parameter
	 * @param string $name
	 * @param mixed $default
	 * @param boolean $is_int
	 */
	function set_parameter_from_cookie (&$parameter, $name, $default, $is_int = false)
	{
	  if (isset($_COOKIE[$name]) && $parameter != $_COOKIE[$name])
	  {
	    $parameter = ($is_int) ? (int) $_COOKIE[$name] : $_COOKIE[$name];
	    $this->state = "changed";
     	$this->log->write ("Set parameter from cookie: $name = $parameter (set_parameter_from_cookie)", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	  }
	  else
	  {
	    if ($parameter !== $default)
	    {
	      $parameter = ($is_int) ? (int)$default : $default;
	      $this->state = "changed";
       	$this->log->write ("Set parameter from default: $name = $parameter (set_parameter_from_cookie)", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	    }
	  }
	}


	/**
	 * Set $parameter (by reference) from POST / GET data or $default
	 * Set parameter referenced by $parameter to the value of the POST / GET with name $name or to $default
	 *
	 * @param mixed $parameter
	 * @param string $name
	 * @param mixed $default
	 * @param boolean $is_int
	 */
	function set_parameter_from_url (&$parameter, $name, $default = null, $is_int = false)
	{
	  $tmp  = variable($name);

		// CHANGED: 22-02-08 (Markus)
		if (isset ($tmp))
		{
			if ($tmp != $parameter)
			{
				$parameter = ($is_int) ? (int)$tmp : $tmp;
				// Set state to changed to indicate that new processing is necessary
				$this->state = "changed";
				$this->log->write ("Set parameter from URL: $name = $parameter (set_parameter_from_url)", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
			}
	  }
	  else
	  // CHANGED Markus / 06-05-09
//	  if (! is_null($default) && $parameter !== $default)
	  if ($parameter !== $default)
	  {
	    $parameter = ($is_int) ? (int)$default : $default;
	    // Set state to changed to indicate that new processing is necessary
	    $this->state = "changed";
     	$this->log->write ("Set parameter from default: $name = $parameter (set_parameter_from_url)", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	  }
	}


	/**
	 * Set $parameter (by reference) and corresponding cookie from POST / GET data or $default
	 *
	 * Set parameter referenced by $parameter and the corresponding cookie
	 * to the value of the url parameter with name $name or to $default
	 * Note: This calls must happen before the first HTML output is generated (because of the call of setcookie()).
	 *
	 * @param mixed $parameter
	 * @param string $name
	 * @param mixed $default
	 * @param boolean $is_int
	 */
	function set_parameter_and_cookie_from_url (& $parameter, $name, $default = null, $is_int = false)
	{
		$tmp = variable($name);
		// CHANGED: 05-01-09 (Markus)
		if (isset ($tmp))
		{
		  if ($tmp != $parameter)
		  {
		    $parameter = ($is_int) ? (int)$tmp : $tmp;
		    // Set state to changed to indicate that new processing is necessary
		    $this->state = "changed";
		    // Set cookie for all pages below the directory where the index page is located; expires in 30 days
		    setcookie ($name, $tmp, time() + $this->settings->cookie_expiration, $this->settings->index_path);
		    // $this->log->write ("Set parameter and cookie from URL: $name / set_parameter_and_cookie_from_url", $AC->log->levels['DEBUG'], SCRIPT_AC, $AC->query->request_id);
		  }
		}
		else
	  if (! is_null($default) && $parameter !== $default)
	  {
	    $parameter = ($is_int) ? (int)$default : $default;
	    // Set state to changed to indicate that new processing is necessary
	    $this->state = "changed";
     	$this->log->write ("Set parameter from default: $name = $parameter (set_parameter_and_cookie_from_url)", $this->log->levels['DEBUG'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
	  }
	}

	

	/**
	 * Initialize a query object from AC settings
	 *
	 * @param Query $query
	 */
//	function init_query(& $query)
//	{
//	  foreach (get_object_vars($query) as $name => $value)
//	  {
//	    // Set only those members which have an corresponding member in $AC->settings
//	    if (isset($this->settings->$name))
//	    {
//	      $query->$name = $this->settings->$name;
//	    }
//	  }
//	}
	
	
	
  /**
	 * Send a query to the completion server.
	 * This includes opening the socket connection, write the query to this socket and wait for the answer.
	 * If the server respond within the timeout period the response is written into $response and true is returned.
	 *
	 * @param string $query the query string to send to the completion server
	 * @param string $response the string containing the answer of the completion server
	 * @return boolean true if sending and receiving was succesful, false otherwise
	 */
  function send_to_server ($query, & $response)
  {
			// Bug in PHP 5.3.2: fsockopen cannot resolve "localhost", using 
			// "127.0.0.1" instead works. See http://bugs.php.net/bug.php?id=51079.
			$tmp_hostname = $this->settings->server_hostname; 
			if ($tmp_hostname == "localhost") $tmp_hostname = "127.0.0.1";
      // printf("Query: \"%s\"</br>", $query); // HHH
      // printf("Host: \"%s\"</br>", $tmp_hostname); // HHH
      // printf("Port: \"%s\"</br>", $this->settings->server_port); // HHH
      // printf("Timeout: \"%s\"</br>", $this->settings->server_timeout); // HHH
      saveTimestamp("before: connect()");
      $socket = fsockopen($tmp_hostname, $this->settings->server_port,
      $errno, $errstr, $this->settings->server_timeout);
      // printf("Error: \"%s\"</br>", $errstr); // HHH
			// exit(1); // HHH
      saveTimestamp("after: connect()");

      if (! $socket)
      {
          $this->log->write("Could not open socket", $this->log->levels['ERROR'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
          return $this->errors['OPEN_SOCKET_FAILED'];
      }

      // Testing: if sleeping for 100 milliseconds here, completion server
      // will close connection and fgets below will block for the maximum execution time of PHP
      // usleep(100000);

      $result = socket_set_blocking($socket, true);
      if (! $result)
      {
          $this->log->write("Could not set blocking mode", $this->log->levels['ERROR'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
          fclose($socket);
          return false;
      }

      saveTimestamp("before: fwrite()");
      $number_of_bytes = fwrite($socket, rawurldecode($query) . "\0");
      saveTimestamp("after: fwrite()");
      if (! $number_of_bytes)
      {
          $this->log->write("Sending failed", $this->log->levels['ERROR'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
          fclose($socket);
          return false;
      }

      $this->log->write("The query sent to the completion server has a size of $number_of_bytes bytes: " . $query, $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);

      // Set timeout for stream operations (here: fgets())
      $result = stream_set_timeout($socket, $this->settings->server_timeout);
      if (! $result)
      {
          // This failure means that in the case of a completion server hangup we don't have an escape from fgets()
          // (except of the maximum execution time of PHP)
          $this->log->write("Could not set timeout for stream operations", $this->log->levels['WARNING'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
      }

      // IN PHP5 we can use the following which is maybe a little bit more efficient?
      // $response = stream_get_contents($socket);

      saveTimestamp("before: fread()");
      while (!feof($socket))
      {
          // We tested time of fgets() versus fread(): there were only very little time differences.
          // So for performaces purposes there is no need to use fread()
          // $lines[] = fgets($socket);
          $response .= fread($socket, 8192);

          // Test for socket timeout
          $info = stream_get_meta_data($socket);
          if ($info['timed_out'])
          {
              $this->log->write("Timeout during fread()", $this->log->levels['ERROR'], SCRIPT_AC, $this->query->request_id, __FUNCTION__, __LINE__);
              fclose($socket);
              return $this->errors['SOCKET_TIMEOUT'];
          }
      }
      saveTimestamp("after: fread()");

      // Extract the XML part of the response by truncating the header information
      $k = strpos($response, "<?xml version=");

      if ($k !== false) {
          $response = substr($response, $k);
      }

      fclose($socket);
      return true;
  }


	/**
	 * BUILD COMPLETION STRING
	 *
	 *
	 * Build a HMTL formatted box of type $type from the completions array of $result
	 * Types are all query types except of the type F (facettes); they have their own build_F_boxes() method
	 *
	 * @param Result $result the data from which the completion box is build
	 * @param Query $query the query for which the completion box is build
	 * @return string the HMTL formatted box containing the completions and their title
	 */
	function build_boxes (& $result, & $query, $type, $query_index)
	{
	  global $log;
    $this->log->write($result, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, "\$result");
    $log->write($result, $log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, "\$result");
    $log->write($query, $log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, "\$query");

    // NEW 11-04-07 (Markus)
		if ($result->error_message_title != "" || $result->error_message != "") {
			return array (array("title" => $result->error_message_title, "body" => "<div class=\"box_line\">$result->error_message</div>"));
		}
		$r = "";

		// NEW 24Jul06 (Holger): build completion string
		for ($i = 0; $i < sizeof($result->completions); $i++)
//		for ($i = 1; $i < sizeof($result->completions); $i++)
		{
			// Rewrite words starting with cn: and ct:
			if (preg_match("|^cn:([^:]+):([^:]*):([^:]*):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches))
//			if (preg_match("|^cn:([^:]+):([^:]*):([^:]*):([^:]+):([^:]+)$|", $result->completions[$i], $matches))
			{
				// Kurt Mehlhorn, the AUTHOR -> AUTHOR: Kurt Mehlhorn
				$completion_show = preg_replace("|_|", " ", $matches[5] . ", the " . strtoupper($matches[4]));
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query
				. /* "ct:" . */ $matches[4] . ":" . $matches[2] . $matches[1] . $matches[3]	. $query->append_to_clicked_cat_completion; // ":";
			}
			// Rewrite words starting with ce:
			else if (preg_match("|^ce:([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//			else if (preg_match("|^ce:([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//			else if (preg_match("|^ce:([^:]+):([^:]+):([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				// NEW 07May07 (Holger): remove the clumsy "(the organism)" and the like
				if (preg_match("|\(.*\)|", $matches[3]))
				{
					$matches[3] = preg_replace("|[\s_]+\(.*\)|", "", $matches[3]);
					$matches[3][0] = strtoupper($matches[3][0]);
					$matches[1] = preg_replace("|the.*$|","", $matches[1]);
				}
				// HACK 13Feb07 (Holger): omit ", the ENTITY", it looks stupid
				$completion_show = strcmp($matches[1], "entity") == 0
					? preg_replace("|_|", " ", /*strtolower*/($matches[3]))
					: preg_replace("|_|", " ", $matches[3] . ", the " . strtoupper($matches[1]));
				//$completion_show = preg_replace("|_|", " ", $matches[3] . ", the " . strtoupper($matches[1]));
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query
					. /* "ce:" . */ $matches[1] . ":" . $matches[2] . $query->append_to_clicked_cat_completion;
			}

			// NEW 11Feb07 (Holger): rewrite synonyms of the form s:123:xyz
			else if (preg_match("|^s:(\d+):(.+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//			else if (preg_match("|^s:(\d+):(.+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//			else if (preg_match("|^s:(\d+):(.+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				$completion_show = $matches[2];
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query
					. $matches[2] . $query->append_to_clicked_cat_completion;
			}

			// ***** NEW 24Jan07 (Holger): rewrite words starting with cf:
			else if (preg_match("|^cf:([^:]+):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//			else if (preg_match("|^cf:([^:]+):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//			else if (preg_match("|^cf:([^:]+):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				// NEW 07May07 (Holger): remove the clumsy "(the organism)" and the like
				$matches[4][0] = strtoupper($matches[4][0]);
				$matches[4] = preg_replace("|[\s_]+\(.*\)|", "", $matches[4]);
				$completion_show = preg_replace("|_|", " ", $matches[4] . ", the " . strtoupper($matches[2]));
        // NEW 22Jul07 (Holger): on click, write "entity:zzz" instead of "ce:xxxtheyyy:zzz"
        // NEW 26Jul07 (Holger): on click, omit the first part of the query
        $completion_link = true || preg_match("|^red|", $query->first_part_of_query)
				                    ? $query->first_part_of_query . $query->last_separator_of_query . "entity:" . $matches[3] . $query->append_to_clicked_cat_completion
                            : "entity:" . $matches[3] . $query->append_to_clicked_cat_completion;
				  //$completion_link = $query->first_part_of_query . $query->last_separator_of_query
					//. "ce:" . $matches[1] . ":" . $matches[3] . $query->append_to_clicked_cat_completion;
			}

			// NEW 21Sep10 (Hannah): rewrite words starting with F:
      else if (preg_match("|^:[ef]:.*:([^:]+):([^:]+):([^:]+)$|",
               $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
			{
				// $matches[4][0] = strtoupper($matches[4][0]);
				// $matches[4] = preg_replace("|[\s_]+\(.*\)|", "", $matches[4]);
				$completion_show = preg_replace("|_|", " ", $matches[3] . ", the " . strtoupper($matches[1]));
        $completion_link = $query->first_part_of_query .
                           $query->last_separator_of_query .
                           ":e:entity:" . $matches[2] . ":";
			}

			// NEW 22Sep10 (Hannah): rewrite words starting with T:
      else if (preg_match("|^:t:([^:]+):(.*):([^:]+):([^:]+):$|",
               $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
			{
				// $matches[4][0] = strtoupper($matches[4][0]);
				// $matches[4] = preg_replace("|[\s_]+\(.*\)|", "", $matches[4]);
				$completion_show = preg_replace("|_|", " ", $matches[1] . ", the " . strtoupper($matches[3]));
        $completion_link = $query->first_part_of_query .
                           $query->last_separator_of_query .
                           $matches[2] . ":" . $matches[3] . ":" . $matches[4];
			}

			// ***** NEW 24Jan07 (Holger): rewrite words starting with ch:
			else if (preg_match("|^ch:([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//			else if (preg_match("|^ch:([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//			else if (preg_match("|^ch:([^:]+):([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				// NEW 22Apr08 (Hannah): remove the trailing colon (process_Y will do the right thing now anyway
				// if the match is unique, see the changes there)
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query . $matches[1];
				// $completion_link = $query->first_part_of_query . $query->last_separator_of_query . $matches[1] . ":";
				// NEW 07May07 (Holger): remove the clumsy "(the organism)" and the like
				$matches[2] = preg_replace("|[\s_]+\(.*\)|", "", $matches[2]);
				$matches[1][0] = strtoupper($matches[1][0]);
				$completion_show = preg_replace("|_|", " ", $matches[1] . ", " . strtoupper($matches[2]));
				//$completion_show = preg_replace("|_|", " ", $matches[1] . " (" . $matches[2] . ")");
			}

			// ***** NEW 25Jan07 (Holger): rewrite words starting with cr:
			else if (preg_match("|^cr:([^:]+):([^:]+):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//			else if (preg_match("|^cr:([^:]+):([^:]+):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//			else if (preg_match("|^cr:([^:]+):([^:]+):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				$completion_show = "RELATION: " . preg_replace("|_|", " ", $matches[5]);
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query . "cr:" . $matches[1];
			}

			// Rewrite words starting with ch:
			else if (false && preg_match("|^([^:]+):([^:]*):([^:]*):([^:]+):([^:]+)$|", $result->completions[$i], $matches) && $query->debug_mode == 0)
//			else if (false && preg_match("|^([^:]+):([^:]*):([^:]*):([^:]+):([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				// HACK 13Feb07 (Holger): omit ", the ENTITY", it looks stupid
				$completion_show = strcmp($matches[4], "entity") == 0
					? preg_replace("|_|", " ", /*strtolower*/($matches[5]))
					: preg_replace("|_|", " ", $matches[5] . ", the " . strtoupper($matches[4]));
				// NEW 27.10.06 (Markus)
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query
					. "ct:" /* "ce:" */. $matches[4] . ":" . $matches[2] . $matches[1] . $matches[3];
			}

			// Rewrite words like ct:::rubrik.
			else if (preg_match("|^ct:::([^:]+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//			else if (preg_match("|^ct:::([^:]+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//			else if (preg_match("|^ct:::([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				$completion_show = strtoupper($matches[1]);
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query . $matches[1] . ":";
			}

			// Also rewrite cn: like words but without the cn: in the beginning
			// TODO: if it works, just merge with the above by replacing the ^cn: in the preg_match by (^cn:|^)
			// and increasing all occurrences of $matches[x] by $matches[x+1]
			else if (preg_match("|^([^:]+):([^:]*):([^:]*):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//			else if (preg_match("|^([^:]+):([^:]*):([^:]*):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//			else if (preg_match("|^([^:]+):([^:]*):([^:]*):([^:]+):([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
			{
				// NEW 07May07 (Holger): remove the clumsy "(the organism)" and the like
				if (preg_match("|\(.*\)|", $matches[5]))
				{
					$matches[5] = preg_replace("|[\s_]+\(.*\)|", "", $matches[5]);
					$matches[5][0] = strtoupper($matches[5][0]);
				}
				$completion_show = preg_replace("|_|", " ", $matches[5] . ", " /* the " */ . strtoupper($matches[4]));
				// NEW 25Oct06 (Holger): Kurt Mehlhorn, the AUTHOR -> AUTHOR: Kurt Mehlhorn
				//$completion_show = preg_replace("|_|", " ", /*strtoupper*/($matches[4]) . ": " . $matches[5]);
				// NEW 16Sep13 (Hannah): now have an UNDERSCORE between words, so need to add one here.
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query
				. /* "ct:" . */ $matches[4] . ":" . $matches[2] . ($matches[2] != "" ? "_" : "") . $matches[1] . $matches[3]	. $query->append_to_clicked_cat_completion;
				// . /* "ct:" . */ $matches[4] . ":" . $matches[2] . $matches[1] . $matches[3]	. $query->append_to_clicked_cat_completion;
				}

			else if (preg_match("|^cs:([^:]+):([^:]*):([^:]*):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches))
//			else if (preg_match("|^cs:([^:]+):([^:]*):([^:]*):([^:]+):([^:]+):([^:]+)$|", $result->completions[$i], $matches))
			{
				$completion_show = preg_replace("|_|", " ", $matches[6] . ", the " . strtoupper(preg_replace("/s$/", "", $matches[5])));
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query
					. "cs:" . $matches[1] . ":" . $matches[2] . ":" . $matches[3] . ":" . $matches[4] . $query->append_to_clicked_cat_completion; // ":";
				//	. $this->query->completion_end_colon_or_not; // ":";
				// $this->first_part_of_query . $this->last_separator_of_query . $this->last_word_of_query;
				// TODO: cs:... words do not have proper format yet
				//	. "cs:" . $matches[1] . ":" . $matches[2] . ":" . $matches[3] . ":" . $matches[4] . ":";
			}

      // HHHHH
      // Rewrite words starting with ct:
			else if (preg_match("|^ct:([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches)) // && $this->settings->debug_mode == 0)
//			else if (preg_match("|^ct:([^:]+):([^:]+):([^:]+)$|", $result->completions[$i], $matches)) // && $this->settings->debug_mode == 0)
			{
				$completion_show = $query->debug_mode == 0
//				$completion_show = $this->settings->debug_mode == 0
                             ? preg_replace("|_|", " ", $matches[3] . ", " /* the " */ . strtoupper($matches[1]))
                             : $matches[1] . ":" . $matches[2] . ":" . $matches[3];
				// NEW 3Aug06 (Holger): if ct:... typed explicitly, the completion should not be prepended as for the faceted queries
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query . /* "ct:" . */ $matches[1] . ":" . $matches[2]
					. $query->append_to_clicked_cat_completion; // ":";
			}

      // NEW 02Oct07 (Holger): rewrite spelling correction words of type <correct word>::<misspelling>
      else if (preg_match("|^([^:]+)::([^:]+)$|", $result->completions[$i]->string, $matches) && $query->debug_mode == 0)
//      else if (preg_match("|^([^:]+)::([^:]+)$|", $result->completions[$i]->string, $matches) && $this->settings->debug_mode == 0)
//      else if (preg_match("|^([^:]+)::([^:]+)$|", $result->completions[$i], $matches) && $this->settings->debug_mode == 0)
      {
        $completion_show = $matches[2];
        $completion_link = $query->first_part_of_query .  $query->last_separator_of_query . $matches[2]
                            . $query->append_to_clicked_cat_completion; // ":";
      }

			else
			{
				$completion_show = $result->completions[$i]->string;
//				$completion_show = $result->completions[$i];
				$completion_link = $query->first_part_of_query . $query->last_separator_of_query
				                    . $result->completions[$i]->string; // . "$"; // . $query->append_to_clicked_completion;
				// NEW 3Aug06 (Holger): deal properly with phrases added by add-words.pl
				$completion_show = preg_replace("|_|"," ", $completion_show);
				// NEW(Hannah, 12Jul10). We now keep the _ in index words.
				// $completion_link = preg_replace("|_|",".", $completion_link);
			}

			// NEW 22Jul06: links differ only in href part, rest is the same
			// NEW 06.11.06 (Markus): the URL in the case of deactivated javascript was incomplete (now the AC->index_page added)
			$completion_href = $this->javascript
  			? "javascript:AC.completion_link('" . htmlspecialchars($completion_link) . "')"
                        : $this->settings->index_url . $this->settings->index_page . "?query=" . $completion_link;
                        // NEW 11Sep12 (baumgari): Search for the given date to 
                        // find the change description.
                        $completion_onclick = $this->javascript
                          ? "onclick=\"AC.completion_link('" . $completion_link . "', event); return false;\""
                          : "";
			// Truncate very long completions: truncate middle part, not the end
			if (strlen($completion_show) > $query->max_completion_length && $query->max_completion_length > 10) {
				$h = $query->max_completion_length/2;
				$completion_show = preg_replace("|\w+$|","",substr($completion_show, 0, $h)) . "..."
					. preg_replace("|^\w+|","",substr($completion_show, strlen($completion_show) - $h));
			}

      // Note: we use for the horizontal rule between two entries a DIV instead of HR because IE can not set padding/margin to zero
			$r .= '<div class="box_line">'
            . "<span class=\"completion\"><a name=\"box_$type\" class=\"completion_link\" $completion_onclick href=\"$completion_href\">$completion_show</a></span>"
            . "<span class=\"hits_number\">(" . $result->completions[$i]->document_count . ")</span>"
//            . "<span class=\"hits_number\">(" . $result->hits_count_per_completion[$i] . ")</span>"
            . "<div class=\"rule\"></div>"
            . '</div>';
		}

		// NEW 16Jan07 (Holger): also ce:
		$cn = preg_match("|^c[ne]:|", $query->last_word_of_query) ? "cn_" : "";
		$prefix_completed = preg_replace("|^c[ne]:|", "", $query->last_word_of_query);

		// NEW Markus / 08-11-08: this was in process_W
    if ($type == "W")
    {
      // NEW 20Dec07 (Holger): no ct:... words in W-Box for DBLP
      // NEW 21Dec07 (Holger): also no W-box for just single completion
      // Wenn man auf einen der Autoren in der "Refine by AUTHOR" Box klickt, steht im Suchfeld
      //"author:...". Wann immer da was mit Doppelpunkt steht, wird am Anfang
      //ein "ct:" hinzugefügt (siehe query.php), d.h. da steht dann eigentlich
      //"ct:author:...". Dann sind alle completions ebenfalls von der Form
      //"ct:author:..." und die will ich dann nicht anzeigen. (Typischerweise
      //ist das dann eh nur eine, wenn im Eingabefeld z.B.
      //"ct:author:kurtmehlhorn:" steht.
      // NEW Markus / 08-11-08: is now configurable
      // HHHHH
      if (! $this->settings->show_single_word_completion
            && sizeof($result->completions) == 1 && preg_match("|" . $result->completions[0]->string . "|i", $query->last_word_of_query)
          || sizeof($result->completions) > 0 && preg_match("|^cx:|", $result->completions[0]->string))
//      if (sizeof($result->completions) == 2 && preg_match("|".$result->completions[1]."|i", $query->last_word_of_query) ||
//             sizeof($result->completions) > 1 && preg_match("|^ct:|", $result->completions[1]))
      {
         $result->completions_sent = -1;
         $this->log->write("*** " . print_r($result->completions, true), $this->log->levels['INFO'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
      }
    }

		if ($result->completions_sent < 0)
		{
			// NEW 26.03.07 (Markus): provide a title for the completions box if query string is empty (completions_count == -1)
			$completions_title = $this->get_text("completion_title_empty_query");
			$completion_box = $this->get_text("completion_box_empty_query");
		}
		// No completions (then "completions" contains as only element the empty string)
		else if ($result->completions_sent == 0)
		{
      // NEW Markus / 08-11-08: is now for W-box configurable
      if ($type == "W" && ! $this->settings->show_empty_wbox) {
        $completions_title = "";
      }
      else {
  			// NEW 16.10.06 (Markus): in case of empty query don't show category completions box
  			$completions_title = ($cn == "" && $query->query_string != "" ? sprintf($this->get_text("completions_no_result"), $prefix_completed) : "");
      }
			$completion_box = "";
		}
		// Exactly one completion
		else if ($result->completions_sent == 1)
		{
			$completions_title = sprintf ($this->get_text($cn."completions_result_singular"), $prefix_completed);
			$completion_box = $r;
		}
		// Number of existing completions is less than our maximum of computation -> all possible completions are found, show all of them
		// No "more" link is necessary, "less" link is to check
/*
    // CHANGED 16-09-08 / Markus: because of new "top X / all X" link concept (instead of "more" links) the following is no longer necessary
		else if ($result->completions_sent <= $query->max_completions_show)
		{
			$completions_title = sprintf ($this->get_text($cn."completions_result"), $result->completions_sent, $prefix_completed);

			// NEW 21.03.07 (Markus): Less link
			if ($this->javascript)
			{
				if ($result->completions_sent > $this->settings->max_completions_show)	{
					$less_link = "[<a class=\"less_link_$type\" name=\"more_link_$type\" href=\"javascript:AC.more_link('$type',$query_index," . ($query->max_completions_show - $this->settings->more_offset) . ");\">" . $this->get_text("less") . "</a>]";
				}
				else $less_link = "";
			}
			else {
				if ($result->completions_sent > $this->settings->max_completions_show)	{
					$less_link = "[<a class=\"more_link_$type\" name=\"more_link_$type\" href=\""
					. url_append ($this->settings->index_url . $this->settings->index_page, array(
								"qt" => "$this->settings->query_types",
               	"qi" => $query_index,
					     	"mcs" => $query->max_completions_show - $this->settings->more_offset))
					. "\">" . $this->get_text("less") . "</a>]";
				}
				else $less_link = "";
			}
			$completion_box = $r . ($less_link == "" ? "" : "<div class='box_line'>$less_link</div>");
		}
*/
		// There exists more completions than we had computed
		else
		// NOTE Markus / 19-12-08: max_completions_compute is no longer used
//		if ($query->max_completions_compute == 0 || $result->completions_sent <= $query->max_completions_compute)
		{
			$completions_title = sprintf ($this->get_text($cn."completions_larger"), $result->completions_sent, $prefix_completed);

			$top_hits_link = "";
			$all_link = "";

			// NEW 23.01.07 (Markus): "more" link is now language specific and has a link to increase and decrease the number of shown completions
			if ($this->javascript)
			{
				// Javascript mode
        // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more / less" links)

				// "top X" links
				foreach ($this->settings->top_hits as $top_hit)
				{
				  if ($result->completions_total >= $top_hit) {
				    if ($result->completions_sent == $top_hit) {
//				    if ($query->max_completions_show == $top_hit) {
				      $top_hits_link .= "<font class=\"more_link_disabled\">[" . $this->get_text("top") . " " . $top_hit . "]</font> ";
				    }
				    else {
				      $top_hits_link .= "[<a class=\"more_link_$type\" name=\"more_link_$type\" href=\"javascript:AC.more_link('$type',$query_index,$top_hit" . ");\">" . $this->get_text("top") . " " . $top_hit . "</a>] ";
				    }
				  }
				}
				// "all X" link
				// NEW 02-10-08 / Markus: don't show $all_link if number of completions is less than the first topX, for example not "all 2"
				if ($result->completions_total > $this->settings->top_hits[0]
				    && $result->completions_total < $this->settings->top_hits[sizeof($this->settings->top_hits) - 1])
				{
				  if ($result->completions_sent == $result->completions_total) {
//				  if ($query->max_completions_show == $result->completions_total) {
				    $all_link = "<font class=\"more_link_disabled\">[" . $this->get_text("all") . " " . $result->completions_total . "]</font>";
				  }
				  else {
				    $all_link = "[<a class=\"more_link_$type\" name=\"more_link_$type\" href=\"javascript:AC.more_link('$type',$query_index,"
				      . $result->completions_total . ");\">" . $this->get_text("all") . " " . $result->completions_total . "</a>]";
				  }
				}

/*
				// more/less links for Javascript mode
				$more_link = "[<a class=\"more_link_$type\" name=\"more_link_$type\" href=\"javascript:AC.more_link('$type',$query_index," . ($query->max_completions_show + $this->settings->more_offset) . ");\">" . $this->get_text("more") . "</a>]";

				if ($query->max_completions_show > $this->settings->max_completions_show) {
					$less_link = "[<a class=\"less_link_$type\" name=\"more_link_$type\" href=\"javascript:AC.more_link('$type',$query_index," . ($query->max_completions_show - $this->settings->more_offset) . ");\">" . $this->get_text("less") . "</a>]";
				}
				else $less_link = "";
*/
			}
			else {
				// Non-Javascript mode
        // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more / less" links)

				// "top X / all X" links
				foreach ($this->settings->top_hits as $top_hit)
				{
				  if ($result->completions_total >= $top_hit) {
				    if ($result->completions_sent == $top_hit) {
				      $top_hits_link .= "<font class=\"more_link_disabled\">[" . $this->get_text("top") . " " . $top_hit . "]</font> ";
				    }
				    else {
				      $href = $this->create_query_url($query->query_string, $type, $query_index, $top_hit);
				      $top_hits_link .= "[<a class=\"more_link_$type\" name=\"more_link_$type\" href=\"$href\">" . $this->get_text("top") . " " . $top_hit . "</a>] ";
				    }
				  }
				}
				// "all X" link
				// NEW 02-10-08 / Markus: don't show $all_link if number of completions is less than the first topX, for example not "all 2"
				if ($result->completions_total > $this->settings->top_hits[0]
				    && $result->completions_total < $this->settings->top_hits[sizeof($this->settings->top_hits) - 1])
	      {
	        if ($result->completions_sent == $result->completions_total) {
	          $all_link = "<font class=\"more_link_disabled\">[" . $this->get_text("all") . " " . $result->completions_total . "]</font>";
	        }
	        else {
	          $href = $this->create_query_url($query->query_string, $type, $query_index, $result->completions_total);
	          $all_link = "[<a class=\"more_link_$type\" name=\"more_link_$type\" href=\"$href\">" . $this->get_text("all") . " " . $result->completions_total . "</a>]";
	        }
	      }
/*
				// more/less links for non-Javascript mode
				// First the more link
				// NEW 13-08-08 (Markus): query string of URL is now like the hash notation of Javascript mode ("query=xyz&qp=W1.4:F1.4 ...", for example)
				$parameters = array(
												"query" => $query->query_string,
												"qp"    => $this->getQueryParametersAsString($type)
																		. $type . $query_index . "." . ($query->max_completions_show + $this->settings->more_offset));

				$href = url_append ($this->settings->index_url . $this->settings->index_page, $parameters);

				// NEW 08.02.07 (Markus): "name" tag added to refer to the more anker in index.php
				$more_link = "[<a class=\"more_link_$type\" name=\"more_link\" href=\"$href\">" . $this->get_text("more") . "</a>]";

				// Now the less link
				if ($query->max_completions_show > $this->settings->max_completions_show)
				{
					// NEW 13-08-08 (Markus): query string of URL is now like the hash notation of Javascript mode ("query=xyz&qp=W1.4:F1.4 ...", for example)
					$parameters = array(
													"query" => $query->query_string,
													"qp"    => $this->getQueryParametersAsString($type)
																			. $type . $query_index . "." . ($query->max_completions_show - $this->settings->more_offset));

					$href = url_append ($this->settings->index_url . $this->settings->index_page, $parameters);

					// NEW 08.02.07 (Markus): "name" tag added to refer to the more anker in index.php
					$less_link = "[<a class=\"more_link_$type\" name=\"more_link\" href=\"$href\">" . $this->get_text("less") . "</a>]";
				}
				else $less_link = "";
*/
			}

      // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more" links)
      // CHANGED 19-09-08 / Markus: append this only if their are links
      if ($top_hits_link != "" || $all_link != "") {
  			$completion_box = $r . "<div class='box_line'><span class=\"more_link\">$top_hits_link$all_link</span></div>";
//			$completion_box = $r . "<div class='box_line'>" . "<span class=\"more_link\">$more_link</span>&nbsp;$less_link" . "</div>";
      }
      else $completion_box = $r;
		}
		// Too large number of completions
		// Markus / 19-12-08: no longer used, isn't it?
//		else
//		{
//			$completions_title = sprintf ($this->get_text($cn . "completions_too_large"), $query->max_completions_compute, $prefix_completed);
//			$completion_box = "";
//		}

    // NEW 23Jul07 (Holger): different title for special boxes (Y, R, P, etc.)
		if ($completions_title != "" && preg_match("|^Y$|", $type))
    {
      $class = preg_match("|,\s*the\s*([A-Z]+)|", $completion_box, $matches) ?  "INSTANCE" : "CLASS";
      $completions_title = sprintf($this->get_text("completions_result_Y"), $class, $result->completions_total);
    }
		if ($completions_title != "" && preg_match("|^R$|", $type))
    {
      $completions_title = sprintf($this->get_text("completions_result_R"), "RELATION", $result->completions_total);
    }
    if ($completions_title != "" && preg_match("|^P$|", $type))
    {
      $completions_title = sprintf("P BOX (just testing)");
    }
    if ($completions_title != "" && preg_match("|^T$|", $type))
    {
      $completions_title = sprintf("NEW: Translations");
    }

		// We return an empty array if we got no completions title
		// (because of a zero completions result; note that the completion box will disappear)
		// If you want the completion box to stay visible return always an array of an array (with an potential empty title)
		if ($completions_title != "") {
      // NEW 18-11-07 (Markus)
//			if ($query->first_hit == 0) {
//        return array (array("title" => $completions_title, "body" => $completion_box));
//			}
//      else {
        // CHANGED 29-10-08 (Markus)
        return array("title" => $completions_title, "body" => $completion_box, "fh" => $result->first_hit, "sent" => $result->completions_sent);
//        return array("title" => $completions_title, "body" => $completion_box, "fh" => $result->first_hit, "count" => sizeof($result->completions) - 1);
//      }
		}
		else {
			// If facets are shown for an empty query show the completion too (to prevent "jumping" of the F boxes)
//			if ($this->settings->show_facets_for_empty_query && $type == "F") {
//        // NEW 17-01-07 (Markus)
//				return array("title" => "", "body" => "");
//			}
//			else
			return array();
		}
	}


	/**
	 * Build the HMTL formatted W box from the completions array of $result
	 *
	 * @param Result $result the data from which the W box is build
	 * @param Query $query the query for which the W box is build
	 * @return array array consisting of tilte and body of the W box
	 */
	function build_W_boxes (& $result, & $query)
	{
    $this->log->write($result, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$result");
    $this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$query");

    return $this->build_boxes($result, $query, "W", "1");
	}


	/**
	 * Build the HMTL formatted C box from the completions array of $result
	 *
	 * @param Result $result the data from which the C box is build
	 * @param Query $query the query for which the C box is build
	 * @return array array consisting of tilte and body of the C box
	 */
	function build_C_boxes (& $result, & $query)
	{
	  // NEW 15-01-08 (Markus)
//		return $this->build_boxes($result, $query, "C");
		return $this->build_boxes($result, $query, "C", "1");
	}


	/**
	 * Build the HMTL formatted J box from the completions array of $result
	 *
	 * @param Result $result the data from which the J box is build
	 * @param Query $query the query for which the J box is build
	 * @return array array consisting of tilte and body of the J box
	 */
	function build_J_boxes (& $result, & $query)
	{
	  // NEW 15-01-08 (Markus)
//		return $this->build_boxes($result, $query, "J");
		return $this->build_boxes($result, $query, "J", "1");
	}


	/**
	 * Build the HMTL formatted R box from the completions array of $result
	 *
	 * @param Result $result the data from which the R box is build
	 * @param Query $query the query for which the R box is build
	 * @return array array consisting of tilte and body of the R box
	 */
	function build_R_boxes (& $result, & $query)
	{
	  // NEW 15-01-08 (Markus)
//		return $this->build_boxes($result, $query, "R");
		return $this->build_boxes($result, $query, "R", "1");
	}


	/**
	 * Build the HMTL formatted Y box from the completions array of $result
	 *
	 * @param Result $result the data from which the Y box is build
	 * @param Query $query the query for which the Y box is build
	 * @return array array consisting of tilte and body of the Y box
	 */
	function build_Y_boxes (& $result, & $query)
	{
	  // NEW 15-01-08 (Markus)
//		return $this->build_boxes($result, $query, "Y");
		return $this->build_boxes($result, $query, "Y", "1");
	}

  /**
   * Build T box from the completions array of $result.
   */
	function build_T_boxes(& $result, & $query)
	{
		return $this->build_boxes($result, $query, "T", "1");
	}

	/**
	 * Build the HMTL formatted F box from the completions array of $result.
	 * The url mode $url_mode indicates how to decode the query parameters for non-Javascript links.
	 * By default this is "?" which is the normal way and used for all boxes except of P boxes
	 * (which are used by the DBLP website of university of Trier).
	 * The P boxes set $url_mode == "#" to ensure we get this kind of url
	 * which enable history navigating with "back" and "forward" of browser after a jump from Trierer DBLP to MPI.
	 *
	 * @param Result $result the data from which the F boxes are build
	 * @param Query $query the query for which the F box is build
	 * @param string $url_mode the url mode to encode query parameters
	 * @return array array consisting of title and body of the F box
	 */
	function build_F_boxes($result, $query, $query_index, $url_mode = "?")
	{
	  global $log;
	  $log->write ("build_F_boxes: " . print_r($query, true) . print_r($result, true), $log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $log->targets['FILE']);
    $log->write($result, $log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $log->targets['SESSION'], "#$query_index \$result");
    $log->write($query, $log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $log->targets['SESSION'], "#$query_index \$query");
    $log->write($query_index, $log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $log->targets['SESSION'], "#$query_index \$query_index");
    $log->write($url_mode, $log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $log->targets['SESSION'], "#$query_index \$url_mode");
		// NEW 11-04-07 (Markus)
		if ($result->error_message_title != "" || $result->error_message != "") {
			return array (array("title" => $result->error_message_title, "body" => "<div class=\"box_line\">$result->error_message</div>"));
		}
		// Build array of completions for each facet
		$facets = array();

		// Array of booleans to store the information that a facet has more hits than $max_completion_show allows
		$more = array();

    // NEW Markus / 07-07-09: if using sizeof we don't need completion_sent
		$number_of_queries = sizeof($result->completions);
//		$number_of_queries = $result->completions_sent;
		for ($i = 0; $i < $number_of_queries; $i++)
		{
			// Iterate over each facet, "ct:author:wbrucecroft:W._Bruce_Croft", for example
//			$this->log->write ("preg_match: " . $result->completions[$i], $this->log->levels['ERROR']);
		  // NEW 19Dec06 (Holger): make it work also without the ct: in the beginning
			if (preg_match("|^ct:([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches)
        || preg_match("|^([^:]+):([^:]+):([^:]+)$|", $result->completions[$i]->string, $matches))
//			if (preg_match("|^ct:([^:]+):([^:]+):([^:]+)$|", $result->completions[$i], $matches)
//			      || preg_match("|^([^:]+):([^:]+):([^:]+)$|", $result->completions[$i], $matches))
			{
//				$this->log->write ("preg_match: JA -> " . print_r($matches, true), $this->log->levels['ERROR']);
				// $matches[1] contains the facet name, "author", for example
				// $matches[2] contains the facet value, "wbrucecroft", for example
				// $matches[3] contains the string which should be displayed for this facet, "W._Bruce_Croft", for example

				// NEW 09-08-07 (Markus): add new category to facets if it's not contained
				if (! isset($facets[$matches[1]])) {
				  $facets[$matches[1]] = array();
				}

				// Replace the "_"'s by blanks
        // HHHHH
        $completion_show = $query->debug_mode == 0
                            ? preg_replace("|_|", " ", $matches[3])
                            : $matches[1] . ":" . $matches[2] . ":" . $matches[3];

				// NEW 24Jul06 (Holger): truncate very long completions
				if (strlen($completion_show) > $query->max_completion_length){
					$completion_show = substr($completion_show, 0, $query->max_completion_length) . "...";
				}

				// NEW 21Aug06 (Holger): if last words matches refinement, remove last word
				$q = $query->first_part_of_query;
        $query_words = explode(" ", $q);
        $whole = $matches[1] . ":" . $matches[2] . ":";
				foreach ($query_words as $query_word)
        {
          // NEW 03Jan08: now match all utf8 encoded words and make it case-insensitive
          // NEW 04Oct13 (baumgari): wasn't working for facet words, since : and _ were 
          // missing.
          $tmp = my_strtolower(preg_replace("|[^:_0-9A-Za-z\x80-\xff]|", "", $query_word));
          // $tmp could be the empty string when query_words starts with a space,
          //  then the replace would insert a space at every position
          if (strlen($tmp) > 0 && preg_match("|$tmp|", $matches[2])) {
						$q = preg_replace("|\s*$query_word\s*|", " ", $q);
          }
          // NEW 04Oct13 (baumgari) This does not work for facet words: E.g. if we look 
          // for author:wen_gao, there won't be a match, since matches[2] 
          // contains "wen_gao". This means, that we have to make the same test 
          // with $matches[1]:$matches[2]:.
          if (strlen($tmp) > 0 && preg_match("|$tmp|", $matches[1] . ":" . $matches[2] . ":")) {
						$q = preg_replace("|\s*$query_word\s*|", " ", $q);
          }
        }

        $q = preg_replace("|\s*$|", "", $q);
        $q = preg_replace("|^\s*|", "", $q);
				if ($q != "") $q = " " . $q;

				$completion_link = $matches[1] . ":" . $matches[2] . $query->append_to_clicked_cat_completion	. $q;
				$completion_href = $this->javascript
					? "javascript:AC.completion_link('" . htmlspecialchars($completion_link) . "')"
                                        : $this->settings->index_url . $this->settings->index_page . $url_mode . "query=" . $completion_link;
                                // NEW 11Sep12 (baumgari): Added a feature to 
                                // exclude queries by pressing ctrl while 
                                // clicking on the query. This can't be done by 
                                // using javascript:somefunction() as href, 
                                // since we lose any information about other 
                                // events than the click event. Therefore I 
                                // needed to change that to an onclick event. 
                                // The upper href doesn't do anything right now 
                                // at all.
                                $completion_onclick = $this->javascript
                                  ? "onclick=\"AC.completion_link('" . htmlspecialchars($completion_link) . "', event); return false;\""
                                  : "";

        // Note: we use for the horizontal rule between two entries a DIV instead of HR because IE can not set padding/margin to zero
				$completion = "<div class='box_line'>"
				    . "<span class=\"completion\"><a name=\"box_F\" class='completion_link' $completion_onclick href=\"$completion_href\">$completion_show</a></span>"
            . "<span class=\"hits_number\">(" . $result->completions[$i]->document_count . ")</span>"
//            . "<span class=\"hits_number\">(" . $result->hits_count_per_completion[$i] . ")</span>"
            . "<div class=\"rule\"></div>"
				    . '</div>';

				// The array $facets contains for every facet (like "author" for instance) an array with all facets of this type
				// The following adds the facet value to its corresponding group
				// but make sure that each group contains only up to the maximum number of completions that should be shown ($max_completion_show)
                                // HACK(Hannah 22Mai11): for special HomeoVim facets always show all.
                                $showAllCompletions = 0;
                                foreach ($this->settings->facets_to_show as $facet)
                                {
				        $showAllCompletions = ($facet == "monat" || $facet == "quartal" || $facet == "jahr");
                                }
				if (count($facets[$matches[1]]) < $query->max_completions_show || showAllCompletions)
				{
					$facets[$matches[1]][] = $completion;
					// NEW 09-08-07 (Markus): we have to set the following to not get a warning some lines later
					$more[$matches[1]] = false;
				}
				else
				{
					// If we reached here we got the entry of a group one more as allowed by $max_completion_show
					// We don't add it to the facets array but we make a note in the $more array to add later a more link to this facets group
					$more[$matches[1]] = true;
				}
			}
		}

		// NEW 25-01-07 (Markus) / Bug "Christian Jensen" / DBLP
	  // In the special case that facets_to_show is defined in autocomplete_config as array with concrete facets
	  // (i.e. array("author") or array("author", "year") but not array("") (the last one lets compute all facets))
	  // the build_F_boxes() method is called by process_F() for every facet separately and so
	  // we can check whether there are more completions for this facet
		if (is_array($this->settings->facets_to_show)
		    && sizeof($this->settings->facets_to_show) > 0
		    && sizeof($facets) > 0	// NEW 04-03-08 (Markus)
		    && $this->settings->facets_to_show[0] != "")
		{
		  $keys = array_keys($facets);
		  // Set more to true if the completion search found more completions than max_completions_show allows
		  $more[$keys[0]] = ($result->completions_sent > $query->max_completions_show);
		}

		// Now check for each facet group (like $facets["author"], for instance) whether more/less links are to add

		foreach ($facets as $p => $v)
		{
      // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more" links)
//			$more_link = "";
//			$less_link = "";
			$top_hits_link = "";
			$all_link = "";

			// CHANGED 16-09-08 / Markus: for the new "top X / all X" link concept (instead of "more" links) we have to ignore the values of the $more array
//			if ($more[$p])
//      {
				if ($this->javascript)
        {
  				// Javascript mode

  				// "top X" links
   				$i = 0;
  				foreach ($this->settings->top_hits as $top_hit)
  				{
  				  if ($result->completions_total >= $top_hit)
  				  {
  				    if ($result->completions_sent == $top_hit) {
  				      $top_hits_link .= "<font class=\"more_link_disabled\">[" . $this->get_text("top") . " " . $top_hit . "]</font> ";
  				    }
  				    else {
  				      $top_hits_link .= "[<a class=\"more_link_F\" name=\"more_link_F\" href=\"javascript:AC.more_link('F',$query_index," . "$top_hit" . ");\">" . $this->get_text("top") . " " . $top_hit . "</a>] ";
//  				      $top_hits_link .= "[<a class=\"more_link_F\" name=\"more_link_F\" href=\"javascript:AC.more_link('F',$query_index," . "$top_hit" . ");\">" . $this->get_text("top") . " " . $top_hit . "</a>] ";
  				    }
  				  }
  				}
  				// "all X" link
  				// NEW 19-09-08 / Markus: don't show $all_link if number of completions is less than the first topX, for example not "all 2"
  				//  or if the total number of completions is too lanrge (more than the highest value of top_hits)
  				if ($result->completions_total > $this->settings->top_hits[0]
  				    && $result->completions_total < $this->settings->top_hits[sizeof($this->settings->top_hits) - 1])
  				{
  				  if ($result->completions_sent == $result->completions_total) {
  				    $all_link = "<font class=\"more_link_disabled\">[" . $this->get_text("all") . " " . $result->completions_total . "]</font>";
  				  }
  				  else {
  				    $all_link = "[<a class=\"more_link_F\" name=\"more_link_F\" href=\"javascript:AC.more_link('F',$query_index,"
  				       . $result->completions_total . ");\">" . $this->get_text("all") . " " . $result->completions_total . "</a>]";
  				  }
  				}

          // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more" links)
//					$more_link = "[<a class=\"more_link_F\" name=\"more_link_F\" href=\"javascript:AC.more_link('F',$query_index," . ($query->max_completions_show + $this->settings->more_offset) . ");\">" . $this->get_text("more") . "</a>]";
				}
				else
				{
  				// Non-Javascript mode
          // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more / less" links)

   				$top_hits_link = "";

  				// "top X" links
  				foreach ($this->settings->top_hits as $top_hit)
  				{
  				  if ($result->completions_total >= $top_hit) 
  				  {
  				    if ($result->completions_sent == $top_hit) {
  				      $top_hits_link .= "<font class=\"more_link_disabled\">[" . $this->get_text("top") . " " . $top_hit . "]</font> ";
  				    }
  				    else {
   				      $href = $this->create_query_url($query->first_part_of_query, "F", $query_index, $top_hit, $url_mode);
  				      $top_hits_link .= "[<a class=\"more_link_F\" name=\"more_link_F\" href=\"$href\">" . $this->get_text("top") . " " . $top_hit . "</a>] ";
  				    }
  				  }
  				}
  				// "all X" link
  				// NEW 19-09-08 / Markus: don't show $all_link if number of completions is less than the first topX, for example not "all 2"
  				//  or if the total number of completions is too lanrge (more than the highest value of top_hits)
  				if ($result->completions_total > $this->settings->top_hits[0]
      				&& $result->completions_total < $this->settings->top_hits[sizeof($this->settings->top_hits) - 1])
  				{
  				  if ($result->completions_sent == $result->completions_total) {
  				    $all_link = "<font class=\"more_link_disabled\">[" . $this->get_text("all") . " " . $result->completions_sent . "]</font>";
  				  }
  				  else {
  				    $href = $this->create_query_url($query->first_part_of_query, "F", $query_index, $result->completions_total, $url_mode);
  				    $all_link = "[<a class=\"more_link_F\" name=\"more_link_F\" href=\"$href\">" . $this->get_text("all") . " " . $result->completions_total . "</a>]";
  				  }
  				}
          // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more" links)
/*
					// TODO: The query argument is passed here solely for one scenario:
					// when build_F_boxes is called from process_P in the case of DBLP. In
					// that case javascript is set to false. In all other cases, when
					// javascript == false, the effect of having the query= here is that
					// the CompletionServer will be asked to compute a number of things
					// (hits, completions, etc.) which are already known. It's not a big
					// problem, because that is inter-machine communication but it would
					// be desirable to find a cleaner solution at some point.
					// NEW 08-07-08 (Markus): $url_mode == "#" is the mode where we use the hash part of url to decode the query parameters
					//	(is used to enable history navigating with "back" and "forward" of browser)
					if ($url_mode == "?")
					{
						// NEW 14-08-08 (Markus): query string od URL is now like the hash notation of Javascript mode ("query=xyz&qp=W1.4:F1.4 ...", for example)
						$parameters = array(
														"query" => $query->first_part_of_query,
														"qp"    => $this->getQueryParametersAsString("F", $query_index)
																				. "F" . $query_index . "." . ($query->max_completions_show + $this->settings->more_offset));

						$href = url_append ($this->settings->index_url . $this->settings->index_page, $parameters);
					}
					else {
						$href = $this->settings->index_url . $this->settings->index_page . "#" . "query=" . $query->first_part_of_query
							. "&qp=" . "F" . $query_index . "." . ($query->max_completions_show + $this->settings->more_offset);
					}
					$more_link = "[<a class=\"more_link_F\" name=\"more_link_F\" href=\"" . $href . "\">" . $this->get_text("more") . "</a>]";
*/
					}
//      }
/*
      // CHANGED 16-09-08 / Markus: new "top X / all X" link concept above, "less" link is no longer necessary
			if (sizeof($v) > $this->settings->max_completions_show)
			{
				if ($this->javascript) {
					$less_link = "[<a class=\"less_link_F\" name=\"more_link_F\" href=\"javascript:AC.more_link('F',$query_index," . ($query->max_completions_show - $this->settings->more_offset) . ");\">" . $this->get_text("less") . "</a>]";
				}
				else {
				  // Note: unlike for $more_link we don't need to check $url_mode == "#" because we use build_P_boxes only by facet_boxes.php
				  //	(where only the more functionality is needed)
					// NEW 14-08-08 (Markus): query string od URL is now like the hash notation of Javascript mode ("query=xyz&qp=W1.4:F1.4 ...", for example)
					$parameters = array(
													"query" => $query->first_part_of_query,
													"qp"    => $this->getQueryParametersAsString("F", $query_index)
																			. "F" . $query_index . "." . ($query->max_completions_show - $this->settings->more_offset));
          $href = url_append ($this->settings->index_url . $this->settings->index_page, $parameters);
          $less_link = "[<a class=\"less_link_F\" name=\"more_link_F\" href=\"" . $href . "\">" . $this->get_text("less") . "</a>]";
				}
			}
*/
      // CHANGED 16-09-08 / Markus: new "top X / all X" link concept (instead of "more" links)
      // CHANGED 19-09-08 / Markus: append this only if their are links
      if ($top_hits_link != "" || $all_link != "") {
			 $facets[$p][] = "<div class='box_line'><span class=\"more_link\">$top_hits_link$all_link</span></div>";
      }
//			$facets[$p][] = "<div class='box_line'><span class=\"more_link\">" . $more_link . ($more_link != "" && $less_link != "" ? " " : "") . $less_link . "</span></div>";
		}

		// Output facets one after the other (in alphabetic order)
		$rr = array();

		// NEW 21-01-07 (Markus)
		if (sizeof($facets) > 0)
		{
		  $facet_names = array_keys($facets);
		  sort($facet_names);
		  foreach ($facet_names as $facet_name)
		  {
		    $body = implode("", $facets[$facet_name]);
		    //			$body = implode("<br>", $facets[$facet_name]);
		    //			$rr[] = array("title" => sprintf($this->get_text("refine_by"), strtoupper($facet_name)), "body" => "<div name=\"box_F\" class=\"ac_completions\">$body</div>");
		    $rr[] = array("title" => sprintf($this->get_text("refine_by"), strtoupper($facet_name)),
		                  "body" => $body, "fh" => $result->first_hit, "sent" => $result->completions_sent);
//        $count = min($query->max_completions_show, sizeof($result->completions) - 1);
//		    $rr[] = array("title" => sprintf($this->get_text("refine_by"), strtoupper($facet_name)), "body" => $body, "fh" => $result->first_hit, "count" => $count);
		  }
		}
		else {
		  $rr[] = array();
		}
		return $rr;
	}



	/**
	 * Build the HMTL formatted P box from the completions array of $result.
	 * These P boxes are used by the DBLP database of university of Trier. They are very similiar to F boxes
	 * but their links must use '#' instead of '?' (which is necessary for history navigation via back / forwards)
	 *
	 * @param Result $result the data from which the F boxes are build
	 * @param Query $query the query for which the F box is build
	 * @return array array consisting of title and body of the F box
	 */
	function build_P_boxes($result, $query, $query_index)
	{
		return $this->build_F_boxes($result, $query, $query_index, "#");
	}


	/**
	 * Build the HMTL formatted hit box from $result
	 *
	 * @param Result $result the data from which the hit box is build
	 * @param Query $query the query for which the completion box is build
	 * @return array tilte and body for hit_box
	 */
  function build_H_boxes ($result, $query)
  {
    $this->log->write($result, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$result");
    $this->log->write($query, $this->log->levels['DEBUG'], SCRIPT_AC, $query->request_id, __FUNCTION__, __LINE__, $this->log->targets["SESSION"], "\$query");
    $this->log->write("build_H_boxes #" . $query->request_id . " (query #" . $query->id .
          "): '" . $query->query_string . "'"
          . ($this->log->level == $this->log->levels['DEBUG'] ? "  " . print_r($result, true) : "") , $this->log->levels['INFO'], SCRIPT_AC, $query->id, __FUNCTION__, __LINE__, $this->log->targets["FILE"]);

    $hits_title = "";
		$next_hits_url = "";

    // TODO 14-11-08 / Markus: Check this for some weeks, this case should never happen
    if ($result->completions_sent < 0) {
      $this->log->write("ASSERT(\$result->completions_sent < 0) in build_H_boxes #" . $query->request_id . " (query #" . $query->id . " q=" . $query->query_string, $this->log->levels['ERROR'], SCRIPT_AC, $query->id, __FUNCTION__, __LINE__);
    }
    
    // if ($result->hit_sent == 0)
    if ($query->query_string == "" || $query->query_string == "*")
    {
      // NEW 03-04-08 (Markus): the both variables are used to construct the result array. Not setting them results in a PHP warning
      // $first_hit = "";
      // $count = 0;

      $subtitle = sprintf($this->get_text("subtitle_idle"), $result->hits_total);
      $hits_title = "";
      $hits_body = sprintf($this->get_text("hit_box_empty_query"), $result->hits_total);
      
      return array("title" => $hits_title, "body" => $hits_body, "subtitle" => $subtitle, "sent" => $result->hits_sent, "total" => $result->hits_total);
    }

    // NEW 18-11-07 / 02-12-07 / 03-04-08 (Markus)
    // Depending of $this->settings->box_navigation_mode set $first_hit
    // NEW Markus / 15-08-08: Depending on $this->settings->box_navigation_mode ** and ** Javascript mode set $first_hit
    // Markus / 06-07-09
    $first_hit = $query->box_navigation_mode == 'A' && $this->javascript ? $this->first_hit : $result->first_hit;

    // $count is the number of hits shown, not $result->hits_sent.
    //  In case the "more" link for hits is used these both values differ, because $result->hits_sent only counts the
    //  newly computed and sent hits, ignoring these ones already shown
		$count = $result->last_hit - $first_hit + 1;

    // NEW 20Jun07 (Holger) [HACK]: shorten long URLs to avoid shrinking of left column
    // Markus / 27-10-08: it's now a transformation
    // $hits = preg_replace("|>(http:[^<]{50})[^<]{5,}([^<]{50})</a>|", ">$1...$2</a>", $hits);

    // NEW 27Aug11 (Hannah): if few hits, offer fuzzy query instead.
    $SEP = $this->settings->separators;
    $current_query_string = $query->query_string;
    $fuzzy_query_string = preg_replace("/([^$SEP~]+)($|[$SEP])/", "$1~$2", $current_query_string);
    $fuzzy_search_hint = $fuzzy_query_string == $current_query_string ? "" :
                           ", <span style=\"color:black;font-weight:bold\">try the fuzzy search query " .
                           sprintf("<a style=\"color:black;font-weight:bold\" " .
                                   "onclick=\"AC.completion_link('%s', event); return false;\" " .
                                   "href=\"javascript:AC.completion_link('%s');\">%s</a></span>", 
                           htmlspecialchars($fuzzy_query_string), htmlspecialchars($fuzzy_query_string), htmlspecialchars($fuzzy_query_string));

    // If query produces no hits or too many completions: no hits to show
    // NEW 26Jul06 (Holger): max_completions_compute == 0 means always show
    if ($result->hits_total == 0 || $query->max_completions_compute > 0
      && $result->completions_total > $query->max_completions_compute)
    {
      $subtitle = $this->get_text("subtitle_no_hits") . $fuzzy_search_hint;
      $hits_title = ""; // $this->get_text("hits_title_no_hits");
    }
    else if ($result->hits_total == 1)
    {
      $subtitle = sprintf ($this->get_text("subtitle"), $result->hits_total) . $fuzzy_search_hint;
      $hits_title = sprintf ($this->get_text("hits_title_one_hit"));
    }
    else if ($result->hits_total > 1)
    {
      $subtitle = sprintf ($this->get_text("subtitle"), $result->hits_total);

      // Set the hits title
      //
      // Compute the first hit index of the next hits (if the user uses page down)
      // Markus / 06-07-09
      if ($query->first_hit <= $result->hits_total - $query->hits_per_page_while_typing)
      // if ($first_hit <= $result->hits_total - $query->hits_per_page_while_typing)
      {
        $next_hit = $result->last_hit + 1;

        if ($this->javascript)
        {
          $next_hits_url = "javascript:AC.navigation_link($next_hit);";
        }
        else {
					$parameters = array(
													"query" => $query->query_string,
													"qp"    => $this->getQueryParametersAsString("H", 1)
																			. "H1" . "." . $this->settings->hits_per_page_while_typing . "." . $next_hit);
	        $next_hits_url = url_append ($this->settings->index_url . $this->settings->index_page, $parameters);
        }
      }
      else $next_hits_url = "";

      // Compute the first hit index of the previous hits (if the user uses page up)
      if ($first_hit > $this->settings->hits_per_page_while_typing) {
        $previous_hit = $first_hit - $this->settings->hits_per_page_while_typing;
      }
      else {
        $previous_hit = 1;
      }

      if ($first_hit > 1)
      {
        if ($this->javascript)
        {
          $previous_hits_url = "javascript:AC.navigation_link($previous_hit);";
        }
        else {
					$parameters = array(
													"query" => $query->query_string,
													"qp"    => $this->getQueryParametersAsString("H", 1)
																			. "H1" . "." . $this->settings->hits_per_page_while_typing . "." . $previous_hit);
	        $previous_hits_url = url_append ($this->settings->index_url . $this->settings->index_page, $parameters);
        }
      }
      else {
        $previous_hits_url = "";
      }

      // Construct the link for the previous hits
      if ($previous_hits_url == "")
      {
        // There are no previous hits (because we are at the beginning of the hit list)
        // So, set no link for that
        $tmp1 = "";
        $tmp2 = "";
      }
      else
      {
        // There are previous hits in the hit list
        // So, set a link to the next previous ones
        // "name" tag is used to refer to the navigation anker in index.php
        $tmp1 = "<a name='navigation_link' class='hit_title_box' href='$previous_hits_url'>";
        $tmp2 = "</a>";
      }

      // Construct the link for the next hits
      if ($next_hits_url == "")
      {
        // There are no next hits (because we are at the end of the hit list)
        // So, set no link for that
        $tmp3 = "";
        $tmp4 = "";
      }
      else
      {
        // There are next hits in the hit list, set a link to these ones
        // NEW 08.02.07 (Markus): "name" tag added to refer to the navigation anker in index.php
        // NEW 23.08.07 (Markus): "id" tag added to enable refering to this link in unit tests
        $tmp3 = "<a id='next_hits' name='navigation_link' class='hit_title_box' href='$next_hits_url'>";
        $tmp4 = "</a>";
      }

      $hits_title = sprintf ($this->get_text('hits_title'), $first_hit, $result->last_hit,
          $result->hits_total, $query->query_string, $tmp1, "images/arrow_up.gif", $tmp2, $tmp3, "images/arrow_down.gif", $tmp4);
          // $result->hits_count, $query->query_string, $tmp1, "images/arrow_up.gif", $tmp2, $tmp3, "images/arrow_down.gif", $tmp4);
    }

    // NEW 1Sep11 (Hannah): for Stefan.
    if (preg_match("/(fuck|shit)/", $query->query_string)) $subtitle = "Do you have to use so many cuss words?";
    if (preg_match("/connection/", $query->query_string)) $subtitle = "Walter, face it, there isn't any connection.";
    if (preg_match("/benefit/", $query->query_string)) $subtitle = "Like Lenin said, look for the person who will benefit.";
    if (preg_match("/element/", $query->query_string)) $subtitle = "Donny, you're out of your element!";
    if (preg_match("/weekday/", $query->query_string)) $subtitle = "Is this a ... what day is this?";
    if (preg_match("/wrong/", $query->query_string)) $subtitle = "You're not wrong, Walter. You're just an asshole.";
    if (preg_match("/money/", $query->query_string)) $subtitle = "Where's the money, Lebowski?";
    if (preg_match("/sympathy/", $query->query_string)) $subtitle = "I don't need your fuckin' sympathy man, I need my fucking Johnson!";
    if (preg_match("/shabbos/", $query->query_string)) $subtitle = "I don't roll on shabbos!";
    if (preg_match("/female/", $query->query_string)) $subtitle = "Does the female form make you uncomfortable, Mr. Lebowski?";
    if (preg_match("/stepmother/", $query->query_string)) $subtitle = "I'm sorry your stepmother is a nympho.";
    if (preg_match("/larry/", $query->query_string)) $subtitle = "Is this your homework, Larry?";
    if (preg_match("/tie/", $query->query_string)) $subtitle = "That rug really tied the room together, did it not?";
    if (preg_match("/checked/", $query->query_string)) $subtitle = "We're talking about unchecked aggression here, dude.";
    if (preg_match("/undies/", $query->query_string)) $subtitle = "Walter, I'm sure there's a reason you brought your dirty undies.";
    if (preg_match("/moron/", $query->query_string)) $subtitle = "I love you, Walter, but sooner or later you're gonna have to face the fact that you're a goddamn moron.";

    // NEW 2Sep11 (Hannah + Ina): providing he links to the XML, JSON, JSONP result.
    if ($result->hits_total > 0 && $this->settings->session_name == "dblpmirror") // preg_match("/xyz/", $query->query_string))
    {
      $max_num_hits = 1000;
      $query_parameters = "&h=" . $max_num_hits .
                          "&c=" . $this->settings->max_completions_show .
                          "&f=" . ($this->first_hit - 1);
      $query_string_rewritten = htmlspecialchars($query->rewrite_query());  // Adds * in the end, ct: in the front, etc.
      $subtitle .= " ... NEW: get these search results as " .
                   " <a href=\"" . $this->settings->dblp_url . "/search/api/" .
                   "?q=" . $query_string_rewritten . $query_parameters . "&format=xml\">XML</a>" .
                   ", <a href=\"" . $this->settings->dblp_url . "/search/api/" .
                   "?q=" . $query_string_rewritten . $query_parameters . "&format=json\">JSON</a>" .
                   ", <a href=\"" . $this->settings->dblp_url . "/search/api/" .
                   "?q=" . $query_string_rewritten . $query_parameters . "&format=jsonp\">JSONP</a>";
      if ($result->hits_total > $max_num_hits) $subtitle .= "&nbsp;&nbsp;&nbsp;(first " . $max_num_hits . " hits only)";
                   // ", <a href=\"http://www.dblp.org/search/api/?q=" . $query->query_string . "&h=100&format=json\">JSON</a>" .
                   // ", <a href=\"http://www.dblp.org/search/api/?q=" . $query->query_string . "&h=100&format=jsonp\">JSONP</a>";
    }
    // if (preg_match("/xyz/", $query->query_string)) $subtitle = "<div style=\"float:left\">xyz</div><div style=\"float:right\">XYZ</div>";

    // NEW 08.08.06 (Markus): if hits_title is not the empty string add an additional <div></div> to provide an own css style
    if ($hits_title != "")
    {
      $hits_title = "<div id=\"hits_title\">" . $hits_title . "</div>";
    }

    // NEW 19-11-07 (Markus): set the footer of the hit box
    // NEW 21-12-07 (Markus): only if box_navigation_mode == "A"
    $hits_footer = "";
    if ($this->settings->box_navigation_mode == "A")
    {
      if ($next_hits_url != "")
      {
      	// NEW 22-02-08 (Markus): distinguish between JS and non-JS mode
      	if ($this->javascript)
      	{
      		$next_hits_url = "javascript:AC.navigation_link($next_hit, \"A\", $first_hit);";
      	}
      	else {
					// NEW 15-08-08 (Markus): query string of URL is now like the hash notation of Javascript mode ("query=xyz&qp=W1.4:F1.4 ...", for example)
					$parameters = array("query" => $query->query_string,
															"qp"    => $this->getQueryParametersAsString("H", 1)
																			. "H1" . "." . ($next_hit - $first_hit + $this->settings->hits_per_page_while_typing) . "." . $first_hit);
          $next_hits_url = url_append ($this->settings->index_url . $this->settings->index_page, $parameters);
      	}

        // There are another next hits
        // NEW Markus / 29-12-08: an animated progress image while loading next hits and a little more space between more link and page bottom
        $hits_footer = "<dl id='more_hits' style='padding-bottom:30px'>"
							          . "<img style='display: none' src='images/progress_H_more.gif' id='progress_image_H_more'/>"
                        . "<span id='more_hits_link'>"
                        . "[<a name='navigation_link' class='hit_title_box' href='$next_hits_url'>" . $this->get_text('more'). "</a>]"
                        . "</span>"
							          . "</dl>";
      }
    }

    // NEW 12-11-08 (Markus)
    $hits_body = $this->construct_hits_body($result);

    if ($this->settings->hits_envelope != "") {
//      $hits_body = sprintf($this->settings->hits_envelope, $hits_body);
      $hits_body = str_replace("%TEMPLATES%", $hits_body, $this->settings->hits_envelope);
    }

    $this->log->write(print_r($hits_body, true), $this->log->levels['DEBUG'], SCRIPT_AC, "", __FUNCTION__, __LINE__);

    // NEW 21-11-07 (Markus)
    // To prevent long lines (because of URL's which cannot be broken) first use urldecode() (which replace "%20" by space, e.g.)
    // and finally replace some characters ("/", ":", e.g.) which can be used as soft hyphens by the word break tag <wbr />
    $hits_body = preg_replace_callback("/(<i>.*?<a href=.*?\>)(.*?)(<\/a>.*?<\/i>)/s",
        create_function('$matches', '$x = $matches[2]; $x = preg_replace("/([\/:])/", "$1<wbr />", $x); return $matches[1] . $x . $matches[3];'),
        urldecode($hits_body));
    // HACK(bast): rewrite the X X X ...
    // $hits_body = preg_replace("|X  X|", "XX", $hits_body);
    // $this->log->write(print_r($hits_body, true), $this->log->levels['INFO'], SCRIPT_AC, "", __FUNCTION__, __LINE__,
    //                   $this->log->targets["FILE"]);
        
	if ($query->first_hit == 1) {
      return array("title" => $hits_title, "body" => $hits_body . $hits_footer, "subtitle" => $subtitle, "fh" => $first_hit, "sent" => $count, "total" => $result->hits_total);
      // return array("title" => $hits_title, "body" => $hits . $hits_footer, "subtitle" => $subtitle, "fh" => $first_hit, "count" => $count);
		}
    else {
      return array("title" => $hits_title,
                   "body" => $hits_body . $hits_footer,
                   "subtitle" => $subtitle,
                   "fh" => $first_hit,
                   "sent" => $count,
                   "total" => $result->hits_total,
                   "mode" => $query->box_navigation_mode,
                   "xyz" => "xyz");
      // return array("title" => $hits_title, "body" => $hits . $hits_footer, "subtitle" => $subtitle, "fh" => $first_hit, "count" => $count, "mode" => $query->box_navigation_mode);
    }
  }



  function construct_hits_body($result)
  {
    // $this->log->write("construct_hits_body" . print_r($result, true), $this->log->levels['DEBUG'], SCRIPT_AC);

    $hits_body = "";
    $hits = & $result->hits;

    // NEW 07Nov07 (Holger): from within MPII directly link to papers for dblp-plus
    if ($this->settings->session_name == "dblpplus" && strpos($_SERVER["REMOTE_ADDR"], "139.19.") === 0)
    {
      $hits = preg_replace("|dblp/show.php\?key=([^\"]+)|", "dblp-plus/archive/$1.pdf", $hits);
    }

    // Depending on the config parameter how_to_rank_docs set $ascending_sort if the hit documents are to be sort in ascending order
    $ascending_sort = substr($this->settings->how_to_rank_docs, 1, 1) == "d";

    // $hit_index is an optional index for supporting the keyword %INDEX% may be contained in the hit template
    $hit_index = $ascending_sort ? 1 : $result->hits_total - $result->first_hit + 1;

    // Set the hit template from settings
    // If hit template is a string it contains simply the template.
    // If it is an array the template provides a grouping (according to year, for example).
    // In this case the first element is the template itself, the second one contains the entity to group ("YEAR", for example),
    // and the third one is the pattern to format the group title ("<bd>%YEAR%</bd>", for example)
    if (is_array($this->settings->hit_template))
    {
      $hit_template = $this->settings->hit_template[0];
      if (count($this->settings->hit_template) > 1)
      {
        $group = strtolower($this->settings->hit_template[1]);
        $cgv = "";  // current group value
        $this->log->write("Set hit group to '$group'", $this->log->levels['DEBUG'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
      }
    }
    else {
      $hit_template = $this->settings->hit_template;
    }

    // Now iterate over every hit to construct the string representing this hit according to the hit template
    foreach ($hits as $hit)
    {
      $hit_presentation = $hit_template;

      // If grouping is provided and the current group value differ from the group value of this hit
      //  we have to insert an extra line with the new group
      if (isset($group) && $hit->title[$group] != $cgv) {
        $this->log->write("New group '" . $hit->title[$group] . "'" , $this->log->levels['DEBUG'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
        $hit_presentation = $this->settings->hit_template[2] . $hit_presentation;
        $cgv = $hit->title[$group];
      }

      // Construct the string presentation of the hit
      //  by replacing the %x%-constructs with its corresponding members from the hit object
      $this->construct_hit_presentation($hit, $hit_presentation);
      // $this->log->write("hit_presentation: $hit_presentation", $this->log->levels['DEBUG'], SCRIPT_AC);

      // Finally set the optional index for enumerate hits
      //  Doing this at the end prevent us from overriding an user defined "%INDEX% in the hit template
      $hit_presentation = str_replace("%INDEX%", $ascending_sort ? $hit_index++ : $hit_index--, $hit_presentation);

      // NEW 29Nov12 (baumgari): It can happen, that some variables cannot be 
      // filled. Those should be erased.
      $hit_presentation = preg_replace('/%.+?%/', '', $hit_presentation);
      
      $hits_body .= $hit_presentation;
    }

    $this->log->write("hits_body: $hits_body", $this->log->levels['DEBUG'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
    return $hits_body;
  }



  /**
   * Construct the string presentation of the hit
   * by replacing the %x%-constructs with its corresponding members from the hit object
   *
   * @param unknown_type $hit
   * @param unknown_type $s
   * @param unknown_type $delimiter
   */
  function construct_hit_presentation($hit, &$presentation, $delimiter = ", ")
  {
    // Note that $hit here can be an object (like in the first level call)
    //  or an array (like in the recursive calls of this method).
    //  At least in PHP4 this don't result neither in error nor in warning.
    //  Indeed, arrays and objects are very similiar in PHP
    foreach ($hit as $attribute => $value)
    {
      // If no transformation function exists we check whether $hit_value is an array
      //  and whether we can apply transformation to its elements
      if (! $this->transform("hit", $attribute, $value, $hit))
      {
        if (is_array($value))
        {
          if (is_sequence($value))
          {
            $t = "";
            $i = 1;
            $n = count($value);
            foreach ($value as $v) {
              // If $attribut has a plural "s" check whether $v has a transformation function and apply it if so
              $k = strlen($attribute);
              if (substr($attribute, $k - 1) == "s") {
                $this->transform("hit", substr($attribute, 0, $k - 1), $v, $hit);
              }
              $t .= $v . ($i++ < $n ? $delimiter : "");
            }
            $presentation = str_replace("%" . strtoupper($attribute) . "%", $t, $presentation);
          }
          else {
            $this->construct_hit_presentation($value, $presentation, $delimiter);
          }
        }
        else {
          $presentation = str_replace("%" . strtoupper($attribute) . "%", $value, $presentation);
        }
      }
      else {
        // When we arrive here all possible transformations were applied
        //  and we can replace the according %-construct by the (potentially) changed $hit_value
        $presentation = str_replace("%" . strtoupper($attribute) . "%", $value, $presentation);
        if (is_array($value)) $this->log->write("construct_hit_presentation: " . print_r($value, true), $this->log->levels['WARNING'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
      }
    }
  }
/*
  function construct_hit_presentation2($hit, &$presentation, $delimiter = ", ")
  {
    // Note that $hit here can be an object (like in the first level call)
    //  or an array (like in the recursive calls of this method).
    //  At least in PHP4 this don't result neither in error nor in warning.
    //  Indeed, arrays and objects are very similiar in PHP
    foreach ($hit as $attribute => $value)
    {
      // If no transformation function exists we check whether $hit_value were is array
      //  and whether we can apply transformation to its elements
      if (! $this->transform("hit", $attribute, $value, $hit))
      {
        if (is_array($value))
        {
          if (is_sequence($value))
          {
            $t = "";
            $i = 1;
            $n = count($value);
            foreach ($value as $v) {
              // If $attribut has a plural "s" check whether $v has a transformation function and apply it if so
              $k = strlen($attribute);
              if (substr($attribute, $k - 1) == "s") {
                $this->transform("hit", substr($attribute, 0, $k - 1), $v, $hit);
              }
              $t .= $v . ($i++ < $n ? $delimiter : "");
            }
            $value = $t;
          }
          else {
            $this->construct_hit_presentation($value, $presentation, $delimiter);
          }
        }
      }

      // When we arrive here all possible transformations were applied
      //  and we can replace the according %-construct by the (potentially) changed $hit_value
      if (! is_array($value)) {
        $presentation = str_replace("%" . strtoupper($attribute) . "%", $value, $presentation);
      }
//      else $this->log->write("construct_hit_presentation: " . print_r($value, true), $this->log->levels['WARNING'], SCRIPT_AC);
    }
  }
*/


  /**
   * Enter description here...
   *
   * @param unknown_type $item
   * @param unknown_type $language
   * @return unknown
   */
	function get_text($item, $language = false)
	{
		if (! $language) {
			$language = $this->settings->language;
		}

		if (array_key_exists($item, $this->text))
		{
			if (array_key_exists($language, $this->text[$item])) {
				$r = $this->text[$item][$language];
			}
			else if (array_key_exists($this->default_settings->language, $this->text[$item]))
			{
				$r = $this->text[$item][$this->default_settings->language];
				$this->log->write("Text pattern for $item" . "[" . $language . "] is missing (AC->text)", $this->log->levels['ERROR'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
//				$this->log("Text pattern for $item" . "[" . $language . "] is missing (AC->text)", AUTOCOMPLETE_LOGLEVEL_ERROR);
			}
		}
		else if (array_key_exists($item, $this->js_text))
		{
			if (array_key_exists($language, $this->js_text[$item])) {
			$r = $this->js_text[$item][$language];
			}
			else if (array_key_exists($this->default_settings->language, $this->js_text[$item]))
			{
				$r = $this->js_text[$item][$this->default_settings->language];
				$this->log->write("Javascript text pattern for $item" . "[" . $language . "] is missing (AC->js_text)", $this->log->levels['ERROR'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
//				$this->log("Javascript text pattern for $item" . "[" . $language . "] is missing (AC->js_text)", AUTOCOMPLETE_LOGLEVEL_ERROR);
			}
		}
		if (!isset($r))
		{
			// To prevent recursive calls of get_text we use the following text without translation from $AC->text array
			$this->log->write("Text pattern for $item is missing", $this->log->levels['ERROR'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
			$r = "Text pattern for &quot;$item&quot; is missing";
		}

		/*
		$r = $this->text[$item][$language];
		if ($r == "")
		{
			@$r = $this->text[$item][$this->default_settings->language];
			if ($r == "")
			{
				@$r = $this->js_text[$item][$language];
				if ($r == "")
				{
					@$r = $this->js_text[$item][$this->default_settings->language];
					if ($r == "")
					{
						// To prevent recursive calls of get_text we use the following text without translation from $AC->text array
						$this->log("Text pattern for $item is missing", AUTOCOMPLETE_LOGLEVEL_ERROR);
						$r = "Text pattern for $item is missing";
						//return "Text pattern for $item is missing";
					}
				}
			}
		}
		*/
		// NEW 25Oct06 (Holger): make fit for utf-8
		// NEW 06.11.06 (Markus): use now an own charset encoding method
		// NEW 23.05.07 (Markus): the texts are now saved utf-8 encoded, so they have to be decoded if encoding is not utf-8
		return (strtolower($this->settings->encoding) == "utf-8") ? $r : utf8_decode($r);
//		return $this->charset_encode($r);
	}


	/**
	 * Rewrite join queries
	 * TODO what about rewrite_joins()?
	 *
	 * @param array $regs
   * @return string the query block rewritten
	 */
	function rewrite_joins_NEW($regs)
	{
	  $join_word = $regs[1];
	  $attr_words = explode(" ", $regs[2]);
	  $this->log("INSIDE REWRITE JOINS (" . $join_word . " : " . implode(" ; ",$attr_words) . ")");
	  foreach ($attr_words as $p => $v) $attr_words[$p] .= " " . $join_word;
	  return "[" . $join_word . "*#" . implode("*#", $attr_words) . "]"; // was "]$"
	}


  /**
   * Rewrite query block of type "auth[sigmod sigir]"
   *
   * @param array $regs
   * @return string the query block rewritten
   */
	function rewrite_joins($regs)
	{
	  $block = $regs[0];
	  $this->log->write("in rewrite_joins with block \"" . $block . "\"", $this->log->levels['DEBUG'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
	  //		$this->log("in rewrite_joins with block \"" . $block . "\"", AUTOCOMPLETE_LOGLEVEL_DEBUG);
	  $pos_beg = strrpos($block, "[");
	  $pos_end = strrpos($block, "]");
	  $join_word = substr($block, 0, $pos_beg);
	  $attr_words = explode(" ", substr($block, $pos_beg+1, $pos_end-$pos_beg-1));
	  //$this->log($join_word . " *** " . implode("|", $attr_words));
	  foreach ($attr_words as $p => $v) $attr_words[$p] .= " " . $join_word . "*"; // Ingmar wants * here
	  return "[" . $join_word . "*#" . implode("#", $attr_words) . "]"; // Ingmar wants $ here // now added above
	}


  /**
   * Rewrite synonymy queries
   *
   * @param array $regs
   * @return string
   */
	function rewrite_tildas($regs)
  {
    $query = new Query();
    $query->query_string = "s:" . $regs[1] . ":";
    $this->log->write("SYNMODE = 2, query : " . $query->query_string, $this->log->levels['INFO'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
    $query->max_completions_show = 1;
    $query->max_completions_compute = 1;
    $query->hits_per_page_while_typing = 0;
		// CHANGED 26-02-08 (Markus)
    $query->first_hit = 1;
    $query->first_hit_shown = 1;
//    $query->first_hit = 0;
//    $query->first_hit_shown = 0;
    $query->excerpt_radius = 0;
    $query->excerpts_per_hit = 0;
    $query->display_mode = 0;
		$this->log->write("Query S : \"" . $query->query_string . "\"", $this->log->levels['INFO'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
    $result = $this->process_single($query);
    if ($result->completions_sent >= 1)
    {
      preg_match("|^s:[^:]+:(.*)$|", $result->completions[1], $matches);
      $synset = "s:" . $matches[1] . ":";
      $this->log->write("*** RESULT: \"" . $synset . "\"", $this->log->levels['INFO'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
      return $synset;
    }
    else
    {
      return $regs[1];
    }
  }


  /**
   * Returns a string in non-Javascript URL notation (the part after "qp=" in "qp=H1.4W1.4F1.4F2.4F3.4", for example)
   * 	for the default query
   *
   * @return string the string for the non-Javascript URL notation
   */
  function getDefaultQueryParametersAsString()
  {
  	$result = "";

  	for ($i = 0; $i < strlen($this->default_settings->query_types); $i++)
  	{
  		$type = $this->default_settings->query_types[$i];
  		if ($type == "H") {
				$result .= "{$type}1.{$this->settings->hits_per_page_while_typing}";
  		}
  		// CHANGED 06-10-08 / Markus: type "P" added
  		else if ($type == "F" || $type == "P") {
  			if ($this->default_settings->facets_to_show) {
  				for ($j = 1; $j <= sizeof($this->default_settings->facets_to_show); $j++) {
  					$result .= "{$type}$j.{$this->settings->max_completions_show}";
  				}
  			}
  		}
  		else {
  			$result .= "{$type}1.{$this->settings->max_completions_show}";
  		}
  	}
  	return $result;
  }


  /**
   * Returns a string which represents the query parameters of all current queries (except of this one specified in $except)
   * 	in the new URL notation ("W1.1:H1.8:F1.4:F2.4:F3.4", for example)
   *
   * @param string $except_type The type of the query which should be excepted from the resulting string (optional); "W", for example
   * @param string $except_index The index of the query which should be excepted from the resulting string (optional); "1", for example
   * @return string The string represention of the current queries
   */
  function getQueryParametersAsString($except_type = "", $except_index = 1)
  {
  	$result = "";

  	foreach ($this->queries as $query)
  	{
 		  $qt = $query->query_types;
 		  // NEW 06-10-08 / Markus: Since P box is special kind of F box, every click inside a P box has to be performed as a F query
 		  if ($qt == "P") $qt = "F";

  		if (strlen($qt) == 1 	// it must be a single query type ("W" and not "WHF", for example)
  				&&
  				($qt != $except_type || $query->query_index != $except_index))
  		{
  			$result .= "{$qt}{$query->query_index}.{$query->max_completions_show}"
  								. ($query->first_hit == 1 ? "" : ".{$query->first_hit}")
 									. ":";
  		}
  	}
  	return $result;
  }


  /**
   * Enter description here...
   *
   * @param unknown_type $history
   * @param unknown_type $query_string
   */
  /*
  function history_navigate($query_string, $inactivity)
  {
    $history = & $this->history;
    $query_string = rtrim($query_string);

    if ($query_string == $this->last_query_string) {
	    $this->log->write("History: entry = '$query_string' [inactivity=$inactivity] --> abort (already in history)", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
    	return;
    }

    $this->log->write("History: entry = '$query_string' [inactivity=$inactivity]", $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
    // NEW 06-03-08 (Markus): Special handling if a temporary entry resides on the last position
    // Note: $history->last_temporary indicates whether the last element of history is temporary or not
    if ($inactivity < 1000)
    {
      // If the last inserted element is temporary overide it
      $history->elements[$history->pos] = $query_string;
    }
    else
    {
      // Save new query string in the query history array

//      if (!isset($history->elements[$history->pos])) {
//        array_push($history->elements, $query_string);
//      }
//      else
      if ($query_string != $history->elements[$history->pos])
      {
      	// Is it the previous element in history, i.e. we are navigation forwards
        if ($history->pos > 0 && $query_string == $history->elements[$history->pos-1]) {
          $history->pos--;
        }
        else
        {
          $history->pos++;
          if ($history->pos < count($history->elements))
          {
            if ($query_string == $history->elements[$history->pos]) {
              // We are moving forwards in the history
              // Nothing to do because the kth entry is equal to the query string
            }
            else {
            	// We are in the history but write the end new ;-)
              $history->elements[$history->pos] = $query_string;
              array_splice($history->elements, $history->pos+1);
            }
          }
          else {
            // The query string is new to the history, append it
            $history->elements[$history->pos] = $query_string;
//            array_push($history->elements, $query_string);
          }
        }
      }
      else {
        // Same query string as the last one, nothing to do
      }
    }

    // Save whether this entry is temporary or not
//    $history->last_temporary = $last_temporary;
    $this->log->write("History: " . print_r($history, true), $this->log->levels['INFO'], SCRIPT_AC, $this->query->request_id);
  }
*/

	/**
	 * Encode string $s using encoding $encoding
	 *
	 * @param string $s string to encode
	 * @param string $encoding encoding, 'utf-8', for example
	 * @return string the encoded string
	 *
	 * Note: the only supported encoding is UTF-8
	 */
	function charset_encode ($s, $encoding = "")
	{
		if ($encoding == "") $encoding = & $this->settings->encoding;

		if (strtolower($encoding) == "utf-8") {
			return utf8_encode($s);
		}
		else return $s;
	}


	/**
	 * Decode string $s using encoding $encoding
	 *
	 * @param string $s string to decode
	 * @param string $encoding encoding, 'utf-8', for example
	 * @return string the decoded string
	 *
	 * Note: the only supported encoding is UTF-8
	 */
	function charset_decode ($s, $encoding = "")
	{
		if ($encoding == "") $encoding = & $this->settings->encoding;

		if (strtolower($encoding) == "utf-8") {
			return utf8_decode($s);
		}
		else return $s;
	}


	/**
	 * Sending an e-mail
	 *
	 * @param string $empfänger
	 * @param string $betreff
	 * @param string $text
	 * @param string $absender
	 */
	function send_email ($empfänger, $betreff, $text, $absender)
	{
		mail ($empfänger, $this->settings->html_title . ": $betreff", $text, "From: $absender");
	}


	/**
	 * Create and return the url to compute a query
	 *
	 * @param string $query_string the query string
	 * @param string $type type of query, "W" or "F", for example
	 * @param integer $query_index the index of query, 2 for the second F box, for example
	 * @param integer $count the number of completion to be computed
	 * @return string the resulting url
	 */
	function create_query_url($query_string, $type, $query_index, $count, $url_mode = "?")
	{
/*
	  // NEW 13-08-08 (Markus): query string of URL is now like the hash notation of Javascript mode ("query=xyz&qp=W1.4:F1.4 ...", for example)
	  $parameters = array(
    	  "query" => $query_string,
    	  "qp"    => $this->getQueryParametersAsString($type) . $type . $query_index . "." . $count);

	  return url_append ($this->settings->index_url . $this->settings->index_page, $parameters);
*/
    // COMMENT 06-10-08 / Markus: the use of getQueryParametersAsString() is necessary to provide bookmarkíng
    //  of the site in Javascript disabled browsers (because the url must represent the complete request history)
		return $this->settings->index_url . $this->settings->index_page . $url_mode . "query=" . $query_string
			. "&qp=" . $this->getQueryParametersAsString($type) . "$type$query_index.$count";
//			. "&qp=" . "$type$query_index.$count";
	}


	/**
	 * Enter description here...
	 *
	 * @param string $entity
	 * @param string $key
	 * @param reference $object
	 * @param Hit $hit
	 * @return bool true if a transformation function was applied, false no such a function exists
	 */
  function transform($entity, $key, &$object, $hit)
  {
    $function_name = "transform_" . $entity . "_" . $key;
//    $this->log->write( "transform $entity / $key", $this->log->levels['DEBUG']);

    $this->log->write( "Check whether $function_name exists ...", $this->log->levels['DEBUG'], SCRIPT_AC, "", __FUNCTION__, __LINE__);

    if (is_callable($function_name)) {
      $this->log->write( "Call $function_name", $this->log->levels['DEBUG'], SCRIPT_AC, "", __FUNCTION__, __LINE__);
      // NEW 05Sep12 (baumgari): call_user_func doesn't allow pass by reference 
      // any more. Nevertheless there is a workaround using call_user_func_array 
      // and "hide" the reference within the array. There are other 
      // possibilities like strictly enabling it:
      // allow_call_time_pass_reference = On within the ini file. Anyway I 
      // didn't wanted to do it globally.
      call_user_func_array($function_name, array(&$object, $hit));
      //call_user_func($function_name, $object, $hit);
      return true;
    }
    else return false;
  }

  // End of class AC
}


?>
