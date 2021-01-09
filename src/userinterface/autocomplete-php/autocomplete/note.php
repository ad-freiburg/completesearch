<?php

//error_reporting(E_NOTICE);
error_reporting(E_ERROR);

// The name of this script (used to identify log message)
define("SCRIPT_OPTIONS", "options");

ob_start();

// FOR TESTING ONLY!! This provocates the AC object to be created from scratch
//session_name($_POST['session_name']);
//session_start();
//unset ($_SESSION['AC']);


/**
 * THE FOLLOWING VALUES WE HAVE TO KNOW TO RESUME THE SESSION OR TO CRATE AC OBJECT FROM SCRATCH (reading configuration from the autocomplete_config.php)
 */
// Path to the error log file as first to provide error handling as soon as possible
$config->error_log = isset($_POST['log']) ?  $_POST['log'] : $_GET['log'];

// Provide own error handling and set the system error handling to our handling
include_once("error_handler.php");

$config->index_path = isset($_POST['path']) ?  $_POST['path'] : $_GET['path'];
$config->index_page = isset($_POST['page']) ?  $_POST['page'] : $_GET['page'];

// The name to identify the session
$config->session_name = isset($_POST['session_name']) ?  $_POST['session_name'] : $_GET['session_name'];


include_once("helper.php");
include_once("times.php"); // functions and variables for time measurement
include_once("generate_javascript.php");

//
$note_topic = variable("note_topic");
$note_title = variable("note_title");
$note_body = variable("note_body");

/**
 * GET CURRENT AC OBJECT
 */
require("AC_init.php");

$NAC = new AC();

// We show this note now, so disable it for the future
$NAC->note[$note_topic] = 0;

// NEW 20.03.07 (Markus): Check whether cookies are enabled
$NAC->cookies = ($_COOKIE["cookie_test"] == "enabled");

// Try to set a test cookie to see with the next request whethter the cookie was set and send
if (! $NAC->cookies) {
	setcookie("cookie_test", "enabled");
}

/**
 * Save the changements to the session's AC object
 */
// Set the session name to the application name
session_name($AC->settings->session_name);
// Initialize session
session_start();

update($_SESSION['AC'], $NAC);

session_commit();

// Build back to index page link depending on whether cookies are enabled or not
$back = $AC->settings->index_path . $AC->settings->index_page;
if (! $AC->cookies) {
	$back = url_append($back, strip_tags(SID));
//	$back = $AC->url_append($AC->settings->index_path . $AC->settings->index_page . "?session_name={$AC->settings->session_name}", strip_tags(SID));
}

?>

<html>
	<head>
		<meta http-equiv="content-type" content="text/html;charset=<?php echo $AC->settings->encoding; ?>">
		<title><?php echo $AC->settings->html_title; ?></title>
		<link rel=stylesheet type="text/css" href="<?php echo $AC->settings->css_file;?>">
	</head>
	<body>
		<!-- LAYOUT WITH TWO COLUMNS: queries (left) and hits (right) -->
		<div id="page">

		  <div id="left_bg" style=""></div>
			<div id="left">

  			<!-- Progress image beside of logo -->
  			<div id="logo">
          <!-- NEW 05.08.06 (Markus): graphic for logo changed (was titel.gif) -->
					<a href="http://search.mpi-inf.mpg.de"><img src="<?php echo $AC->settings->index_url; ?>/images/logo.gif" border="0"/></a>
          <!-- <a href="http://geek2.ag1.mpi-sb.mpg.de"><img src="images/logo.gif" border="0"/></a> -->
  			</div>

				<div style="margin: 80px 10px 0px 0px; font-size: 11pt;">
				</div>
			</div>	<!-- id="left", END OF LEFT COLUMN -->

				<!-- RIGHT COLUMN IS FOR HITS -->
			<div id="right">

				<div id="title">
					<h1 style="margin: 0px; padding: 0px;"><?php echo $AC->get_text($note_title);?></h1>
<!--					<h1 style="margin: 0px; padding: 0px;"><?php echo $AC->get_text("javascript_note_title");?></h1>-->
				</div>

				<div style="position: relative; top: 0px; padding-bottom: 20px">
					<?php echo $AC->get_text($note_body);?>
<!--					<?php echo $AC->get_text("javascript_note");?>-->
				</div>

				<form id="back_form" method="post" action="<?php echo $back;?>">
					<input type="hidden" id="session_name" value="{$AC->settings->session_name}"></input>

					<table style="border-spacing: 0px; border-collapse:collapse">
						<tr>
							<td style="height: 40px; width: 28px; background: url('<?php echo $AC->settings->index_url; ?>/images/back_1.gif') no-repeat;"></td>
							<td style="height: 40px; background: url('<?php echo $AC->settings->index_url; ?>/images/back_2.gif') repeat-x; vertical-align: middle; padding: 0px 3px 0px 3px;">
								<!-- NEW 26.01.07 (Markus): support now session handling without cookies using SID -->
								<button type="submit" style="border: 0px; background: transparent; margin: 0px; padding: 0px"><?php echo $AC->get_text('button_back');?></button>
							</td>
							<td style="height: 40px; width: 28px; background: url('<?php echo $AC->settings->index_url; ?>/images/back_3.gif') no-repeat;"></td>
						</tr>
					</table>
				</form>


		</div>	<!-- id="right", END OF RIGHT COLUMN -->

	</body>
</html>
