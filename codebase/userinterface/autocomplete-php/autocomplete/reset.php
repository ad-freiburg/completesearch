<?php

// Get the application name to know which session to resume
$session_name = isset($_POST['session_name']) ?  $_POST['session_name'] : $_GET['session_name'];
$index_url = isset($_POST['index_url']) ?  $_POST['index_url'] : $_GET['index_url'];

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

// NEW Markus / 05-05-09: Go back to index page using header() which is independant from Javascript enabled
header("Location: $index_url");
?>


