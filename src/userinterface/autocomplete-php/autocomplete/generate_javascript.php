<?php

// The name of this script (used to identify log message)
define("SCRIPT_GENERATE_JAVASCRIPT", "generate_javascript");


/**
 * Make javascript code from a php variable
 *
 * Will be called, for example, as variable_in_javascript($AC->configuration, "AC")
 * creating code which, when eval'd, creates AC.configuration in JavaScript as
 * copy of $AC->configuration on the PHP side
 */
function variable_in_javascript(&$object, $members, $js_prefix, $urlencode = false)
{
//	global $AC;

	$javascript_code = "";
	foreach ($members as $member)
	{
	  $javascript_code .= variable_in_javascript3($object, $member, $js_prefix, $urlencode);
	  continue;
		$value = $object->$member;
		// CASE 1: array
		if (is_array($object->$member))
		{
		  $javascript_code .= "$js_prefix$member=" . array_in_javascript($object->$member, $urlencode) . ";\n";
      /*
		  if (is_int(key($object->$member)))
			{
				// The array is a list, not an associative array (the first key is an integer)
				// Therefore create a javascript array
				$javascript_code .= "$js_prefix$member=new Array(";
				$n = count($object->$member);
				// NEW 23.11.06 (Markus): old code didn't worked when keys of the array ($p) are not a integer index (as in $AC->settings->varnames_abbreviations)
				$i = 1;
				foreach ($object->$member as $p => $v)
				{
					$javascript_code .= javascript_rhs($v, true) . ($i++ < $n ? ", " : "");
				}
				$javascript_code .= ");\n";
			}
			else {
				// The array is an associative array (the first key is not an integer)
				// Therefore create a javascript object
				$javascript_code .= "$js_prefix$member={";
				$n = count($object->$member);
				// NEW 23.11.06 (Markus): old code didn't worked when keys of the array ($p) are not a integer index (as in $AC->settings->varnames_abbreviations)
				$i = 1;
				foreach ($object->$member as $p => $v)
				{
					$javascript_code .= javascript_rhs2($p, $v, true) . ($i++ < $n ? ", " : "");
				}
				$javascript_code .= "};\n";
			}
			*/
		}
		// CASE 2: scalar
		else
		{
			// !!! CHECK: is the true here (urldecode) a problem for autocomplete.php ???
			$javascript_code .= "$js_prefix$member=" . javascript_rhs($value, $urlencode) . ";\n";
		}
	}
	return $javascript_code;
}
/**
 * Generate javascript code from a php member variable of $object.
 * The member can be an array or a scalar but not an object.
 *
 * @param object $object The object which member we consider
 * @param string $member The name of the member
 * @param string $js_prefix The string prefix which precedes the member
 * @param boolean $urlencode If true urlencode $member's value addionally
 * @return string
 */
function variable_in_javascript3(&$object, $member, $js_prefix, $urlencode = false)
{
	global $AC;  // For using the AC::log methods
	
	// CASE 1: array
	if (is_array($object->$member))
	{
		// NEW 19-08-08 / Markus
		if (sizeof($object->$member) == 1) 
		{
			// NEW 06-09-08 / Markus: Here reset() must be called before each() to be sure we are at the beginning of the array
			reset($object->$member);
			list($key, $value) = each($object->$member);
//			$AC->log->write("$member: $key, $value", $AC->log->levels['DEBUG'], SCRIPT_GENERATE_JAVASCRIPT);
			return "$js_prefix$member" . "[$key]=" . array_in_javascript($value, $urlencode) . ";\n";
		}
		else 
		// END NEW 19-08-08 / Markus
			return "$js_prefix$member=" . array_in_javascript($object->$member, $urlencode) . ";\n";
	}
	// CASE 2: scalar
	else
	{
		// TODO !!! CHECK: is the true here (urldecode) a problem for autocomplete.php ???
		return "$js_prefix$member=" . javascript_rhs($object->$member, $urlencode) . ";\n";
	}
}


/**
 * Write a PHP array in javascript style (associative array); works recursivly.
 * Delivers for example: completions={0 : 'artic', 1 : 'article', 2 : 'articles', 3 : 'articulata', 4 : 'articleid'};
 */
function array_in_javascript($v, $urlencode = false)
{
  // CASE 1: array
  if (is_array($v))
  {
    $i = 1;
    $n = count($v);

    // NEW 14-01-07 (Markus): distinguish between "simple" (non-associative) arrays and associative arrays (hashes)
    if (is_sequence($v))
    {
      $javascript_code = "new Array(";

      $n = count($v);
      foreach ($v as $p1 => $v1)
      {
        $javascript_code .= array_in_javascript($v1, $urlencode) . ($i++ < $n ? ", " : "");
      }
      $javascript_code .= ")";
    }
    else
    {
      $javascript_code = "{";

      foreach ($v as $p1 => $v1)
      {
        $javascript_code .= "'$p1' : " . array_in_javascript($v1, $urlencode) . ($i++ < $n ? ", " : "");
      }
      $javascript_code .= "}";
    }
  }
	// CASE 2: scalar
	else
	{
		$javascript_code = javascript_rhs($v, $urlencode);
	}
	return $javascript_code;
}


/**
 * Make right hand side for javascript assignment
 *
 * If $value is an integer return it unchanged.
 * If $value is a string put it in single quotes.
 * Otherwise add back slashes to ', " and \ to prevent trouble with occurences of these characters.
 *
 * @param unknown_type $value value which should be pretty printed
 * @param bool $urlencode if true urlencode $value addionally
 * @return the pretty printed version of $value
 */
function javascript_rhs($value, $urlencode = false)
{
	// if called for an object or array --- don't do it --- returns empty string
	if (!isset($value) || is_object($value) || is_array($value)) { return "''"; }

	// return any number as is
	if (is_integer($value) || is_float($value)) { return $value; }
	// either escape
	if ($urlencode) { return "'" . rawurlencode($value) . "'"; }
        // or add slashes to ' or " or \
	else { return "'" . addslashes((string) $value) . "'"; }
}


/**
 * Slightly modified version of javascript_rhs which extends the result of javascript_rhs
 * with a preceding "'$property': "
 *
 */
function javascript_rhs2($property, $value, $urlencode = false)
{
	return "'$property': " . javascript_rhs($value, $urlencode);
}


/**
 * Update $obj1 from $obj2.
 * Is used to write all changes in the $NAC object to the $AC object (which is saved in the current session).
 * Optionally, all changes can be written as Javascript code (which will be used to answer to client's request).
 *
 * @param object $obj1
 * @param object $obj2
 * @param boolean $generate_javascript If set to true all changes are written as Javascript code
 * @param string $prefix Is no longer used
 */
/*
function update_normal(&$obj1, $obj2, $generate_javascript = false, $prefix = "")
{
	global $AC;  // For retrieving the encoding and using the AC::log methods

	if ($prefix == "") $prefix = get_class($obj1);
	$prefix .= "->";

	if (is_object($obj2))
	{
	  // Check all members of object $obj2 whether they have a defined value
	  // (these are exactly these members we want to copy to object $obj1)
		foreach (get_object_vars($obj2) as $key => $value) {
			if (isset($value))
			{
				if (is_object($value))
				{
					$AC->log->write("Update $prefix$key => $value", $AC->log->levels['DEBUG'], SCRIPT_GENERATE_JAVASCRIPT);
					update($obj1->$key, $value, $generate_javascript, $prefix . $key);
				}
				else
				{
					$obj1->$key = $value;
					$AC->log->write("Update $prefix$key => $value", $AC->log->levels['DEBUG'], SCRIPT_GENERATE_JAVASCRIPT);
					if ($generate_javascript) {
					  // NEW 10-01-07 (Markus): write all array as associative arrays
						echo variable_in_javascript3($obj1, $key, "", $AC->settings->encoding == "utf-8" ? false : true);
//						echo variable_in_javascript2($obj1, $key, "", $AC->settings->encoding == "utf-8" ? false : true);
//						$AC->log->write(variable_in_javascript3($obj1, $key, "", $AC->settings->encoding == "utf-8" ? false : true));
					}
				}
//				else $AC->log->write("Update [$obj1]: ++$key => $value");
			}
		}
	}
	else
	{
		$obj1 = $obj2;
		$AC->log->write("Update $prefix$obj1 => $obj2", $AC->log->levels['DEBUG'], SCRIPT_GENERATE_JAVASCRIPT);
		if ($generate_javascript) {
      // Now (after element is updated) write changements of the element as javascript code if the corresponding option is set
		  // NEW 10-01-07 (Markus): write all array as associative arrays
			echo variable_in_javascript3($obj1, "", "", $AC->settings->encoding == "utf-8" ? false : true);
		}
	}
}
*/


function update(&$obj1, $obj2, $generate_javascript = false, $prefix = "")
{
	global $AC;  // For retrieving the encoding and using the AC::log methods
	
	if ($prefix == "") $prefix = get_class($obj1);
	$prefix .= "->";

	if (is_object($obj2))
	{
	  // Check all members of object $obj2 whether they have a defined value
	  // (these are exactly these members we want to copy to object $obj1)
		foreach (get_object_vars($obj2) as $key => $value) 
		{
			// Only if $value is set this member is changed during the last query/computation and only these ones must be considered
			if (isset($value))
			{
				if (is_object($value))
				{
//					$AC->log->write("Update $prefix$key => $value", $AC->log->levels['DEBUG']);
					update($obj1->$key, $value, $generate_javascript, $prefix . $key);
				}
				else
				{
          // NEW 28-02-08 (Markus): do the following only for associative arrays
  	      // *** Merge *** (not override!) arrays to only update those elements of the array which are changed
  	      // Means: instead of copying the whole array replace only single key/value pairs (those ones which are in $value)
  	      // Background: if only the (for example) second F box was computed by this request ...
	        if (is_array($value) && !is_sequence($value))
	        {
	        	foreach ($value as $p => $v)
	        	{
	        		$obj1_array = &$obj1->$key;
	        		// Possibly the array of $obj1 is not yet initialized; if so set it to a new array
	        		if (! is_array($obj1_array)) 
	        		{
	        			$obj1_array = array();
	        		}
//	        		$AC->log->write("Update $key" . "[$p]", $AC->log->levels['DEBUG']);
	        		$obj1_array[$p] = $v;
	        	}
	        }
	        // Non-associative arrays or scalar types of $obj1 members set simply to their corresponding value in $obj2
	        else 
	        {
//	        	$AC->log->write("Update $prefix$key => $value", $AC->log->levels['DEBUG'], SCRIPT_GENERATE_JAVASCRIPT);
	        	$obj1->$key = $value;
	        }

		      // Now (after element is updated) write changements of the element as javascript code if the corresponding option is set
	        if ($generate_javascript) {
	        	// NEW 29-05-08 (Markus): using $obj2 writes only the changed part of boxes, for example only the third F box instead of all F boxes
	        	// using $obj1 writes the complete boxes array, for examples first, second, and third F box (which is not necessary and uses more network traffic)
        		echo variable_in_javascript3($obj2, $key, "", $AC->settings->encoding == "utf-8" ? false : true);
//        		echo variable_in_javascript3($obj1, $key, "", $AC->settings->encoding == "utf-8" ? false : true);
        		// $AC->log->write(variable_in_javascript3($obj1, $key, "", $AC->settings->encoding == "utf-8" ? false : true));
        	}
				}
			}
		}
	}
	else
	{
//		$AC->log->write("Update $prefix$obj1 => $obj2", $AC->log->levels['DEBUG'], SCRIPT_GENERATE_JAVASCRIPT);
		$obj1 = $obj2;
    // Now (after element is updated) write changements of the element as javascript code if the corresponding option is set
		if ($generate_javascript) {
			echo variable_in_javascript3($obj1, "", "", $AC->settings->encoding == "utf-8" ? false : true);
		}
	}
}

?>
