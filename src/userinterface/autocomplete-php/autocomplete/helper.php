<?php


  //
  // Helper functions
  //


	/**
	 * Construct the full path for the relative path given by $relative_path
	 *
	 * @param string $relative_path the relative path
	 * @return string the full path
	 */
	function get_full_path ($relative_path)
	{
		return $_SERVER['DOCUMENT_ROOT'] . $relative_path;
	}

	

	/**
	 * Construct the full URL for the relative path given by $relative_path
	 *
	 * @param string $relative_path the relative path
	 * @return string the full URL
	 */
	function get_full_url ($relative_path)
	{
		return $AC->settings->http_host . $relative_path;
		// return "http://ad-research.cs.uni-freiburg.de:17385" . $relative_path;
		// return "http://" . $_SERVER['SERVER_NAME'] . ":" . $_SERVER['SERVER_PORT'] . $relative_path;
		// return "http://" . $_SERVER['HTTP_HOST'] . $relative_path;
	}

	

	/**
	 * Append $parameters as query string to a given URL $url and return the resulting URL
	 * If $parameters is given as string this string is appended to the url.
	 * If $parameters is given as an array of paramters and their corresponding values
	 * the url is composed using these pairs.
	 *
	 * @param string $url the url of the script
	 * @param mixed $parameters the parameters fromwhich the query string should be constructed
	 * @return string the given URL $url with the constructed query string
	 */
	function url_append ($url, $parameters)
	{
		$tmp = '';
		if (isset($parameters))
		{
			if (is_array($parameters))
			{
				if ((float)phpversion() >= 5)
				{
					// PHP >= 5
					$tmp = http_build_query($parameters);
				}
				else
				{
					$start = true;

					foreach ($parameters as $parameter => $value)
					{
						if ($start)
						{
							$tmp .= "$parameter=$value";
							$start = false;
						}
						else $tmp .= "&$parameter=$value";
					}
				}
			}
			else // should be a string like "parameter=value"
			{
				$tmp = $parameters;
			}
			if ($tmp != "") {
				if (strpos($url, "?")) 
				{
					$url .= "&$tmp";
				}
				else $url .= "?$tmp";
			}
		}
		return $url;
	}



	/**
	 * Function to get post and get parameters
	 *
	 * @param string $name
	 * @return string the value of the url parameter with name $name
	 */
	function variable ($name)
	{
		if (array_key_exists($name, $_POST))
//		if (isset($_POST["$name"]))
		{
			return $_POST[$name];
		}
		else
		{
			if (array_key_exists($name, $_GET))
//			if (isset($_GET["$name"]))
			{
				return $_GET[$name];
			}
			else return null;
		}
	}

	

	/**
   * Parse the XML response from complete-search server (cs server) and deliver the gotten information in a Result object
   *
   * @param string $response the answer of cs server in XML format
   * @return Result $result an instance of class Result which contains the answer of CS server
   */
	/*
	function xml_parse_response($response)
	{
	  // We use the class xml from xml_parser.php
	  $xml = new xml($response);

	  $doc = $xml->get_xml_array();
	  $doc_root = $doc[0];

	  // The Result object which is returned
 		$result = new Result();

	  foreach ($doc_root['children'] as $child1)
	  {
	    switch ($child1['name'])
	    {
	      //      case "QUERY": $result->query_string = $child1['tagData'];
	      //      break;

	      case "STATUS": $result->status = $child1['tagData'];
	      break;

	      case "COMPLETIONS":
	        $result->completions_total = $child1['attrs']['TOTAL'];
	        $result->completions_computed = $child1['attrs']['COMPUTED'];
	        $result->completions_sent = $child1['attrs']['SENT'];

	        foreach ($child1['children'] as $child2)
	        {
	          $attributes = $child2['attrs'];
	          $completion = new Completion();
	          $completion->id = $attributes['ID'];
	          $completion->score = $attributes['SC'];
	          $completion->document_count = $attributes['DC'];
	          $completion->occurrence_count = $attributes['OC'];
	          $completion->string = $child2['tagData'];
	          $result->completions[] = $completion;
	        }
	        break;

	      case "HITS":
	        $result->hits_total = $child1['attrs']['TOTAL'];
	        $result->hits_computed = $child1['attrs']['COMPUTED'];
	        $result->hits_sent = $child1['attrs']['SENT'];
	        $result->first_hit = $child1['attrs']['FIRST'];

	        foreach ($child1['children'] as $child2)
	        {
	          $attributes = $child2['attrs'];
	          $hit = new Hit();
	          $hit->id = $attributes['ID'];
	          $hit->score = $attributes['SCORE'];

	          foreach ($child2['children'] as $child3) {
	            switch ($child3['name'])
	            {
	              case "TITLE":
               	  if (array_key_exists('children', $child3)) 
               	  {
                     foreach ($child3['children'] as $child4) 
                     {
                       $r = split(":", $child4['name']);
                       $result->collection = $r[0];
    
                       if (array_key_exists('attrs', $child4)) {
                        foreach ($child4['attrs'] as $attribute => $value) {
                        $hit->title[strtolower($r[1]) . "_" . strtolower($attribute)] = $value;                       	
                        }
                       }
                       switch ($r[1])
                       {
                       	case "AUTHORS":
                       	  foreach ($child4['children'] as $child5) {
                            $hit->title["authors"][] = $child5['tagData'];
                       	  }
                          break;
                        default:
                          $hit->title[strtolower($r[1])] = $child4['tagData'];
                          break;
                       }
                     }
               	  }
               	  else {
               	    $hit->title = $child3['tagData'];
               	  }
               		break;

	              case "URL":
	                $hit->url = $child3['tagData'];
	                break;

	              case "EXCERPT":
	                $hit->excerpts[] = $child3['tagData'];
	                break;

	              default:
	                break;
	            }
	          }
	          $result->hits[] = $hit;
	        }
	        break;

	      case "TIME": $result->time_A = $child1['tagData'];
	      break;
	    }
	  }

	  // Adaption to internal specifications: first_hit is "1" for the first hit, not "0" as the cs server delievers
	  $result->last_hit = $result->first_hit + $result->hits_sent;
	  $result->first_hit++;

	  return $result;
	}
	*/
	

  /**
   * Convert $string to lower cases.
   * Encapsulate the usage of strtolower and mb_strtolower.
   *
   * @param string $string to convert to lower cases
   * @return string The converted string
   */
  function my_strtolower($string)
  {
  	global $AC;
    // Use mb_strtolower only for utf-8 encoding (where it is necessary),
    // because not all PHP installation support mbsting extension
    if (strtolower($AC->settings->encoding) == "utf-8") {
      if (function_exists("mb_strtolower")) {
        return mb_strtolower($string, $AC->settings->encoding);
      }
      else {
        trigger_error("For use of multibyte string encodings the mbstring extension is needed which is probably not installed. Please change the encoding in the autocomplete_config.php or install the mbstring extension.", E_USER_ERROR);
        return "";
      }
    }
    else return strtolower($string);
  }
  
  
  
  /**
   * Generate Javascript code to preload all images of the folder specified by $image_dir.
   *
   * @param string $image_dir Preload all images contained in this folder
   * @param string $image_url URL prefix for the SRC attribute of the images
   */
  function preloadImages($image_dir, $image_url)
  {
    echo "\n\t\t\tif (document.images){";
    $dh = opendir($image_dir);
    if ($dh !== false)
    {
      $i = 1;
      while (false != ($file = readdir($dh)))
      {
        if (is_file("$image_dir/$file") == 1)
        {
          if (strpos ($file, ".jpg") > 0 || strpos ($file, ".gif") > 0)
          {
            // Größe des Bildes bestimmen
            list($width, $height, $typ, $att) = GetImageSize ("$image_dir/$file");
            echo "\n\t\t\t\tpicture$i = new Image($width,$height);";
            echo "picture$i.src = '$image_url/$file';";
            $i++;
          }
        }
      }
    }
  	echo "\n\t\t\t}";
  }
  
  
  
  /**
   * Check whether the keys of array $A are an integer sequence starting from $start.
   * Is used to destinguish between a "standard" array with numeric keys and an associative array with arbitrary keys (hash).
   *
   * @param array $A
   * @param integer $start
   * @return boolean
   */
  function is_sequence (& $A, $start = 0)
  {
    $AK = array_keys($A);
    for ($i = 0; $i < sizeof($AK); $i++) {
      if (! is_integer($AK[$i]) || $i + $start != $AK[$i]) return false;
    }
    return true;
  }
  
  	
?>
