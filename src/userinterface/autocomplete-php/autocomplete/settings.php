<?php

class Settings
{
  // Set the name of the application which is using the autocomplete functionality
  // NEW 28.02.07 (Markus): no longer used
  //	var $application_name;

  // The Url to the folder of the application
  var $index_url;

  // Url to the folder of the autocompletion files
  var $autocomplete_url;

  // The path to the folder of the application files
  var $index_path;

  // The path to the folder of the autocompletion files
  var $autocomplete_path;

  // The Name of the index page of the application
  var $index_page;

  // Set the name of the autocomplete application
  var $session_name;

  // NEW 24Oct06 (Holger): the encoding
  var $encoding;

  // The file names of the access and the error log file
  var $access_log;
  var $error_log;

  // Name of CSS file of the application
  var $css_file;

  // Completion server
  var $server_hostname;
  //	var $server_url;
  var $server_port;
  var $server_timeout;
  var $service_name;

  // The language to use for the GUI
  var $language;

  // The level of logging information
  var $log_level;
  var $log_function_name;
  
  // An array containing all parameters which can be changed by user (by the options page)
  var $user_preferences;

  // For variables which can be set by URL: translations from the variable name to the name of the corresponding GET parameter
  var $varnames_abbreviations;

  // E-mail address to which log information is sent
  var $admin_email;

// The title of the main page (used for the HTML title-tag)
  var $html_title;

  // Name of file with custom texts (will be read just after text.php)
  var $custom_text_file_name;

  // Characters that separate words
  var $separators;
  var $block_separators;

  // Capital characters
  var $capitals;

  // No completions before this many characters typed
  var $min_query_size;

  // No completions for words shorter than this
  var $min_prefix_size;

  // Append star only if prefix longer than this
  var $min_prefix_size_to_append_star;

  // If not zero, assume implicit * after every word
  var $always_prefix;

  // What string to append when clicking a completion
  var $append_to_clicked_completion = ""; // "$"

  // What string to append a : to ct/cn completions
  var $append_to_clicked_cat_completion; // ":"

  // Delays after each keystroke before launching query (for prefixes of size 0, 1, 2, 3, ...)
  var $delays;

  // The expiration for cookies
  var $cookie_expiration;

  // Optional array declaring which facets (in the specified order) are shown
  var $facets_to_show;

  // Optional array declaring which facets (in the specified order) are shown for the empty query
  var $show_facets_for_empty_query;

  // Auto scroll for the hits: how pre-active the "more hits"-action is triggered is determined by the following value
  // The value indicates the part of the scrollable area the user has to be scrolled through before autoscrolling should be startet; 
  // the value ranges from 0 (means 0%) to 1 (means 100%) where the value "0" has a special meaning: the autoscroll functionality is disabled
  // A value of "1" means that system start autoscroll not until before the user reached the bottom
  var $hits_autoscroll_threshold;

  // Allowed values are "0" and "1" (default is "0").
  // If "1" hit requests are sent until the hit box is full of hits or all hits are shown and scrollbars appear
  var $hits_autofill;
  
  // How to rank documents: 0 = by score, 1 = by doc id, 2 = by word id, a = ascending, d = descending.
  var $how_to_rank_docs;
  
  // How to rank words (completions): 0 = by score, 1 = by doc count, 2 = by
  // occurrence count, 3 = by word id, 4 = by doc id, a = ascending, d = descending.
  var $how_to_rank_words;
  
  // NEW 11May07 (Holger): synonym mode (0 = leave query as is, 1 = expand words with trailing ~, 2 = expand all words in query)
  var $synonym_mode;

  // Parameter concerning user preferences about query settings

  // Compute at most this many completions
  var $max_completions_compute;

  // The Number of ompletion to show in a box
  var $max_completions_show;

  // The Number of ompletion to show in the right (detail) box
  var $max_completions_show_right;

  // The maximum lenght of completion strings in the completion box
  var $max_completion_length;

  // The index of the first hit displayed
  //	var $first_hit;

  // The maximum number of hits that are displayed per page while typing.
  var $hits_per_page_while_typing;
  
  // The maximum number of hits that are displayed per page on click.
  var $hits_per_page_on_click;

  // The maximum number of word matches per hit that are displayed
  var $excerpt_radius;

  // The maximum number of excerpts per hit that are displayed
  var $excerpts_per_hit;

  // Different kinds of displaying the hit list
  var $display_mode;

  // Mode to navigate through entries of boxes
  var $box_navigation_mode;

  // Debug mode (0 = rewrite completions, 1 = show raw completions)
  var $debug_mode;

  // String which encodes the kinds of queries should be processed (query types), for example "HWF"
  // (H = hit, W = word, J = join, C = category, F = faceted, Y = yago, R = yago (relation), P = *p*recomputed DBLP facets, X = alternative to Y )
  var $query_types;

  // String which indicates which boxes of a query type should be processed, for example "1" (=> the first box) or "" (=> all boxes of this type)
  var $query_index;

  // Show progress images?
  var $show_progress_images;

  // Time to start fade out the boxes when they are new computed (in milliseconds)
  var $boxes_fade_out;

  // The amount to increase/decrease number of shown completions when user click more/less
  var $more_offset;

  // An alternative kind of "more" link in the completions boxes: an array of "top X" values.
  //  For example, "array(5, 50, 250)" offers the top (first) 5, 50, and 250 completions
  var $top_hits;
  
  // NEW 11Sep07 (Holger): show scores
  var $show_scores;

  // If there is only a single completion in the W box, show it only if the following is true                                  
  var $show_single_word_completion;                    
  
  // If there is no completion in the W box, show it only if the following is true                                  
  var $show_empty_wbox;
  
  // *** This is for formatting the hits ***
  //
  // The hit template can be a string or an array of three strings.
  // If hit template is a string it contains simply the template.
  // If it is an array the template provides a grouping (according to year, for example).
  // In this case the first element is the template itself, the second one contains the entity to group ("YEAR", for example),
  // and the third one is the pattern to format the group title ("<bd>%YEAR%</bd>", for example)
  //
  // To apply additional transformations to the %ENTITY% constructs below add transformation functions in "transformations.php"
  var $hit_template;
  
  // This is to embed the hits in an "envelope", e.g. surround by the following string; "%s" is replaced by the hits
  var $hits_envelope;
  

  
  // Constructor
  //	function Settings()
  function initialize()
  {
    // encoding (iso-8859-1 or utf-8)
    $this->encoding = "utf-8";

    // timeout for connectiong to the completion server via socket (in secondes)
    $this->server_timeout = 3;

    $this->access_log = "access.log";
    $this->error_log = "error.log";

    // default language of GUI
    $this->language = "de";

    // Name of file with custom texts (will be read just after text.php)
    $this->custom_text_file_name = "";

    // Special handling for max_completions_compute
    // Althought the mechanism is still working, we prefer to computes all completions.
    // "0" means computes an infinite number of completions
    // If not set the completions server takes an own default, momentary 200
    $this->max_completions_compute = 0;

    // number of completions per box
    $this->max_completions_show = 4;

    // number of completions in the right box
    $this->max_completions_show_right = 40;

    // number of hits per page while typing
    $this->hits_per_page_while_typing = 20;

    // number of hits per page on click
    $this->hits_per_page_on_click = 1000;
    
    // number of excerpts per hit
    $this->excerpts_per_hit = 3;

    // number of words displayed to the left and right of highlighted words in excerpts
    $this->excerpt_radius = 5;

    // maximum length of a completion (rest ist truncated and shown as ...)
    $this->max_completion_length = 40;

    // The log level default (1 = fatal, 2 = error, 3 = warning, 4 = info, 5 = debug)
    $this->log_level = 4;
    $this->log_function_name = "";

    // Characters that separate words/blocks
    $this->separators = " .\|\-\[\]=,;";

    // NEW 03May06 (Holger): transferred this from AC_init.php (but forgot where it is/was needed)
    $this->block_separators = " .\|\-\[=,;";

    $this->capitals = "A-ZÄÖÜÑÅÉÚÓÁØÆÑÇ";

    // TODO Does this work?
    $this->cookie_expiration = 60*60*24*30;	// 30 days

    // Name of CSS file of the application
    $this->css_file = "autocomplete.css";

    // don't start search before this many characters typed
    $this->min_query_size = 2;

    // no completions for words shorter than this
    $this->min_prefix_size = 2;

    // append star only if prefix longer than this
    $this->min_prefix_size_to_append_star = 3;

    // assume implicit * after every word if non-zero
    $this->always_prefix = 1;

    // delays after each keystroke before launching query (for prefixes of size 0, 1, 2, 3, ...)
    $this->delays = array(1000, 400, 200, 200, 0, 0);

    // show only selected facets (in the specified order), if array("") show all facets in alphabetical order
    $this->facets_to_show = array("");

    // show facets for empty query
    $this->show_facets_for_empty_query = false;

    // how to rank documents: 0 = by score, 1 = by doc id, 2 = by word id, a = ascending, d = descending.
    $this->how_to_rank_docs = "0d";
    
    // how to rank words (completions): 0 = by score, 1 = by doc count, 2 = by occurrence count, 3 = by word id, 4 = by doc id, a = ascending, d = descending.
    $this->how_to_rank_words = "1d";
    
    // NEW 11May07 (Holger): synonym mode (0 = leave query as is, 1 = expand words with trailing ~, 2 = expand all words in query)
    $this->synonym_mode = 0;

    // query types (J = join, C = category, F = faceted)
    $this->query_types = "HWF";
    
    // query index ("" => computes all boxes)
    $this->query_index = "";

    // display mode (1 = title + excerpts + url, 2 = same, but one line per excerpt, 3 = same, but no url)
    $this->display_mode = 1;

    // Mode to navigate through entries of boxes (A = append to previous hits, R = replace previous hits)
    $this->box_navigation_mode = 'R';

    // debug mode (0 = rewrite completions, 1 = show raw completions)
    $this->debug_mode = 0;

    // What string to append when clicking a completion
    $this->append_to_clicked_completion = ""; // "$this->"

    // Whether to add a : to ct/cn completions or not
    $this->append_to_clicked_cat_completion = ""; // ":"

    // Show progress images?
    $this->show_progress_images = true;

    // time delay before fading boxes out
    $this->boxes_fade_out = 1000;	// in milliseconds

    // The amount to increase/decrease number of shown completions when user click more/less
    // NEW 12Sep07 (Holger): 20 -> 100
    $this->more_offset = 100; // = 20;

    // Auto scroll for the hits: how pre-active the "more hits"-action is triggered is determined by the following value
    // The value indicates the part of the scrollable area the user has to be scrolled through before autoscrolling should be startet; 
    // the value ranges from 0 (means 0%) to 1 (means 100%) where the value "0" has a special meaning: the autoscroll functionality is disabled
    // A value of "1" means that system start autoscroll not until before the user reached the bottom
    $this->hits_autoscroll_threshold = 0.8;
  
    // Allowed values are "0" and "1" (default is "0").
    // If "1" hit requests are sent until the hit box is full of hits or all hits are shown and scrollbars appear
    $this->hits_autofill = 0;
    
    // An alternative kind of "more" link in the completions boxes: an array of "top X" values.
    //  For example, "array(5, 50, 250)" offers the top (first) 5, 50, and 250 completions
    $this->top_hits = array("4", "50", "250");
  
    // NEW 11Sep07 (Holger): show scores
    $this->show_scores = false;

    // NEW 06Mar08: default for admin_email
    $this->admin_email = "bast@mpi-inf.mpg.de";

    // The following declares those members of the settings class which can be modified by user (for example, they appear on the options page
    // ATTENTION: members declared here have to be mapped to an abbreviation in the varnames_abbreviations array below
    $this->user_preferences = array("max_completions_show", "hits_per_page_while_typing", "hits_per_page_on_click", "max_completion_length",	"excerpts_per_hit",	"excerpt_radius", "display_mode", "box_navigation_mode", "log_level", "query_types");

    $this->varnames_abbreviations = array(
    "language" => "language",
    "max_completions_show" => "mcs",
    "max_completions_show_right" => "mcsr",
    "max_completions_compute" => "mcc",
    "max_completion_length" => "mcl",
    "hits_per_page_while_typing" => "hppwt",
    "hits_per_page_on_click" => "hppoc",
    "excerpts_per_hit" => "eph",
    "excerpt_radius" => "er",
    "display_mode" => "dm",
    "box_navigation_mode" => "bnm",
    "log_level" => "ll",
    "more_offset" => "mo",
    "query_types" => "qt",
    "query_index" => "qi",
    "first_hit" => "fh",
    "append_to_clicked_completion" => "acc",
    "append_to_clicked_cat_completion" => "accc",
    "synonym_mode" => "syn",
    "debug_mode" => "deb",
    "show_scores" => "sco",
    "how_to_rank_docs" => "hrd",
    "how_to_rank_words" => "hrw",
    "first_hit_shown" => "fhs"
    );
    
    // If there is only a single completion in the W box, show it only if the following is true                                  
    $this->show_single_word_completion = false;                    
    
    // If there is no completion in the W box, show it only if the following is true                                  
    $this->show_empty_wbox = false;

    // *** This is for formatting the hits ***
    //
    // The hit template can be a string or an array of three strings.
    // If hit template is a string it contains simply the template.
    // If it is an array the template provides a grouping (according to year, for example).
    // In this case the first element is the template itself, the second one contains the entity to group ("YEAR", for example),
    // and the third one is the pattern to format the group title ("<bd>%YEAR%</bd>", for example)
    //
    // To apply additional transformations to the %ENTITY% constructs below add transformation functions in "transformations.php"
    $this->hit_template = "";

    // This is to embed the hits in an "envelope", e.g. surround by the following string; "%s" is replaced by the hits
    $this->hits_envelope = "";
  }
}
?>
