<?php
  // The following is very important to force browsers to load this page from server,
  //  especially after a back from external pages
  //header( "Last-Modified: " . gmdate( "D, j M Y H:i:s" ) . " GMT" );
  //header( "Expires: " . gmdate( "D, j M Y H:i:s", time() ) . " GMT" );
  header( "Cache-Control: no-store, no-cache, must-revalidate" );
  // HTTP/1.1 Header( "Cache-Control: post-check=0, pre-check=0", FALSE );
  //header( "Cache-Control: post-check=0, pre-check=0", FALSE );
  header( "Pragma: no-cache" );
  ob_start();
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<!-- Very important that the line above ^^^^ is the first line the browser find!
      If not, the quirks mode is used instead of standard mode
-->
<html>
  <head>
    <?php
      // Version of this completion server application
      define("VERSION", '1.0.7');
      error_reporting(E_ALL);
    
      /* USER CONFIG FILE, ESSENTIAL! */
      require("autocomplete_config.php");
      // The following line causes invalide HTML code (because of the DOCTYPE which is not at the beginning).
    	// Do the DOCTYPE at the beginning results in the effect that when using Firefox the autocomplete_query input-Tag is empty
    	// when we came from a external link or from the options page
    	// Internet Explorer and Opera work fine
    	require($_SERVER['DOCUMENT_ROOT'] . $config->autocomplete_path . "/autocomplete.php");
    ?>

    <meta http-equiv="expires" content="0">
    <meta name="robots" content="noindex,nofollow">
    <meta http-equiv="content-type" content="text/html;charset=<?php echo $AC->settings->encoding; ?>">
		<title><?php echo $AC->settings->html_title; ?></title>
		<link rel=stylesheet type="text/css" href="<?php echo $AC->settings->css_file;?>">
		<!--<link rel="search" type="application/opensearchdescription+xml" title="Complete Search for DBLP" href="opensearch.xml">-->

		<!-- Own style sheet for internet explorer versions less than IE 7 -->
		<!--[if lt IE 7]>
		<link rel=stylesheet type="text/css" href="<?php echo str_replace('.css', '_ie5-6.css', $AC->settings->css_file);?>">
    <![endif]-->
		<!-- Own style sheet for internet explorer versions equal to IE 5.5 which only defines what differs from the CSS file above -->
		<!--[if lte IE 5.5]>
		<link rel=stylesheet type="text/css" href="<?php echo str_replace('.css', '_ie55.css', $AC->settings->css_file);?>">
    <![endif]-->

		
	</head>

	<body onload="init();">
  	<?php
	  if (! isset($config->javascript) || $config->javascript)
	  {
	  ?>
		<script type="text/javascript">
		<!--
			document.onmousemove = mouseMove;
			document.onmouseup = mouseUp;
			document.onmousedown = mouseDown;

			// Override standard behaviour of control + cursor left/right
			// Note1: overriding the keydown event of the input field itself don't work
			// Note2: this don't work for Opera
			document.onkeydown = preventCtrlLeftRight;

			// Resize height of hit box and detail box
			window.onresize = resizeHeight;
			window.onscroll = autoScrollMoreHits;

			<?php
  		// NEW 30.04.07 (Markus): Preload all images to prevent uncomplete boxes at beginning
  		if ($AC->state == "start")	{
  		  preloadImages($_SERVER['DOCUMENT_ROOT'] . $AC->settings->index_path . "images", $AC->settings->index_url . "images");
  		}
  		?>
    //-->
		</script>
    <?php
    }
    ?>


		<!-- LAYOUT WITH TWO COLUMNS: queries (left) and hits (right) -->
		<!-- LEFT COLUMN IS FOR QUERIES (input field, completions, refinments, etc.) -->
		<div id="page">

		  <!-- Attention: between the following DIV's no space is allowed -->
		  <div id="detail_bg" style=""></div>
		  <div id="detail" style="display: none"></div>

		  <div id="left_bg" style=""></div>
			<div id="left">
					<div id="logo">
            <!-- NEW 05.08.06 (Markus): graphic for logo changed (was titel.gif) -->
            <a href="http://search.mpi-inf.mpg.de"><img src="images/logo.gif" border="0"/></a>
            <!-- <a href="http://geek2.ag1.mpi-sb.mpg.de"><img src="images/logo.gif" border="0"/></a> -->
					</div>

					<div id="option">
						<!-- Language selection -->
						<!-- (if javascript is activated use a javascript function to change the
						language and provide all parameters to the next side.
						If not use a link to change to the new language; in this case the query string cannot be passed) -->
						<?php
						if ($AC->settings->language == "de") {
						  $tmp = array('en', 'English', 'deutsch');
						}
						else $tmp = array('de', 'deutsch', 'English');

//						echo "\n<a id=\"language1\" href=\"" . $AC->settings->index_page . "?autocomplete_query=" . $AC->query->query_string
						echo "\n<a id=\"language1\" href=\"" . $AC->settings->index_page . "#query=" . $AC->query->query_string
						  . "&language=$tmp[0]\">$tmp[1]</a>&nbsp;<span id=\"language2\">$tmp[2]</span>";

						if (! isset($config->javascript) || $config->javascript)
						{
  						echo "<script type=\"text/javascript\">
  						<!--
  							document.getElementById(\"language1\").href = \"javascript:AC.language_link('$tmp[0]');\";
  						//-->
  						</script>";
						}
						?>

						&nbsp;
						<!-- Link for options page
								Note: strip_tags is used for security reasons to prevent XSS like attacks -->
						<!-- NEW Markus (21.02.07): new parameter added (to be able to create a new session in options.php in case of session expiring -->
						<?php
							// Build link to options page depending on whether cookies are enabled or not
							if ($AC->cookies) {
								$options = $AC->settings->autocomplete_path . "options.php?session_name={$AC->settings->session_name}&path={$AC->settings->index_path}&page={$AC->settings->index_page}&log={$AC->settings->error_log}";
							}
							else {
								$options = url_append($AC->settings->autocomplete_path . "options.php?session_name={$AC->settings->session_name}&path={$AC->settings->index_path}&page={$AC->settings->index_page}&log={$AC->settings->error_log}", strip_tags(SID));
							}
						?>
						<a href='<?php echo $options;?>'>
							<span id='autocomplete_options_title'><?php echo $AC->get_text("options_title");?></span>
						</a>
					</div>

					<div id="reset">
					  	<a id="reset_link" href="<?php echo $AC->settings->autocomplete_path . "/reset.php?session_name=" . $AC->settings->session_name . "&index_url=" . $AC->settings->index_path . $AC->settings->index_page;?>">
					  		reset
					  	</a>
					</div>

					<div id="note">
					<noscript>
						<?php
							if ($AC->note["javascript"] == "1") {
  							echo "<a title=\"" . $AC->get_text("note_hint") . "\" href=\"" . $AC->settings->autocomplete_path . "note.php?session_name={$AC->settings->session_name}&path={$AC->settings->index_path}&page={$AC->settings->index_page}&log={$AC->settings->error_log}&note_topic=javascript&note_title=javascript_note_title&note_body=javascript_note_body" . "\"><img src=\"images/note.gif\" border=\"0\"></a>";
							}
						?>
					</noscript>
          </div>
					<form id="autocomplete_form" action="<?php echo $AC->index_page; ?>" method="GET" onsubmit="<?php echo (! isset($config->javascript) || $config->javascript) ? 'return always_return_false()' : '';?>">
						<!-- This input takes the user query and starts the search process -->
						<!-- NEW 22Jul06 (Holger): taking onkeypress instead of onkeyup (see autocomplete.js) -->
						<!-- NO! Does not work, because right after an onkeypress, the input field is not up to date! -->
						<table class="caption" id="input">
							<tr>
								<td id="input_1_1"></td>
								<td id="input_1_2">
<!--									<input type="text" id="query" name="query" autocomplete="off" value="<?php echo $AC->query->query_string;?>" />-->
									<input type="text" id="autocomplete_query" name="query" autocomplete="off"
						    			value="<?php echo $AC->query->query_string;?>"
						    			onkeyup="<?php echo (! isset($config->javascript) || $config->javascript) ? 'AC.processInput(event)' : '';?>"
						    			onfocus="mozillaWorkaroundForAbsentCursorOnFocus()"
						    			onblur="mozillaWorkaroundForAbsentCursorOnBlur()">
								</td>
								<td id="input_1_3"></td>
							</tr>
							<tr>
								<td id="input_2_1"></td><td id="input_2_2"></td><td id="input_2_3"></td>
							</tr>
						</table>
<!--
						<div style="float: right">
							<table id="submit" style="border-collapse: collapse">
								<tr>
									<td id="history_back" style="cursor: pointer; visibility: hidden;" onclick="AC.history_back();"></td>
									<td id="submit_button_1"></td>
									<td id="submit_button_2"><input class="submit" type="submit" value="<?php echo $AC->get_text('search_button');?>"></td>
									<td id="submit_button_3"></td>
									<td id="history_forward" style="cursor: pointer; display: none" onclick="AC.history_forward()";></td>
								</tr>
							</table>
						</div>
-->
					</form>

					<!-- Subtitle -->
					<div id="autocomplete_H_boxes_1_subtitle" name="autocomplete_H_boxes_1_subtitle"><?php echo $AC->result->H_boxes[1]['subtitle']; ?></div>

					<!-- Error messages -->
<!--
					<table class="caption" id="autocomplete_error_box" style="display:<?php echo ($AC->result->error_message == "") ? 'none' : 'block';?>">
						<tr>
							<td class="box_alert_1_1"></td>
							<td class="box_alert_1_2" id="autocomplete_error_message_title" name="autocomplete_error_message_title"><?php echo $AC->result->error_message_title; ?></td>
							<td class="box_alert_1_3"></td>
						</tr>
						<tr>
							<td class="box_alert_3_1"></td><td class="box_alert_3_2"></td><td class="box_alert_3_3"></td>
						</tr>
						<tr>
							<td class="box_alert_4_1"><img src="images/box_alert_4_1.gif"></td>
							<td class="box_alert_4_2" id="autocomplete_error_message"><?php echo $AC->result->error_message; ?></td>
							<td class="box_alert_4_3"><img src="images/box_alert_4_3.gif"></td>
						</tr>
						<tr>
							<td class="box_alert_5_1"></td><td class="box_alert_5_2"></td><td class="box_alert_5_3"></td>
						</tr>
					</table>
-->
					<!-- W boxes -->
					<?php
					if (isset($AC->result->W_boxes)) {
						foreach ($AC->result->W_boxes as $i => $box)
						{
					?>
					<div id='autocomplete_W_boxes_<?php echo $i;?>' class='box' style='clear: both; display:<?php echo (sizeof($box) > 0 || $AC->settings->show_empty_wbox) ? "block" : "none";?>'>
						<table class='caption'>
							<tr>
								<td class='box_1w_1'></td>
								<td class='box_1w_2'>
									<img id="toggle_W_boxes_<?php echo $i;?>" name="js_box_option" class="toggle_box_image" onclick="toggleBox('W',<?php echo $i;?>)" src="images/arrow_up2.gif">
									<div id='autocomplete_W_boxes_<?php echo $i;?>_title' class='ac_completions_title'>
									   <?php echo (sizeof($box) > 0) ? stripslashes($box['title']) : "";?>
									</div>

									<img id="move_W_boxes_<?php echo $i;?>" name="js_box_option" style="float: right;" class="move_box_image" onclick="minmaximize('W',<?php echo $i;?>)" src="images/box_max.gif">
  							</td>
								<td class="box_1w_3"><img name="progress_image_W" src="images/no_progress_W.gif" class="progress_image" style="display: none"></td>
							</tr>
							<tr>
								<td class="box_3w_1"></td>
								<td class="box_3w_2"></td>
								<td class="box_3w_3"></td>
							</tr>
						</table>
						<div id='autocomplete_W_boxes_<?php echo $i;?>_body' name='box_W' class='ac_completions'><?php echo (sizeof($box) > 0) ? stripslashes($box['body']) : "";?></div>
					</div>
					<?php
						}
					}
					?>

					<!-- C boxes -->
					<?php
					if (isset($AC->result->C_boxes)) {
						foreach ($AC->result->C_boxes as $i => $box)
						{
					?>
					<div id='autocomplete_C_boxes_<?php echo $i;?>' class='box' style='display:<?php echo (sizeof($box) > 0) ? "block" : "none";?>'>
						<table class='caption'>
							<tr>
								<td class='box_1w_1'></td>
								<td class='box_1w_2'>
									<img id="toggle_C_boxes_<?php echo $i;?>" name="js_box_option" class="toggle_box_image" onclick="toggleBox('C',<?php echo $i;?>)" src="images/arrow_up2.gif">
									<div id='autocomplete_C_boxes_<?php echo $i;?>_title' class='ac_completions_title'>
				            <?php echo (sizeof($box) > 0) ? stripslashes($box['title']) : "";?>
									</div>
									<img id="move_C_boxes_<?php echo $i;?>" name="js_box_option" style="float: right;" class="move_box_image" onclick="minmaximize('C',<?php echo $i;?>)" src="images/box_max.gif">
								</td>
								<td class="box_1w_3"><img name="progress_image_W" src="images/no_progress_W.gif" class="progress_image" style="display: none"></td>
							</tr>
							<tr>
								<td class="box_3w_1"></td>
								<td class="box_3w_2"></td>
								<td class="box_3w_3"></td>
							</tr>
						</table>
						<div id='autocomplete_C_boxes_<?php echo $i;?>_body' name='box_C' class='ac_completions'><?php echo (sizeof($box) > 0) ? stripslashes($box['body']) : "";?></div>
					</div>
					<?php
						}
					}
					?>


					<!-- F boxes 1..n -->
					<?php
					if (isset($AC->result->F_boxes)) {
						foreach ($AC->result->F_boxes as $i => $box)
						{
					?>
					<div id='autocomplete_F_boxes_<?php echo $i;?>' class='box' style='display:<?php echo (sizeof($box) > 0) ? "block" : "none";?>'>
						<table class='caption'>
							<tr>
								<td class='box_1f_1'></td>
								<td class='box_1f_2'>
									<img id="toggle_F_boxes_<?php echo $i;?>" name="js_box_option" class="toggle_box_image" onclick="toggleBox('F',<?php echo $i;?>)" src="images/arrow_up2.gif">
									<div id='autocomplete_F_boxes_<?php echo $i;?>_title' class='ac_completions_title'>
									   <?php echo (sizeof($box) > 0) ? stripslashes($box['title']) : "";?>
									</div>
									<img id="move_F_boxes_<?php echo $i;?>" name="js_box_option" style="float: right;" class="move_box_image" onclick="minmaximize('F',<?php echo $i;?>)" src="images/box_max.gif">
								</td>
								<td class='box_1f_3'><img name="progress_image_F" src="images/no_progress_F.gif" class="progress_image" style="display: none"></td>
							</tr>
							<tr>
								<td class='box_3f_1'></td>
								<td class='box_3f_2'></td>
								<td class='box_3f_3'></td>
							</tr>
						</table>
						<div id='autocomplete_F_boxes_<?php echo $i;?>_body' name='box_F' class='ac_completions'><?php echo (sizeof($box) > 0) ? stripslashes($box['body']) : "";?></div>
					</div>
					<?php
						}
					}
					?>


					<!-- Yago Y-Boxes 1..n -->
					<?php
					if (isset($AC->result->Y_boxes)) {
						foreach ($AC->result->Y_boxes as $i => $box)
						{
					?>
					<div id='autocomplete_Y_boxes_<?php echo $i;?>' class='box' style='display:<?php echo (sizeof($box) > 0) ? "block" : "none";?>'>
						<table class='caption'>
							<tr>
								<td class='box_1f_1'></td>
								<td class='box_1f_2'>
									<img id="toggle_Y_boxes_<?php echo $i;?>" name="js_box_option" class="toggle_box_image" onclick="toggleBox('Y',<?php echo $i;?>)" src="images/arrow_up2.gif">
									<div id='autocomplete_Y_boxes_<?php echo $i;?>_title' class='ac_completions_title'>
									   <?php echo (sizeof($box) > 0) ? stripslashes($box['title']) : "";?>
									</div>
									<img id="move_Y_boxes_<?php echo $i;?>" name="js_box_option" style="float: right;" class="move_box_image" onclick="minmaximize('Y',<?php echo $i;?>)" src="images/box_max.gif">
								</td>
								<td class='box_1f_3'><img name="progress_image_Y" src="images/no_progress_Y.gif" class="progress_image" style="display: none"></td>
							</tr>
							<tr>
								<td class='box_3f_1'></td>
								<td class='box_3f_2'></td>
								<td class='box_3f_3'></td>
							</tr>
						</table>
						<div id='autocomplete_Y_boxes_<?php echo $i;?>_body' name='box_Y' class='ac_completions'><?php echo (sizeof($box) > 0) ? stripslashes($box['body']) : "";?></div>
					</div>
					<?php
						}
					}
					?>


					<!-- Yago R-Boxes 1..n -->
					<?php
					if (isset($AC->result->R_boxes)) {
						foreach ($AC->result->R_boxes as $i => $box)
						{
					?>
					<div id='autocomplete_R_boxes_<?php echo $i;?>' class='box' style='display:<?php echo (sizeof($box) > 0) ? "block" : "none";?>'>
						<table class='caption'>
							<tr>
								<td class='box_1f_1'></td>
								<td class='box_1f_2'>
									<img id="toggle_R_boxes_<?php echo $i;?>" name="js_box_option" class="toggle_box_image" onclick="toggleBox('R',<?php echo $i;?>)" src="images/arrow_up2.gif">
									<div id='autocomplete_R_boxes_<?php echo $i;?>_title' class='ac_completions_title'>
									   <?php echo (sizeof($box) > 0) ? stripslashes($box['title']) : "";?>
									</div>
									<img id="move_R_boxes_<?php echo $i;?>" name="js_box_option" style="float: right;" class="move_box_image" onclick="minmaximize('R',<?php echo $i;?>)" src="images/box_max.gif">
								</td>
								<td class='box_1f_3'><img name="progress_image_R" src="images/no_progress_R.gif" class="progress_image" style="display: none"></td>
							</tr>
							<tr>
								<td class='box_3f_1'></td>
								<td class='box_3f_2'></td>
								<td class='box_3f_3'></td>
							</tr>
						</table>
						<div id='autocomplete_R_boxes_<?php echo $i;?>_body' name='box_R' class='ac_completions'><?php echo (sizeof($box) > 0) ? stripslashes($box['body']) : "";?></div>
					</div>
					<?php
						}
					}
					?>


					<?php
					// NEW 08-01-09 / Markus: check now against Javascript enabled or not
					if ($AC->log->level >= $AC->log->levels['INFO'] && $AC->javascript !== false)
					{
					?>
					<!-- Area for notifications -->
					<div id="autocomplete_info_boxes_1" class='box'>
						<table class='caption'>
							<tr>
								<td class='box_1w_1'></td>
								<td class='box_1w_2'>
									<img id="toggle_info_boxes_1" name="js_box_option" class="toggle_box_image" onclick="toggleBox('info',1)" src="images/arrow_up2.gif">
									<div id='autocomplete_info_boxes_1_title' class='ac_completions_title'>Info:</div>
									<span id="clear_infobox" name="js_box_option" onclick="clearInfo()">[x]</span>
									<img id="move_info_boxes_1" name="js_box_option" style="float: right;" class="move_box_image" onclick="minmaximize('info',1)" src="images/box_max.gif">
								</td>
								<td class='box_1w_3'><img name="progress_image_W" src="images/no_progress_W.gif" class="progress_image" style="display: none"></td>
							</tr>
							<tr>
								<td class='box_3w_1'></td>
								<td class='box_3w_2'></td>
								<td class='box_3w_3'></td>
							</tr>
						</table>
						<div id='autocomplete_info_boxes_1_body' name='box_info' class='ac_completions'></div>
					</div>

          <?php
					}
					?>
				</div>	<!-- id="left", END OF LEFT COLUMN -->

				<!-- RIGHT COLUMN IS FOR HITS -->
				<div id="right">
		   		<div id="autocomplete_H_boxes_1_title"><?php echo $AC->result->H_boxes[1]['title']; ?></div>
					<!-- Hits -->
					<div id="autocomplete_H_boxes_1_body" name='box_H'>
<!--					<div id="autocomplete_hit_box_body">-->
					<?php
						// NEW 18.01.07 (Markus): intro
						if ($AC->state == "start")
						{
							echo sprintf($AC->get_text("hit_box_at_beginning"), $AC->documents_count, $AC->settings->service_name, $AC->settings->server_port);
						}
						else if ($AC->result->H_boxes[1]['body'] == "")
						{
							echo "<p style='font-style: italic; font-size: 16pt; color: rgb(200,200,200); margin: 24px;'>" . sprintf($AC->get_text("hit_box_empty_query"), $AC->documents_count) . "</p>";
						}
						else echo $AC->result->H_boxes[1]['body'];
					?>
					</div>
				</div>	<!-- id="right", END OF RIGHT COLUMN -->

		</div>	<!-- PAGE -->


		<?php
	  if (! isset($config->javascript) || $config->javascript)
	  {
	  ?>
    <!-- JAVASCRIPT FOR INITIALIZATION (focus, etc.) -->
		<script type="text/javascript">
		<!--

			// Show the toggle display buttons (set display to 'block')
			nodes = document.getElementsByName ("js_box_option");
			if (nodes) {
				for (var i=0; i<nodes.length; i++)
				{
					nodes[i].style.display = "block";
				}
			}

/*
			// If javascript is enabled replace all href's of navigation links by javascript calls
			var nodes = document.getElementsByName("navigation_link");
			for (var i=0; i < nodes.length; i++)
			{
				result = nodes[i].href.match (/fh=(.*)/);
				if (result) {
					nodes[i].href = "javascript:AC.navigation_link(" + result[1] + ");";
				}
			}

			// If javascript is enabled replace all href's of more/less links by javascript calls
			var nodes = document.getElementsByName("more_link");
			for (var i=0; i < nodes.length; i++)
			{
				result = nodes[i].href.match (/qt=(.*)&mcs=(.*)/);
				if (result) {
					nodes[i].href = "javascript:AC.more_link('" + result[1] + "'," + result[2] + ");";
				}
			}
*/
      var left = document.getElementById("left");
      var right = document.getElementById("right");
      var detail = document.getElementById("detail");

      // Correct the padding-left of the right box
//      right.style.paddingLeft = (left.offsetWidth + getStyle(right, 'paddingLeft')) + "px";

      // Set the height of the detail box to full page height
      detail.style.height = (getWindowHeight() - 10) + "px";

      // Set the width of the detail box to the width of left box
      detail.style.width = left.offsetWidth + "px";

    //-->
		</script>
    <!-- END OF JAVASCRIPT FOR INITIALIZATION -->
    <?php
	  }
    ?>

		<!--[if lt IE 8]>
    <iframe id="historyFrame" src="autocomplete/ie_history_frame.html" style="visibility:none; height:0px; width:0px"><html></html></iframe>
    <![endif]-->

	</body>
</html>
