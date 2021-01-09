<?php

// This includes the class AC with all the autocomplete functionality
include("AC.php");

/*
	Get the application name to know which session to resume
*/
if (isset($_POST["session_name"])) {
  $session_name = $_POST["session_name"];
}
else {
  $session_name = $_GET["session_name"];
}

// Check whether page is called with application/session context
if ($session_name == "")
{
	echo "Fatal: missing url parameter 'session_name', I don't know which session to resume.";
	exit();
}

/*
Start or resume session and get current AC object
*/
session_name($session_name);
session_start();

$AC = &$_SESSION["AC"];

$AC->javascript = true;
$AC->state = "changed";

// The following output has no deeper meaning except to provide some output
// (to let the output display of firebug terminate, for example)
echo "ok";

session_commit();
?>