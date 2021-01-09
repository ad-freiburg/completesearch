<?php

class Query
{
  /*
  Request parameters: are passed to the autocomplete server with the request.
  They are already set here as a kind as default, but they can be modified by user at every time
  */
  var $id;    // Identifier for requests during a session
  var $request_id;    // Identifier for requests during a session
  var $query_string;    // The query string
  var $query_types; // String which encodes the kinds of queries should be processed, for example "ACFJ"

  // Index i (integer 0..k) which means the i-th subtype of the query types above should be processed, for example "ACFJ"
  // Corresponds to the i-th box. Empty string means compute all
  var $query_index;
  var $max_completions_compute; // Compute at most this many completions
  var $max_completions_show;  // The Number of words in excerpt to the left and right of each word match
  var $max_completion_length; // The maximum lenght of completion strings in the completion box
  var $first_hit; // The index of the first hit displayed
  var $first_hit_shown;
  var $hits_per_page_while_typing; // Maximal number of hits that are displayed per page while typing.
  var $hits_per_page_on_click; // Maximal number of hits that are displayed per page on click.
  var $excerpt_radius;  // Maximal number of word matches per hit that are displayed
  var $excerpts_per_hit;  // Maximal number of excerpts per hit that are displayed
  var $how_to_rank_docs;      // How to rank documents, the digit stands for the kind of sorting, the letter for a / d for ascending / descending
  var $how_to_rank_words;     // How to rank words (completions), the digit stands for the kind of sorting, the letter for a / d for ascending / descending
  var $display_mode;  // Different kinds of displaying the hit list
  var $debug_mode;    // Debug mode (0 = rewrite completions, 1 = show raw completions)
  var $box_navigation_mode; // Mode to navigate through entries of boxes
  var $append_to_clicked_completion; // "$" // What string to append when clicking a completion
  var $append_to_clicked_cat_completion; // ":"   // What string to append a : to ct/cn completions
  var $port;        
  var $last_word_of_query;    // The last word of the query
  var $last_separator_of_query;       // The separator before that (empty word for 1-word queries)
  var $first_part_of_query;   // The part of the query before that separator (empty word for 1-word queries)

  

  /**
   * Constructor
   */
  function Query()
  {
    $this->id = 1;
    $this->query_string = "";
    $this->query_index = "";
  	$this->first_hit = 1;
    $this->first_hit_shown = 1;
  }



  /**
	 * Initialize the query object from settings of the $ac instance
	 *
	 * @param AC $ac
	 */
	function initialize(& $ac)
	{
	  foreach (get_object_vars($this) as $name => $value)
          {
	    // Set only those members which have an corresponding member in $AC->settings
	    if (isset($ac->settings->$name))
	    {
	      $this->$name = $ac->settings->$name;
            }
	  }
	}
  /**
   * BUILD QUERY TO BE SENT TO COMPLETION SERVER (append *, rewrite join queries, append parameters, etc.)
   *
   * @return string the constructed query string whcih is sent to the completion server
   *
   * NEW 21Oct11 (Hannah + Ina): now returns only the query + parameters, since 
   * we need exactly that when generating the links for the "these search 
   * results as XML, JSON, etc." feature in AC.php, line 2835 (version 21Oct11).
   * Before we also used to add GET /?q= before and the HTTP/1.0 after; this is 
   * now done after the call of query_for_completion_server in AC.php, line 1321 
   * (version 21Oct11).
   * NEW 25Oct11 (Ina): now returns just rewritten query, since we don't need 
   * all parameters in "these search results as XML, JSON, etc.". The parameters 
   * can still be get by get_query_parameters (see line 1321).
   */
  function rewrite_query()
  {
    global $AC;
    $SEP = $AC->settings->separators;
    $MIN = $AC->settings->min_prefix_size_to_append_star;

    $query = $this->query_string;
    // NEW 03AUg06 (Holger): replace all _ by .
    // NEW 24Jan07 (Holger): temporarily disabled
    //$query = preg_replace("|_|", ".", $query);

    // NEW 02Aug06 (Holger): put * after each word that is at least 3 characters long
    // NEW 06Aug06 (Holger): prepend ct: to all words containing a colon
    // NEW 07Sep11 (Hannah): change the ct: to ce:
    if (preg_match("|::|", $query) == 0)
    $query = preg_replace("/([^$SEP]+:[^$SEP]*)/", "ce:$1", $query);
    // NEW 12Mar07 (Holger): but "ct:entity:..." -> "ce:entity:..."
    $query = preg_replace("/\b(ct|ce):(ce|ch|cr|s|ct):/", "$2:", $query);
    $query = preg_replace("/\bct:(entity|persontheorganism):/", "ce:$1:", $query);
    // NEW 2Aug06 (Holger): and prepend cn: to all words in ALL CAPITALS
    $query = preg_replace("/(^|[$SEP])(ct:)(tg:|c[tns]:)/", "$1$3", $query);
    // NEW 01Jul08 (Holger): replace - by . unless it comes after a space or unless --- (new range operator)
    $query = preg_replace("|([^- ])-([^-])|", "$1.$2", $query);
    // NEW 03Jul08 (Holger): replace ". " by single space (because many people type sth like "k. mehlhorn" in DBLP)
    $query = preg_replace("|\. |", " ", $query);
    // NEW 11Jul08 (Holger): replace ' by . because people type something like O'Hare
    $query = preg_replace("|\\\'|", ".", $query);
    //$query = preg_replace("/([A-ZÄÖÜ]{2,})/", "cn:$1", $query); // 29.Sep06 Ingmar: Turn this of (so that IMAP->imap)
    // NEW 23Jul07 (Holger): treat one space as proximity, two spaces as normal (convenient for demos)
    //$query = preg_replace("| |", "..", $query);
    //$query = preg_replace("|\.{3,}|", " ", $query);
    // NEW 27.07.07 (Markus): mb_strtolower only for utf-8 encoding (where it is necessary), because not all PHP installation support mbsting extension
    $query = preg_replace("/([^$SEP*]{".$MIN.",})($|[$SEP])/", "$1*$2", my_strtolower($query));
    
    $query = preg_replace("|(.*?)\\*\\$|", "\"$1\"", $query);
    $query = preg_replace("|(.*?)\\$\\*|", "\"$1\"", $query);
    $query = preg_replace("|\\)\\*|", "*)", $query);
    // NEW 25AUg10 (Hannah): deal with fuzzy search and synonym search in a nice 
    // way. Automatically do fuzzy search.
    if ($AC->settings->session_name == "dblp-fuzzy")
      $query = preg_replace("|\\*|", "*~", $query);
    if ($AC->settings->session_name == "telefondaten")
      $query = preg_replace("|\\*|", "*~", $query);
    $query = preg_replace("|ct:c:|", "C:", $query);
    $query = preg_replace("|\\~\\*|", "*~", $query);
    $query = preg_replace("|\\^\\*\\~|", "^", $query);
    $query = preg_replace("|\\^\\*|", "*^", $query);
    // NEW 21Sep10 (Hannah): for new semantic search.
    $query = preg_replace("|ct::e:|", ":e:", $query);
    $query = preg_replace("|ct::t:|", ":t:", $query);
    // $query = preg_replace("|ce:entity:|", "E:entity:", $query);
    // NEW 30Aug11 (Hannah): disable *~ because it takes too long, just do ~.
    // $query = preg_replace("|\\*\\~|", "~", $query);


    // REWRITE JOIN PARTS OF QUERY, e.g., author[sigmod sigir] -> [author#sigmod author#sigir author]
    $pos_beg = strrpos($query, "[");
    $pos_end = strrpos($query, "]");
    if ($pos_beg !== false && ($pos_end === false || $pos_end < $pos_beg))
    {
      // NEW 3Aug06 (Holger): have to use join-block separators here (TODO: use $AC->configuration->...)
      $BLOCK_SEP = "\[ ";
      $query = preg_replace("/([^$BLOCK_SEP]+)$/", "] $1", $query); // was: $query .= "]";
      //$query = preg_replace("/([^$SEP]+)$/", "] $1", $query); // was: $query .= "]";
      $query = preg_replace("/ \]/", "]", $query);
      $query = preg_replace("/\[$/", "[]", $query);
    }
    $query = preg_replace_callback("|([^$SEP]+\\[[^]]+\\])|", array($this, "rewrite_joins"), $query);

    /*
    // APPEND PARAMETER STRING (unless it is already there)
    $query .= " " //"%20"
    . "M" . $this->max_completions_compute
    . "C" . $this->max_completions_show
    . "H" . $this->hits_per_page_while_typing
    . "F" . ($this->first_hit - 1)
    . "E" . $this->excerpts_per_hit
    . "R" . $this->excerpt_radius
    . "D" . $this->display_mode;
    */
    /*
    $query = $query . "&h="  . $this->hits_per_page_while_typing
                    . "&c="  . $this->max_completions_show
                    . "&f="  . ($this->first_hit - 1)
                    . "&en=" . $this->excerpts_per_hit
                    . "&er=" . $this->excerpt_radius
                    . "&rd=" . $this->how_to_rank_docs
                    . "&rw=" . $this->how_to_rank_words;
    */
    return $query; // urlencode($result);
  }

  /* NEW 25Oct11 (Ina): Rewriting query and adding parameters are now splitted 
   * into two functions: rewrite_query & get_query_parameters.
   * Returns string with default query parameters: &h=20&c=100&...
   */
  function get_query_parameters($javascript_on)
  {
    $query_parameters =   "&h="  . ($javascript_on ? $this->hits_per_page_while_typing : $this->hits_per_page_on_click)
                        . "&c="  . $this->max_completions_show
                        . "&f="  . ($this->first_hit - 1)
                        . "&en=" . $this->excerpts_per_hit
                        . "&er=" . $this->excerpt_radius
                        . "&rd=" . $this->how_to_rank_docs
                        . "&rw=" . $this->how_to_rank_words;

    return $query_parameters; 
  }



  /**
   * Rewrite query block of type "auth[sigmod sigir]"
   *
   * @param unknown_type $regs
   * @return unknown
   */
  function rewrite_joins($regs)
  {
    $block = $regs[0];
    //$this->log("in rewrite_joins with block \"" . $block . "\"");
    $pos_beg = strrpos($block, "[");
    $pos_end = strrpos($block, "]");
    $join_word = substr($block, 0, $pos_beg);
    $attr_words = explode(" ", substr($block, $pos_beg+1, $pos_end-$pos_beg-1));
    //$this->log($join_word . " *** " . implode("|", $attr_words));
    foreach ($attr_words as $p => $v) $attr_words[$p] .= " " . $join_word;
    return "[" . $join_word . "#" . implode("#", $attr_words) . "]"; // was "]$"
  }


  function to_string()
  {
    return "$this->query_string (#$this->id): " . "types: $this->query_types";
  }
}
?>
