<?php

// The name of this script (used to identify log message)
$path_info = pathinfo(__FILE__);
define("SCRIPT_RESULT", $path_info["basename"]);

/**
 * Class for representing the result of completion computation
 *
 */
class Result
{
  var $subtitle;					         // The sub title of the current result
  var $completions;                // The completions list as an array of objects of class Completion
  var $completions_total;          // The overll number of completions for the current query
  var $completions_computed;       // The count of completions completion server has computed
  var $completions_sent;           // The count of completions we get from completion server
  var $hits_count_per_completion;	 // Array with the counts of hits per every completion
  var $completion_index;
  var $completion_start_index;

  var $hits;						           // The hits we get from completion server, an array of objects of class Hit
  var $hits_total;				         // The overall number of hits
  var $hits_computed;              // The count of hits the completion server has computed
  var $hits_sent;                  // The count of hits we get from completion server
  var $first_hit;
  var $first_hit_shown;
  var $last_hit;

  //	var $hit_box;					 // The hits of the current query as an array array('title' => ..., 'body' => ...)
  var $H_boxes;					 // The hits of the current query as an array array('title' => ..., 'body' => ...)
  var $W_boxes;          // Words
  var $C_boxes;
  var $F_boxes;
  var $J_boxes;
  var $R_boxes;
  var $Y_boxes;
  var $T_boxes;

  var $options_title;				// The title of the options link

  var $response_size;				// Size of the server response in characters
  var $time_A;					// Time (duration) of the last request processing (without communication completion server <-> web server, value is provided by the completion server)
  var $time_B;					// Time (duration) of the last request processing (time for transfer included)
  var $time_C;					// Time (duration) of ...
  var $error_message_title;		// If errors occurres while processing they are containted here
  var $error_message;				// If errors occurres while processing they are containted here
  var $do_continue;				// If true coninue in case of error, if not stop the current search processing
  var $status;
  var $collection;    // If the complete search server uses a xml namespace in its response set this namespace as collection




  /**
      * Constructor
      * 
      * Create a result object and set its members accordingly to the XML response from complete-search server (cs server).
      *
      * @param string $xml the answer of cs server in XML format
      * @return Result an instance of class Result which contains the answer of CS server
      */
  function Result($xml = "")
  {
    global $log;
    if ($xml == "") return;
    // This is to test the error handling of the parser
//    $xml .= "</result>";

    // We use the class xml from xml_parser.php
    $xml_dom = new xml($xml);

    // Markus / 01-07-09: now we get an result code and the array if call was successful
    list($rc, $doc) = $xml_dom->get_xml_array();
    if ($rc > 0) {
      // An error occurred
      $log->write($doc, $log->levels['ERROR'], SCRIPT_RESULT, $AC->query->request_id, __FUNCTION__, __LINE__, $log->targets["FILE_SESSION"], "XML parse error");
      $log->write("The following listing of the XML don't show the original line breaks, so the line number above don't match this listing!", $log->levels['ERROR'], SCRIPT_RESULT, $AC->query->request_id, __FUNCTION__, __LINE__, $log->targets["FILE_SESSION"], "NOTE");
      $log->write($xml, $log->levels['ERROR'], SCRIPT_RESULT, $AC->query->request_id, __FUNCTION__, __LINE__, $log->targets["FILE_SESSION"], "\$xml");
      return;
    }
//    $doc = $xml_dom->get_xml_array();
    $doc_root = $doc[0];

    foreach ($doc_root['children'] as $child1)
    {
      switch ($child1['name'])
      {
        case "STATUS": $this->status = $child1['tagData'];
        break;

        case "COMPLETIONS":
          $this->completions_total = $child1['attrs']['TOTAL'];
          $this->completions_computed = $child1['attrs']['COMPUTED'];
          $this->completions_sent = $child1['attrs']['SENT'];

          if (!array_key_exists('children', $child1)) break; 
          foreach ($child1['children'] as $child2)
          {
            $attributes = $child2['attrs'];
            $completion = new Completion();
            $completion->id = $attributes['ID'];
            $completion->score = $attributes['SC'];
            $completion->document_count = $attributes['DC'];
            $completion->occurrence_count = $attributes['OC'];
            $completion->string = $child2['tagData'];
            $this->completions[] = $completion;
          }
          break;

        case "HITS":
          $this->hits_total = $child1['attrs']['TOTAL'];
          $this->hits_computed = $child1['attrs']['COMPUTED'];
          $this->hits_sent = $child1['attrs']['SENT'];
          $this->first_hit = $child1['attrs']['FIRST'];
          if (!array_key_exists('children', $child1)) break; 
          foreach ($child1['children'] as $child2)
          {
            $attributes = $child2['attrs'];
            $hit = new Hit();
            $hit->id = $attributes['ID'];
            $hit->score = $attributes['SCORE'];

            foreach ($child2['children'] as $child3)
            {
              switch ($child3['name'])
              {
                case "TITLE":
                  // NEW 26Sep12 (baumgari):
                  // Title is deprecated and was changed to "info", therefore just 
                  // don't break the case and change the name to info.
                  $child3['name'] = "INFO";
                case "INFO":
                  // NEW Markus / 02-02-09
                  if (array_key_exists('children', $child3)) {
                    $children = & $child3['children'];
                  }
                  else {
                    // If the <title> tag is not interpreted as XML (potentially because of CDATA section)
                    // we try to interpret the inner part as XML (DBLP contains further XML coded information, for example)
                    // First we have to replace some non XML conform characters ("&", e.g)
                    // Additionally we need a root element, <title> for example
                    $xml_dom = new xml("<info>" . str_replace("&", "&amp;", $child3['tagData']) . "</info>");
                    // Markus / 01-07-09: now we get an result code and the array if call was successful
                    list($rc, $doc) = $xml_dom->get_xml_array();
                    if ($rc > 0) {
                      // An error occurred
                      $log->write($doc, $log->levels['ERROR'], SCRIPT_RESULT, $AC->query->request_id, __FUNCTION__, __LINE__);
                      $log->write($xml, $log->levels['ERROR'], SCRIPT_RESULT, $AC->query->request_id, __FUNCTION__, __LINE__, $log->targets["SESSION"], "\$xml");
                      return;
                    }
//                    $doc = $xml_dom->get_xml_array();
                    // $AC->log->write("*****RESULT aus tagData*****" . print_r($doc, true));
                    if (! array_key_exists("children", $doc[0])) {
                      $hit->title['title'] = $this->unescapeXml($child3['tagData']);
                      break;
                    }
                    $children = & $doc[0]['children'];
                  }

                  foreach ($children as $child4)
                  {
                    // Split string of kind dblp:author.
                    $r = explode(":", $child4['name']);
                    if (count($r) == 2)
                    {
                      $this->collection = $r[0];
                      $field = $r[1];
                    }
                    else $field = $r[0];

                    if (array_key_exists('attrs', $child4)) {
                      foreach ($child4['attrs'] as $attribute => $value) {
                        $hit->title[strtolower($field) . "_" . strtolower($attribute)] = $value;
                      }
                    }
                    switch ($field)
                    {
                      case "AUTHORS":
                        // If there are no authors the "children" value is not given; we have to set $hit->title["authors"] to the empty string
                        if (! array_key_exists('children', $child4)) {
                          $hit->title["authors"] = "";
                        }
                        else {
                          foreach ($child4['children'] as $child5) {
                            $hit->title["authors"][] = $this->unescapeXml($child5['tagData']);
                          }
                        }
                        break;
                      case "TITLE":
                        $tmp = $child4['tagData'];
                        $tmp = $this->unescapeXml($tmp);
                        // NEW 21Jun12 (baumgari):
                        // For some reason <i>...</i> doesn't work and leads to an error, such that no hits are printed.
                        // <em>...</em>, which actually does the same thing, does work. Therefore I just replace it.
                        $tmp = preg_replace("/<i>/", "<em>", $tmp);
                        $tmp = preg_replace("/<\/i>/", "</em>", $tmp);
                        $tmp = preg_replace("/\+/", "&#43;", $tmp);
                        $hit->title[strtolower($field)] = $tmp;
                        break;
                      // NEW 17May12 (Ina): Add support for <show field ... >.
                      case "SHOW":
                        if (array_key_exists('attrs', $child4)) {
                          switch ($child4['attrs']['FIELD'])
                          {
                            // Usually the generic / default case, is what we 
                            // want to use. But there are some exceptions for 
                            // which $hit->title[<field>] are incorrect, since 
                            // they are handled differently: $hit->url and 
                            // $hit->excerpts. 
                            case "URL":
                              $hit->url = $this->unescapeXml($child4['tagData']);
                              break;
                            case "EXCERPTS":
                              $hit->excerpts[] = $this->unescapeXml($child4['tagData']);
                              break;
                            default:
                              $tmp = $child4['tagData'];
                              $tmp = $this->unescapeXml($tmp);
                              $tmp = preg_replace("/<i>/", "<em>", $tmp);
                              $tmp = preg_replace("/<\/i>/", "</em>", $tmp);
                              $tmp = preg_replace("/\+/", "&#43;", $tmp);
                              $hit->title[strtolower($child4['attrs']['FIELD'])] = $tmp;
                              break;
                          }
                        }
                        break;

                      default:
                        $hit->title[strtolower($field)] = $this->unescapeXml($child4['tagData']);
                        break;
                    }
                  }
                  break;

                case "URL":
	          if (empty($hit->url)) $hit->url = $this->unescapeXml($child3['tagData']);
                  break;

                case "EXCERPT":
                  $tmp = $child3['tagData'];
                  // NEW Markus / 17-11-08: Support for highlighting of search words in the hit excerpts
                  //  Replace '<hl idx="k">...</hl>' by '<font class="hl-k">...</font>'. The corresponding CSS classes
                  //  are defined in autocomplete.css
                  // "U" is for un-greedy matching
                  $tmp = preg_replace("|\<hl idx=\"(.*)\"\>(.*)\</hl\>|U", "<font class=\"hl-$1\">$2</font>", $tmp);
                  $hit->excerpts[] = $tmp;
                  //	                $hit->excerpts[] = $child3['tagData'];
                  break;

                default:
                  break;
              }
            }
            $this->hits[] = $hit;
          }
          break;

        case "TIME": $this->time_A = $child1['tagData'];
        break;
      }
    }

    // Adaption to internal specifications: first_hit is "1" for the first hit, not "0" as the cs server delievers
    $this->last_hit = $this->first_hit + $this->hits_sent;
    $this->first_hit++;
  }
  
  function reset ()
  {
    $this->subtitle = "";
    $this->hit_title_box = "";
    $this->W_boxes = array();
    $this->C_boxes = array();
    $this->F_boxes = array();
    $this->J_boxes = array();
    $this->R_boxes = array();
    $this->Y_boxes = array();
    $this->T_boxes = array();
    $this->hits_total = 0;
    $this->hits_computed = 0;
    $this->hits_sent = 0;
    $this->completions = array();
    $this->completions_sent = 0;
    $this->hits_count_per_completion = 0;
    $this->completion_index = -1;
    $this->completion_start_index = -1;
    $this->response_size = 0;
    $this->time_A = 0;
    $this->time_B = 0;
    $this->options_title = "";
    $this->error_message_title = "";
    $this->error_message = "";
    $this->do_continue = false;
  }


  function set_error_message ($message, $title = "", $do_continue = false)
  {
    // Do not reset options_title
    $tmp = $this->options_title;

    $this->reset();

    // Restore options_title
    $this->options_title = $tmp;

    $this->error_message = $message;
    $this->error_message_title = $title;
    $this->do_continue = $do_continue;
  }

  // NEW 01Nov12 (baumgari):
  // Added generic function for unescaping everything from the xml, since it's 
  // proper xml now and not anymore encapsulated within cdata-tags.
  function unescapeXml ($xml)
  {
    $xml = preg_replace("/&apos;/", "\'", $xml);
    $xml = preg_replace("/&lt;/", "<", $xml);
    $xml = preg_replace("/&gt;/", ">", $xml);
    $xml = preg_replace("/&quot;/", "\"", $xml);
    $xml = preg_replace("/&amp;/", "&", $xml);
    return $xml;
  }

}

?>
