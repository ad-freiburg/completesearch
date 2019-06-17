<?php

include("request_init.php");

// Get the application name to know which session to resume
$session_name = isset($_POST['session_name']) ?  $_POST['session_name'] : $_GET['session_name'];

// Check whether page is called with application/session context
if ($session_name == "") {
	echo "<h1>ERROR in unset.php: missing url parameter 'session_name', don't know which session to unset</h1>";
	exit();
}

//! GET SESSION UNSET
session_name($session_name);

session_start();

// Unset all of the session variables.
$_SESSION = array();

// If it's desired to kill the session, also delete the session cookie.
// Note: This will destroy the session, and not just the session data!
if (isset($_COOKIE[session_name()])) {
    setcookie(session_name(), '', time() - 42000, '/');
}

// Finally, destroy the session.
session_destroy();

echo "ok";

?>