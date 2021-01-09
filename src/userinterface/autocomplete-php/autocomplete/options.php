<?php
error_reporting(E_ERROR);
ob_start();
?>
<!DOCTYPE html>
<!-- Very important thst the line above ^^^^ is the first line the browser find!
      If not, the quirks mode is used instead of standard mode
-->

<?php

// The name of this script (used to identify log message)
//define("SCRIPT_OPTIONS", "options");
$path_info = pathinfo(__FILE__);
define("SCRIPT_OPTIONS", $path_info["basename"]);

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


include_once("times.php"); // functions and variables for time measurement
include_once("helper.php");
include_once("generate_javascript.php");


/**
 * GET CURRENT AC OBJECT
 */
saveTimestamp('before: include(AC_init.php)');
require("AC_init.php");
saveTimestamp('after: include(AC_init.php)');

// NEW 20.03.07 (Markus): Check whether cookies are enabled
$AC->cookies = (isset($_COOKIE["cookie_test"]) && $_COOKIE["cookie_test"] == "enabled");

// Try to set a test cookie to see with the next request whethter the cookie was set and send
if (! $AC->cookies) {
	setcookie("cookie_test", "enabled");
}

// Build back to index page link depending on whether cookies are enabled or not
if ($AC->cookies) {
	$back = $AC->settings->index_path . $AC->settings->index_page;
}
else {
	$back = url_append($AC->settings->index_path . $AC->settings->index_page, strip_tags(SID));
}
?>

<html>
	<head>
		<meta http-equiv="content-type" content="text/html;charset=<?php echo $AC->settings->encoding; ?>">
		<title><?php echo $AC->settings->html_title; ?></title>
		<link rel=stylesheet type="text/css" href="<?php echo $AC->settings->css_file;?>">
		<!-- Own style sheet for internet explorer versions less than IE 7 -->
		<!--[if lt IE 7]>
		<link rel=stylesheet type="text/css" href="<?php echo str_replace('.css', '_ie5-6.css', $AC->settings->css_file);?>">
    <![endif]-->
		<!-- Own style sheet for internet explorer versions equal to IE 5.5 which only defines what differs from the CSS file above -->
		<!--[if lte IE 5.5]>
		<link rel=stylesheet type="text/css" href="<?php echo str_replace('.css', '_ie55.css', $AC->settings->css_file);?>">
    <![endif]-->

		<script type="text/javascript" src="<?php echo get_full_url($AC->settings->autocomplete_path); ?>utils.js"></script>

		<script type="text/javascript">
		<!--
			function restore_defaults()
			{
				<?php
					foreach ($AC->settings->user_preferences as $pref)
					{
							echo "\n\t\t\t\tdocument.getElementById('" . $AC->settings->varnames_abbreviations[$pref] . "').value = '{$AC->default_settings->$pref}';";
					}
				?>
			}
		//-->
		</script>
	</head>

	<body>
		<!-- LAYOUT WITH TWO COLUMNS: queries (left) and hits (right) -->
		<div id="page">

		  <div id="left_bg"></div>
			<div id="left">

  			<!-- Progress image beside of logo -->
  			<div id="logo">
          <!-- NEW 05.08.06 (Markus): graphic for logo changed (was titel.gif) -->
					<a href="http://search.mpi-inf.mpg.de"><img src="<?php echo $AC->settings->index_url; ?>/images/logo.gif" border="0"/></a>
          <!-- <a href="http://geek2.ag1.mpi-sb.mpg.de"><img src="images/logo.gif" border="0"/></a> -->
  			</div>

				<div style="margin: 80px 10px 0px 0px; font-size: 11pt;">
					<?php //echo $AC->get_text("options_notes");?>
					<?php echo $AC->get_text($AC->cookies ? "options_notes_cookies_allowed" : "options_notes_cookies_not_allowed");	?>
				</div>
			</div>	<!-- id="left", END OF LEFT COLUMN -->

			<!-- RIGHT COLUMN IS FOR HITS -->
			<div id="right">

				<div id="title">
					<h1 style="margin: 0px; padding: 0px;"><?php echo $AC->get_text("options_title");?></h1>
				</div>

				<div style="position: relative; top: 0px; padding-bottom: 20px">
					<?php echo $AC->get_text("options_subtitle");?>
				</div>

				<form id="options_form" action="change_options.php" method="GET">
					<input type="hidden" name="session_name" value="<?php echo $AC->settings->session_name;?>">
					<input type="hidden" name="path" value="<?php echo $AC->settings->index_path;?>">
					<input type="hidden" name="page" value="<?php echo $AC->settings->index_page;?>">
					<input type="hidden" name="log" value="<?php echo $AC->settings->access_log;?>">
					<!-- NEW 26.01.07 (Markus): support now session handling without cookies using explicit passing of name and ID -->
					<?php
						if (! $AC->cookies) {
							echo '<input type="hidden" name="' . session_name() . '" value = "' . session_id() . '">';
						}
					?>
					<table style="border-spacing:0px; border-collapse: collapse;">
						<?php
							$flag = true;
							foreach ($AC->settings->user_preferences as $pref)
							{
								$tmp = $AC->settings->varnames_abbreviations[$pref];
								if ($flag) {
									$style = "background-color: rgb(229, 229, 229);";
								}
								else {
									$style = "background-color: transparent;";
								}
								$flag = !$flag;

								echo "\n<tr>";
								echo "<td class='option' style='$style'>" . $AC->get_text($pref) . "&nbsp;</td>";
								echo "<td style='vertical-align:middle; border: 0px solid;'>";
								echo "<input class='input' id='{$tmp}' name='{$tmp}' size='3' value='{$AC->settings->$pref}'>";
								echo "</td>";
								echo "</tr>";
							}
						?>

						<tr>
							<td style="height:40px; vertical-align: bottom; text-align:right;">
								<!-- The restore button should be only visible if javascript is enabled -->
								<input type="button" id="restore" value="<?php echo $AC->get_text('button_restore_defaults');?>" style="visibility: hidden" onclick="restore_defaults()">
							</td>
							<td></td>
						</tr>
						<tr>
							<td style="height:30px; text-align:right;">
								<input type="submit" id="ok" value="<?php echo $AC->get_text('button_apply');?>">
							</td>
							<td></td>
						</tr>
					</table>
				</form>

				<noscript>
  				<p style="margin-top: 30px">
  				To go back without changes use the browser back button, please!
  				</p>
				</noscript>
				
				<!-- Now the back button -->
				<table id="back" style="border-spacing: 0px; border-collapse: collapse; display: none;">
					<tr>
						<td style="height: 40px; width: 28px; background: url('<?php echo $AC->settings->index_url; ?>/images/back_1.gif') no-repeat;"></td>
						<td style="height: 40px; background: url('<?php echo $AC->settings->index_url; ?>/images/back_2.gif') repeat-x; vertical-align: middle; padding: 0px 3px 0px 3px;">
      				<button type="button" style="border: 0px; background: transparent; margin: 0px; padding: 0px" onclick="history.back()">
      				  <?php echo $AC->get_text('button_back');?>
      				</button>
							</td>
						<td style="height: 40px; width: 28px; background: url('<?php echo $AC->settings->index_url; ?>/images/back_3.gif') no-repeat;"></td>
					</tr>
				</table>
				
		</div>	<!-- id="right", END OF RIGHT COLUMN -->

    <script type="text/javascript">
    <!--
      // Correct the padding-left of the right box
//      var left = document.getElementById("left");
//      var right = document.getElementById("right");
//      right.style.paddingLeft = (left.offsetWidth + getStyle(right, 'paddingLeft')) + "px";

      // Since javascript is enabled show the restore button
    	var node = document.getElementById("restore");
    	if (node) { 
      	node.style.visibility = 'visible';
    	}
      // Since javascript is enabled show the back button
    	var node = document.getElementById("back");
    	if (node) { 
    	  node.style.display = 'block';
    	}
    //-->
    </script>

	</body>
</html>
