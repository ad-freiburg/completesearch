<?php 
// Start output buffering; it's necessary to support cookies to be set/sent in the following
// (without output buffering with the following "DOCTYPE" HTTP headers are sent and so cookies cannot set (and sent) anymore
ob_start(); 
?>
<!DOCTYPE html>
<!-- Very important thst the line above ^^^^ is the first line the browser find!
      If not, the quirks mode is used instead of standard mode.
      But be careful: if you want to sent cookies later be sure to use PHP output buffering (with ob_start())
-->
<?php


// The name of this script (used to identify log message)
//define("SCRIPT_CHANGE_OPTIONS", "change_options");
$path_info = pathinfo(__FILE__);
define("SCRIPT_CHANGE_OPTIONS", $path_info["basename"]);


// FOR TESTING ONLY!! This provocates that the AC object must be created from scratch
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

// Now the remaining $config variables
$config->index_path = isset($_POST['path']) ?  $_POST['path'] : $_GET['path'];
$config->index_page = isset($_POST['page']) ?  $_POST['page'] : $_GET['page'];

// The name to identify the session
$config->session_name = isset($_POST['session_name']) ?  $_POST['session_name'] : $_GET['session_name'];


include_once("times.php"); // functions and variables for time measurement
include_once("helper.php");
include_once("generate_javascript.php");

/**
 * GET CURRENT AC OBJECT
 */
//saveTimestamp('before: include(AC_init.php)');
require("AC_init.php");
//saveTimestamp('after: include(AC_init.php)');


$NAC = new AC();

/*
	Change AC object according to the new values
*/
//echo "<br>user_preferences: "; print_r($AC->settings->user_preferences);
foreach ($AC->settings->user_preferences as $pref)
{
	// Abbreviation of the preference
	$abbreviation = $AC->settings->varnames_abbreviations[$pref];
	$v = $_GET[$abbreviation];
//	echo "<br>abbreviation=$abbreviation --> $v";
	// NEW 20.12.06 (Markus): it's must be allowed to set a preference to empty string, for example for query_types
	if (isset ($v))
//		if (isset ($v) && $v <> "")
	{
		// NEW 20.12.06 (Markus)
		if ($AC->settings->$pref != $v)
		{
			if (! isset($NAC->query)) {
				$NAC->query = new Query();
			}
			if (! isset($NAC->settings)) {
				$NAC->settings = new Settings();
			}
			$NAC->query->$pref = $v;
			$NAC->settings->$pref = $v;
			$NAC->state = "changed";
		}
		setcookie ($abbreviation, $v, time()+60*60*24*30, $AC->settings->index_path);
		$AC->log->write ($abbreviation ." ". $v, $AC->log->levels['DEBUG'], SCRIPT_CHANGE_OPTIONS, "", __FUNCTION__, __LINE__);
	}
}
//echo ("<br>qt" . " " . $NAC->query->query_types);

// NEW 12-12-07 (Markus)
// Note: $AC->first_hit is correct, not $NAC->first_hit!
// The new AC member last_hit must be updated according to a changed value of hits_per_page_while_typing
// (last_hit has been necessary beacuse of "more" mechanism of hits list)
$NAC->last_hit = $AC->first_hit + $NAC->query->hits_per_page_while_typing - 1;

// UNCOMMENT 07-08-07 (Markus): A is now explicit member of query_types again
// For reasons of backwards compatibility: allow redundant 'A' in the query types string in $config and remove it now if it exists
/*
if (isset($NAC->query->query_types) && $NAC->query->query_types != '')
{
	$v = str_replace('A', '', $NAC->query->query_types);
	$NAC->query->query_types = $v;
	$NAC->settings->query_types = $v;
	$NAC->state = "changed";
	setcookie ("qt", $v, time()+60*60*24*30, $AC->settings->index_path);
	$AC->log->write ("qt" . " " . $v, $AC->log->levels['DEBUG'], SCRIPT_CHANGE_OPTIONS);
}
*/

/**
 * Save the changements to the session's AC object
 */
// Set the session name to the application name
session_name($AC->settings->session_name);
// Initialize session
session_start();

update($_SESSION['AC'], $NAC);

//print_r($NAC->settings);
session_commit();

// NEW Markus / 05-05-09: Go back to index page using header() which is independant from Javascript enabled
header("Location: " . $AC->settings->index_path . $AC->settings->index_page);
// This is the END (of the script ;-)
// Left the following for a while ...
// Build back to index page link depending on whether cookies are enabled or not
$back = $AC->settings->index_path . $AC->settings->index_page;
if (! $AC->cookies) {
	$back = url_append($back, strip_tags(SID));
}
?>

<html>
	<head>
		<meta http-equiv="content-type" content="text/html;charset=<?php echo $AC->settings->encoding; ?>">
		<title>Wikipedia mit Autovervollständigung</title>
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
					<?php echo $AC->get_text("options_changed_note");?>
					<?php //echo $AC->get_text($AC->cookies ? "options_notes_cookies_allowed" : "options_notes_cookies_not_allowed");	?>
				</div>
			</div>	<!-- id="left", END OF LEFT COLUMN -->

			<!-- RIGHT COLUMN IS FOR HITS -->
			<div id="right">

				<div id="title">
					<h1 style="margin: 0px; padding: 0px;"><?php echo $AC->get_text("options_changed_title");?></h1>
				</div>

				<div style="position: relative; top: 0px; padding-bottom: 20px">
					<?php //echo $AC->get_text("options_changed_note");?>
				</div>

				<!-- Now the back button -->
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

				<script type="text/javascript">
				<!--
					document.getElementById("back_form").submit();
				//-->
				</script>

		</div>	<!-- id="right", END OF RIGHT COLUMN -->

	</body>
</html>
