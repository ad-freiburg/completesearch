<?php 
  ob_start(); 
  // The path to the tools folder (relative to web server's document root, with leading and trailing slash!)
  $tools_path = "/autocomplete-php.NEW_MARKUS/tools/";
//  $tools_path = "/MPI/autocomplete-php/tools/";
?>

<html>
  <head>
    <style>
      body { font-family: arial; color: rgb(100, 100, 100); }
      .name { color: rgb(170, 150, 150); margin: 3px; font-size: small; }
      .value { color: rgb(170, 100, 100); margin: 3px; font-size: small; }
    </style>
  </head>

  <body>

    <h2>Inspect</h2>
    <div id="info"></div>


<?php

  $index_path = "../../";
  if (isset($_GET["index_path"])) {
    $index_path = $_GET["index_path"];
  }
  include $index_path . "autocomplete_config.php";

	include $_SERVER['DOCUMENT_ROOT'] . $config->autocomplete_path . "AC.php";
	include $_SERVER['DOCUMENT_ROOT'] . $config->autocomplete_path . "log.php";
	include $_SERVER['DOCUMENT_ROOT'] . $config->autocomplete_path . "helper.php";

	// Get the session name
	if (! isset($config->session_name)) {
		$config->session_name = $config->application_name;
	}

	// As we are using cookie-based sessions, we must call session_name() and session_start() before anything is outputted to the browser.
  session_name($config->session_name);
  session_start();

  echo "<h2>PHP</h2>";
  echo "<li>PHP version: " . phpversion() . " [ <a target='Info' href='phpinfo.php'>phpinfo()</a> ]</li>";

  echo "<h2>Session</h2>";
  echo "<li>Session name: '$config->session_name'</li>";

  if (isset($_SESSION['AC']))
  {
  	$AC = & $_SESSION["AC"];
  	echo "<li>AC exists in \$_SESSION: yes</li>";
  	if (is_a($_SESSION["AC"], "AC")) {
  		echo "<li>Type of AC: AC</li>";
  	}
  	else {
  		echo "<li>Type of AC: <font style='color:red'>***not***</font> AC  [";
  		$tmp = print_r($_SESSION, true);
  		echo "<font style='color:red'>" . substr($tmp, 0, 100) . " ...)</font>";
  		echo "]</li>";
  	}
  	echo "Session <a href=\"javascript:reset_session('{$config->session_name}');\">reset</a><span id=\"reset-message\"></span>";
  }
  else {
  	echo "<li>AC exists in \$_SESSION: no</li>";
  	exit;
  }


  function draw($name, $value, $level)
  {
  	if (is_object($value)) {
  		echo "\nnotes.add(\"<font class=\'name\'>$name:</font>  <font class=\'value\'>Object of class " . strtoupper(get_class($value)) . "</font>\", $level, $level == 1);";
  		foreach (get_object_vars($value) as $name2 => $value2) {
  			draw($name2, $value2, $level + 1);
  		}
  	}
  	else {
  		if (is_array($value)) {
  			echo "\nnotes.add(\"<font class=\'name\'>$name:</font>  <font class=\'value\'>[] # = " . sizeof($value) . "</font>\", $level, $level == 1);";
  			foreach ($value as $name2 => $value2) {
  				draw($name2, $value2, $level + 1);
  			}
  		}
  		else {
  			if (is_string($value)) {
  				// Remove line breaks, HTML tags and HTML entities
  				//	        $value = htmlentities(strip_tags(preg_replace("/\r|\n/s", "", $value)));
  				$value = htmlentities(preg_replace("/\r|\n/s", "", $value));
  			}
  			echo "\nnotes.add(\"<font class=\'name\'>$name:</font>  <font class=\'value\'>$value</font>\", $level, $level == 1);";
  		}
  	}
  }

?>

<link rel=stylesheet type="text/css" href="<?php echo get_full_url($config->autocomplete_path); ?>logging.css">

<script src="<?php echo get_full_url($config->autocomplete_path); ?>logging.js"></script>
<script src="<?php echo get_full_url($config->autocomplete_path); ?>utils.js"></script>
<script src="<?php echo get_full_url($tools_path); ?>Ajax.js"></script>

<script type="text/javascript">

//This is a hack (Notes class ask AC for log level)
//var AC = {"log": {"level": 4}};
var notes = new Notes();
notes.clear();
notes.add("Session", 0, true);


<?php
//  foreach (get_object_vars($AC) as $name => $value) {
foreach ($_SESSION as $name => $value) {
  draw($name, $value, 1);
}
session_commit();
?>

window.onload = init;

function init()
{
	var infobox = document.getElementById("info");
	if (infobox) {
		infobox.appendChild(notes.toDOM());
	}
}

function reset_session(name)
{
	var t = document.getElementById("reset-message");
	if (t) {
		t.innerHTML = ": working ...";
	}
	Ajax.send("<?php echo get_full_url($tools_path); ?>ajax_requests/reset_session.php", "session_name=" + name, reset_session_callback);
}

function reset_session_callback(response)
{
	var t = document.getElementById("reset-message");
	if (t) {
		t.innerHTML = ": " + response;
	}
}

</script>

  </body>
</html>