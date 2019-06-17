<?php

/**
 * ALL APPLICATION SPECIFIC PARAMETERS.
 * They are set and saved in the stdClass object $config.
 * 
 * ALL PARAMETERS ARE USER-EDITABLE (but change only right hand sides!)
 */


// Use an own error handling (instead of the PHP standard error handler which outputs error message onto the page)
// If you have trouble to get error messages set this to false
$config->use_own_error_handling = true;


//
// *** PATHS, URLS AND MORE (names, title, encoding ...) ***
//

// The path to the application folder (relative to web server's document root, with leading and trailing slash!)
// For example:
//$config->path = "dblp-mirror";

// The path to the autocomplete folder (relative to web server's document root, with leading and trailing slash!)
$config->autocomplete_path = "/autocomplete-php/autocomplete/";

// *Full* path name of log files (replace by your own)
//$config->access_log = $_SERVER['DOCUMENT_ROOT'] . $config->path . "access.log";
//$config->error_log = $_SERVER['DOCUMENT_ROOT'] . $config->path . "error.log";
//$config->access_log = "/var/opt/completesearch/log/completesearch.access_log";
//$config->error_log = "/var/opt/completesearch/log/completesearch.error_log";
$config->access_log = "/var/log/dblp/access.log";
$config->error_log = "/var/log/dblp/error.log";


// Base URL of the DBLP page; specific for the DBLP collection
$config->dblp_url = "http://www.panarea.informatik.uni-freiburg.de";

// The Name of the CSS file
$config->css_file = "autocomplete.css";

// The name for this application (must be unique over all applications on this server which are using autocomplete functionality.
// The session name is created from this name, this name appears in log file messages.
$config->application_name = "dblpmirror";

// The title of the main page (used for the HTML title-tag)
$config->html_title = "CompleteSearch DBLP";

// File name of the index page
$config->index_page = "index.php";

// Encoding (iso-8859-1 or utf-8)
$config->encoding = "utf-8";
//$config->encoding = "iso-8859-1";


// base URL of the DBLP page
$config->dblp_url = "http://www.dblp.org/db";
// Custom text (absolute path or relative to this directory)
$config->custom_text_file_name = "custom_text.php";



//
// *** COMPLETION SERVER ***
//

// Host name of completion server
$config->server_hostname = "panarea";

// Port of the completion server to which to talk to
// For example:
// $config->server_port = rtrim(file_get_contents("/var/opt/completesearch/ports/dblp.port"));
 $config->server_port = 8181;

// Timeout for connecting to the completion server via socket (in seconds)
$config->server_timeout = 15;


//
// *** GUI PARAMETERS (defaults that can be changed interactively) ***
//

// Default language of GUI
$config->language = "en";
//$config->language = "de";


//
// *** GUI PARAMETERS (cannot be changed interactively) ***
//

// Don't start search before this many characters typed
$config->min_query_size = 1;

// No completions for words shorter than this
$config->min_prefix_size = 1;

// Append star only if prefix longer than this
$config->min_prefix_size_to_append_star = 2;

// Assume implicit * after every word if non-zero
$config->always_prefix = 1;

// Delays after each keystroke before launching query (for prefixes of size 0, 1, 2, 3, ...)
$config->delays = array(0, 1000, 1000, 500, 200);

// Show only selected facets (in the specified order), if array("") show all facets in alphabetical order
// ATTENTION: the second mode - array("") for all facets - is no longer supported
// (because of the new URL parameter concept "#query=info&qp=H1.20:W1.4:F1.4", for example)
$config->facets_to_show = array("author", "venue", "year", "type");

// Show facets for empty query
$config->show_facets_for_empty_query = true;

// If there is only a single completion in the W box, show it only if the following is true                                  
$config->show_single_word_completion = false;                    

// If there is no completion in the W box, show it only if the following is true                                  
$config->show_empty_wbox = false;

// String which encodes the kinds of queries should be processed (query types), for example "HWF"
// (H = hit, W = word, J = join, C = category, F = faceted, Y = yago, R = yago (relation), P = *p*recomputed DBLP facets, X = alternative to Y )
$config->query_types = "HWF";

// Synonym mode (0 = leave query as is, 1 = expand words with trailing ~, 2 = expand all words in query)
$config->synonym_mode = 0;

// Debug mode (0 = normal operation, 1 = do not rewrite completions)
$config->debug_mode = 0;

// An alternative kind of "more" link in the completions boxes: an array of "top X" values.
// For example, "array(5, 50, 250)" offers the top (first) 5, 50, and 250 completions
$config->top_hits = array(4, 50, 250);

// Not longer supported: Show this much more/less when clicking on [more]/[less]
// $config->more_offset = 4;

// Mode to navigate through entries of boxes (A = append to previous hits, R = replace previous hits)
$config->box_navigation_mode = "A";

// What string to append when clicking a completion
$config->append_to_clicked_completion = "$";

// What string to append to ct/cn completions
$config->append_to_clicked_cat_completion = ":";

// Display mode (1 = title + excerpts + url, 2 = same, but one line per excerpt, 3 = same, but no url)
$config->display_mode = 3;

// Number of completions per box
$config->max_completions_show = 4;

// Number of hits per page
$config->hits_per_page = 20;

// Number of excerpts per hit
$config->excerpts_per_hit = 1;

// Number of words displayed to the left and right of highlighted words in excerpts
$config->excerpt_radius = 20;

// Maximum length of a completion (rest is truncated and shown as ...)
$config->max_completion_length = 80;

// Auto scroll for the hits: how pre-active the "more hits"-action is triggered is determined by the following value
// The value indicates the part of the scrollable area the user has to be scrolled through before autoscrolling should be startet; 
// the value ranges from 0 (means 0%) to 1 (means 100%) where the value "0" has a special meaning: the autoscroll functionality is disabled
// A value of "1" means that system start autoscroll not until before the user reached the bottom
$config->hits_autoscroll_threshold = 0.9;

// Allowed values are "0" and "1" (default is "1").
// If "1" hit requests are sent until the hit box is full of hits or all hits are shown and scrollbars appear
$config->hits_autofill = 0;

// The amount to increase/decrease number of shown completions when user click more/less
//$this->more_offset = $config->hits_per_page;

// The log level default (1 = fatal, 2 = error, 3 = warning, 4 = info, 5 = debug)
$config->log_level = 2;

// How to rank documents, the digit stands for the kind of sorting, the letter for a / d for ascending / descending
$config->how_to_rank_docs = "1a";

// How to rank words (completions), the digit stands for the kind of sorting, the letter for a / d for ascending / descending
$config->how_to_rank_words = "1d";

// Default parameters (which can be changed by user on the options page)
// NOTE: max_completions_show is now replaced by the first element of top_hits
$config->user_preferences = array("max_completions_show",
                                  "hits_per_page", 
                                  //"excerpts_per_hit",
                                  //"excerpt_radius",
                                  //"query_types",
                                  //"display_mode",
                                  //"log_level",
                                  //"synonym_mode",
                                  //"debug_mode"
                                  );

// Here you can override the general texts defined in text.php
// For example:
// $config->text["options_title"] = array
//	(
//		"de" => "Optionen (D)",
//		"en" => "Options (EN)"
//	);

// The following option should only be set if we have sure information about the browsers allowing javascript or not.
//  For examples the use of this option is necessary in the following cases:
//  - true for facetboxes.php (the DBLP application) and the SeleniumRC tests for javascript enabled tests
//    (because both do only work with javascript enabled)
//  - false for the SeleniumRC tests for javascript disabled tests
//  Be very **careful** in setting this option in all other cases! If true browsers with Javascript disabled don't work.
//  So in the most cases this option should not be set! 
//  On the other hand if we can be sure that javascript is enabled it is useful to set the option to true 
//  because this prevents the reload of the page after a reset or if the session timed out
//$config->javascript = true;
//$config->javascript = false;


//
// *** This is for formatting the hits ***
//

// The hit template can be a string or an array of three strings.
// If hit template is a string it contains simply the template.
// If it is an array the template provides a grouping (according to year, for example).
// In this case the first element is the template itself, the second one contains the entity to group ("YEAR", for example),
// and the third one is the pattern to format the group title ("<bd>%YEAR%</bd>", for example)
//
// To apply additional transformations to the %ENTITY% constructs below add transformation functions in "transformations.php"

$config->hit_template = array(
  "<tr>" .
  "<td align=\"right\" valign=\"top\" bgcolor=\"%TYPE%\"><a href=\"%URL%\">%INDEX%</a></td>" .
  "%TITLE_EE%" .
  "<td>%AUTHORS%: %TITLE%<a href=\"http://www.dblp.org/%VENUE_URL%\">%VENUE%</a></td>" .
  "</tr>",
  "YEAR",
  "<tr><th bgcolor=\"#ffffcc\" colspan=\"3\" align=\"center\">%YEAR%</th></tr>");        


// This is to embed the hits in an "envelope", e.g. surround by the following string; "%s" is replaced by the hits
// For example: $config->hits_envelope = "<table border=\"1\">%s</table>";
// The following is the default, so we need nothing to declare:
// $config->hits_envelope = "";
$config->hits_envelope = "<table border=\"1\">%TEMPLATES%</table>";


// Further examples
// ================
// DBLP
// ----------------
// $config->hit_template = "<a href='%URL%'><p class='title'>%TITLE%</p></a><font class='authors'>%AUTHORS%</font>"
//        . "<br><font class='venue'>%VENUE%</font> <font class=\"type\">[%TYPE%]</font> (<font class=\"year\">%YEAR%</font>)"
//        . "<div class='excerpts'>%EXCERPTS%</div>";

// Wikipedia        
// ----------------
// $config->hit_template = "<a href='%URL%'><font class='title'>%TITLE%</font></a>"
//        . "<div class='excerpts'>%EXCERPTS%</div>"
//
// $config->hits_envelope = "";

?>
