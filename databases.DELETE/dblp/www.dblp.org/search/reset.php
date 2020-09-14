<?php

// Get the application name to know which session to resume
$session_name = isset($_POST['session_name']) ?  $_POST['session_name'] : $_GET['session_name'];
$index_url = isset($_POST['index_url']) ?  $_POST['index_url'] : $_GET['index_url'];

// Check whether page is called with application/session context
if ($session_name == "") {
	echo "<h1>ERROR in unset.php: missing url parameter 'session_name', don't know which session to unset</h1>"; exit(); }

//! GET SESSION UNSET
session_name($session_name);

session_start();

// Unset all of the session variables.
$_SESSION = array();

// If it's desired to kill the session, also delete the session cookie.
// Note: This will destroy the session, and not just the session data!
if (isset($_COOKIE[session_name()])) {
    setcookie(session_name(), '', time()-42000, '/');
}

// Finally, destroy the session.
session_destroy();

//session_unset();
//session_destroy();

//setcookie('qt', "", time() - 3600);
//setcookie('qt', '', 0, $this->settings->index_path);

?>

<html>
	<head>
		<title>AC --- Session unset</title>
  </head>

	<body>
  <script language="javascript">
  location = "<?php echo $index_url; ?>";
  </script>
  <noscript>
    <?php echo "<h1>Unset session \"" . $session_name . "\"</h1>" ?>
		<a href="<?php echo $index_url; ?>">Back to search page</a>
<!--		<a href="index.php">Back to search page</a>-->
  </noscript>
  </body>
</html>
