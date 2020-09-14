<?php
/**
 * All application specific parameters are set and saved in the stdClass object $config
 */

error_reporting(E_ALL);

// --- ALL PARAMETERS ARE USER-EDITABLE (but change only right hand sides!)

// Use an own error handling (instead of the PHP standard error handler which outputs error message onto the page)
// If you have trouble to get error messages set this to false
$config->use_own_error_handling = true;

//
// *** PATHS AND NAMES ***
//

// The path to the application folder (relative to web server's document root, with leading and trailing slash!)
$config->path = "/search/";

// The path to the autocomplete folder (relative to web server's document root, with leading and trailing slash!)
$config->autocomplete_path = "/autocomplete-php/autocomplete/";
// $config->autocomplete_path = "/autocomplete-php.UNTIL_18May09/autocomplete/";

// The Name of the CSS file
$config->css_file = "autocomplete.css";

// The name of this application (appears in log file, used for session name generation, etc.) 
$config->application_name = "dblpfacets";

// The title of the main page
$config->html_title = "CompleteSearch DBLP";

// encoding (iso-8859-1 or utf-8)
$config->encoding = "utf-8";
//$config->encoding = "iso-8859-1";

// NEW 24Aug07 (Holger): just trying
$config->index_url = "http://www.dblp.org/search/";
//$config->index_url = "http://geek2.ag1.mpi-sb.mpg.de/dblp-mirror/";
//$config->index_url = "http://search.mpi-sb.mpg.de/dblp-mirror/";

// base URL of the DBLP page
$config->dblp_url = "http://www.dblp.org/search/";

// Custom text (absolute path or relative to this directory)
$config->custom_text_file_name = "custom_text.php";

//
// *** COMPLETION SERVER ***
//

// host name of completion server
//$config->server_hostname = "localhost";
$config->server_hostname = "panarea";
//$config->server_hostname = "geek2.mpi-sb.mpg.de";

// url for alternative communication with completion server (experimental)
$config->server_url = "http://www..dblp.org/search/relay.pl?port=%s&input=%s";

// port of the completion server to which to talk to
//$config->server_port = rtrim(file_get_contents("/var/opt/completesearch/ports/dblp.port"));
$config->server_port = 8181;

// timeout for connectiong to the completion server via socket (in seconds)
$config->server_timeout = 15;

//
// *** GUI PARAMETERS (defaults that can be changed) ***
//

// default language of GUI
$config->language = "en";


//
// *** GUI PARAMETERS (cannot be changed interactively) ***
//

// don't start search before this many characters typed
$config->min_query_size = 1;

// no completions for words shorter than this
$config->min_prefix_size = 1;

// append star only if prefix longer than this
$config->min_prefix_size_to_append_star = 3;

// assume implicit * after every word if non-zero
$config->always_prefix = 1;

// delays after each keystroke before launching query (for prefixes of size 0, 1, 2, 3, ...)
$config->delays = array(1000, 1000, 1000, 500, 200); //,200, 200, 200, 200, 200, 200, 200, 200, 200); 
//$config->delays = array(1000, 1000, 1000, 500, 100); //,200, 200, 200, 200, 200, 200, 200, 200, 200); 

// show only selected facets (in the specified order), if array("") show all facets in alphabetical order
$config->facets_to_show = array("author", "venue", "year");

// show facets for empty query
$config->show_facets_for_empty_query = false;

// query types (A = normal, J = join, C = category, F = faceted)
$config->query_types = "F";

// synonym mode (0 = leave query as is, 1 = expand words with trailing ~, 2 = expand all words in query)
$config->synonym_mode = 0;

// debug mode (0 = normal operation, 1 = do not rewrite completions)
$config->debug_mode = 0;

// show this much more/less when clicking on [more]/[less]
$config->more_offset = 4;

// append hits (A) or show next page (M)
$config->box_navigation_mode = "M";

// What string to append when clicking a completion
$config->append_to_clicked_completion = ""; // "$"

// What string to append to ct/cn completions
$config->append_to_clicked_cat_completion = ""; //""; // ":"

// *full* path name of log files (replace by your own)
//$config->access_log = $_SERVER['DOCUMENT_ROOT'] . $config->path . "access.log";
$config->access_log = "/var/log/dblp/access.log";
//$config->error_log = $_SERVER['DOCUMENT_ROOT'] . $config->path . "error.log";
$config->error_log = "/var/log/dblp/error.log";


// Display mode (1 = title + excerpts + url, 2 = same, but one line per excerpt, 3 = same, but no url)
$config->display_mode = 3;

$config->top_hits = array(4, 50, 250);
// number of completions per box
$config->max_completions_show = 4;

// number of hits per page
$config->hits_per_page = 20;

// number of excerpts per hit
$config->excerpts_per_hit = 1;

// number of words displayed to the left and right of highlighted words in excerpts
$config->excerpt_radius = 9999;

// maximum length of a completion (rest is truncated and shown as ...)
$config->max_completion_length = 80;

// The log level default (1 = fatal, 2 = error, 3 = warning, 4 = info, 5 = debug)
$config->log_level = 2;

// Defaults: parameters which can be changed by user (by the options page)
//
// First we define all parameters which are accessible by user (user preferences)
// For each parameter exists several (dot separated) settings.
//      The first part is the name of the corresponding AC member,
//      the second one is the name of the corresponding URL parameter,
//      the third one determines whether the paramter should be configurable in the options page ("y" = yes, "n" = no)
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

// Must be false (which is default, so this is optional)
$config->javascript = false;

?>
