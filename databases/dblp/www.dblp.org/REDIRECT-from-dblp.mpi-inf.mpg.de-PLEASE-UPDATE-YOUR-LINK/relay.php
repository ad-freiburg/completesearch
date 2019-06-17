<?php
  error_reporting(E_ALL);

  // We must provide a value for index_page to override the name of the relay script 
  //	(otherwise in the HTML links of the boxes would appear relay.php?query=... instead of index.php?query=...)
	$config->index_page = "index.php";
	
	// If we are sure that Javascript is enabled here is the best (because: earliest) place to set AC memeber javascript to true.
	// If it's set here all links of the consecutive page index.php are Javascript links 
	$config->javascript = true;
	
	require("autocomplete_config.php");
  require($_SERVER['DOCUMENT_ROOT'] . $config->autocomplete_path . "/autocomplete.php");

  // Set state of the session AC object to "relay" to cause a special threatment in the index.php
	// As we are using cookie-based sessions, we must call session_name() and session_start() before anything is outputted to the browser. 
  session_name($config->session_name);
  session_start();
  $_SESSION["AC"]->state = "relay";
  session_commit();
  
  $path_info = preg_replace("|relay.php$|", "index.php", $_SERVER["PHP_SELF"]);
  // The following would result in a call of index.php with URL parameters. In javascript enabled mode that would damage the back functionality
  if (isset($_SERVER["QUERY_STRING"]) && $_SERVER["QUERY_STRING"] != "") $path_info .=  "#" . $_SERVER["QUERY_STRING"];
  header("Location: $path_info");
?>
