<?php
  // The following is very important to force browsers to load this page from server,
  //  especially after a back from external pages
  //header( "Last-Modified: " . gmdate( "D, j M Y H:i:s" ) . " GMT" );
  //header( "Expires: " . gmdate( "D, j M Y H:i:s", time() ) . " GMT" );
  header("Cache-Control: no-store, no-cache, must-revalidate");
  // HTTP/1.1 Header( "Cache-Control: post-check=0, pre-check=0", FALSE );
  header("Pragma: no-cache");
  ob_start();
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<?php
//    error_reporting(E_NOTICE);
//    error_reporting(E_STRICT);
  error_reporting(E_ALL);
//    error_reporting(E_ERROR);

  /* USER CONFIG FILE, ESSENTIAL! */
  require("autocomplete_config.php");
  // The following line causes invalid HTML code (because of the DOCTYPE which is not at the beginning).
  // Do the DOCTYPE at the beginning results in the effect that whwn using the Firefox the autocomplete_query input-Tag is empty when we came from a external link or from the options page
  // Internet Explorer and Opera work fine
  require($_SERVER['DOCUMENT_ROOT'] . $config->autocomplete_path . "/autocomplete.php");
?>
<html>
<head>
  <meta name="robots" content="nofollow">
  <meta http-equiv="content-type" content="text/html;charset=<?php echo $AC->settings->encoding; ?>">
  <!-- NEW 19Dec07 (Holger): always show vertical scrollbar in Firefox (in IE that's the default anyway) -->
  <link rel=stylesheet type="text/css" href="autocomplete.css">
  <title><?php echo $AC->settings->html_title; ?></title>
</head>
<body bgcolor="#ffffff" text="#000000" link="#000000" onload="init();">
  <script type="text/javascript">
  <!--
    // Inform server that javascript is enabled
    // AC.JSNotification();
    <?php
      // NEW 30.04.07 (Markus): Preload all images to prevent incomplete boxes at beginning
      if ($AC->state == "start") {
        echo "\n\t\t\tif (document.images) {";
        $image_dir = $_SERVER['DOCUMENT_ROOT'] . $AC->settings->index_path . "images";
        $image_url = $AC->settings->index_url . "images";
        $dh = opendir($image_dir);
        if ($dh !== false) {
          $i = 1;
          while (false != ($file = readdir($dh))) {
             if (is_file("$image_dir/$file") == 1) {
              if (strpos ($file, ".jpg") > 0 || strpos ($file, ".gif") > 0) {
                // Größe des Bildes bestimmen
                list($width, $height, $typ, $att) = GetImageSize ("$image_dir/$file");
                echo "\n\t\t\t\tpicture$i = new Image($width,$height);";
                echo "picture$i.src = '$image_url/$file';";
                $i++;
              }
            }
          }
        }
        echo "\n\t\t\t}";
      }
    ?>
  //-->
  </script>

  <div id="tip" style="width:20em;float:right;">
  <?php echo $AC->get_text("expert_tip");?>
  </div>

  <!-- <a style="float:left" href="<?=$config->dblp_url;?>"><img alt="dblp.uni-trier.de" src="logo.png" border="0" height="60" width="170"/></a> -->
  <a style="float:left" href="<?=$config->dblp_url;?>"><img alt="dblp.uni-trier.de" src="images/Logo.gif" border="0" height="60" width="170"/></a>

  <!--
  <div style="float:left; background:lightgrey;text-align:center;padding:0.4em;margin-bottom:1em;font-size:small;">
  This is a DBLP mirror with extended capabilities. The original site is located at 
  <a href="http://www.informatik.uni-trier.de/~ley/db">http://www.informatik.uni-trier.de/~ley/db</a>.
  For more information on this mirror click on the Reset &amp; Info link below.</div> -->
  <div style="clear:left"></div>
  <h1><?=$AC->settings->html_title;?></h1>
  <!-- List of publications from the
  <a href="<?=$config->dblp_url;?>">DBLP Bibliography Server</a> -
  <a href="<?=$config->dblp_url;?>about/faq.html">FAQ</a> -->
  <div style="float:right">
    <!-- email comments -->
    <a href="mailto:Hannah Bast <bast@informatik.uni-freiburg.de>?subject=DBLP feedback">Feedback</a>
    <!-- Switch to CompleteSearch layout -->
    <!--
    &nbsp;&nbsp;&nbsp;
    <a href="/search/relay.php?autocomplete_query=<?php echo $AC->query->query_string?>">Advanced GUI</a>
    -->
    <!-- Help & Reset -->
    &nbsp;&nbsp;&nbsp;
    <a id="reset_link" href="<?php echo $AC->settings->autocomplete_path .
      "/reset.php?session_name=" . $AC->settings->session_name . "&index_url=" .
      $AC->settings->index_path . $AC->settings->index_page;?>">Help</a>
    <!-- Link for options page -->
    <!--
    &nbsp;&nbsp;&nbsp;
    -->
    <?php
      // // Build link to options page depending on whether cookies are enabled or not
      // if ($AC->cookies) {
      //   $options = $AC->settings->autocomplete_path . "options.php?session_name={$AC->settings->session_name}&path={$AC->settings->index_path}&page={$AC->settings->index_page}&log={$AC->settings->error_log}";
      // } else {
      //   $options = url_append($AC->settings->autocomplete_path . "options.php?session_name={$AC->settings->session_name}&path={$AC->settings->index_path}&page={$AC->settings->index_page}&log={$AC->settings->error_log}", strip_tags(SID));
      // }
    ?>
    <!--
    <a href='<?php echo $options;?>' id="autocomplete_options_title"><?php echo $AC->get_text("options_title");?></a>
    -->
  </div>
  a DBLP mirror with extended search capabilities maintained by 
  <a href="http://ad.informatik.uni-freiburg.de/staff/bast">Hannah Bast</a>, 
  <a href="http://ad.informatik.uni-freiburg.de">University of Freiburg</a>
  (formerly <a href="http://www.mpi-inf.mpg.de/~bast">MPII Saarbr&uuml;cken</a>)
  <!-- <a href="http://www.mpi-inf.mpg.de/departments/d1/areas/ir.html">AG1-IR</a> -->
  <!-- ; the original site is located at <a href="http://www.informatik.uni-trier.de/~ley/db">http://www.informatik.uni-trier.de/~ley/db</a>. -->

  <noscript style="font-weight: bold;">
    <hr/><?php echo $AC->get_text("javascript_note"); ?>
  </noscript>
<hr/>
  <table width="100%"><td>
  <!-- Search field -->
  <div style="float:right">
      <form id="autocomplete_form" action="index.php" method="GET" onsubmit="return false;" style="margin:0;display:inline;">
        <!-- This input takes the user query and starts the search process -->
        <input type="text" id="autocomplete_query" name="query" autocomplete="off" size="40" style="font-size:100%;margin-left:20px;"
          value="<?php echo $AC->query->query_string;?>" onkeyup="javascript:AC.processInput(event);"/>
        <!-- <input type="submit" id="submit" value="<?php echo $AC->get_text('search_button');?>"/> -->
      </form>
  </div>
  <!-- Subtitle -->
  <div style="margin-top:0.2em">
    <span id="autocomplete_H_boxes_1_subtitle"><?php echo $AC->result->H_boxes[1]['subtitle']; ?></span>
  </div>
  </td></table>
<hr/>
  <div id="sidebar" style="width:20em;margin-left:20px;float:right;">

    <!-- Completion Box 1..n -->
    <?php
      if (isset($AC->result->W_boxes))
      foreach ($AC->result->W_boxes as $i => $box)
      {
    ?>
      <div id='autocomplete_W_boxes_<?php echo $i;?>' style='padding-bottom:6px;display:<?php echo (sizeof($box) > 0) ? "block" : "none";?>'>
        <table border="1" width="100%">
          <tr><td bgcolor="#ffffcc">
            <img name="progress_image_W" src="images/no_progress_W.gif" style="display:none;float:right;margin-top:2px;">
            <div id='autocomplete_W_boxes_<?php echo $i;?>_title'>
              <?php echo (sizeof($box) > 0) ? stripslashes($box['title']) : "";?>
            </div>
          </td></tr>

          <tr><td>
            <div id='autocomplete_W_boxes_<?php echo $i;?>_body'>
              <?php echo (sizeof($box) > 0) ? stripslashes($box['body']) : "";?>
            </div>
          </td></tr>
        </table>
      </div>
    <?php
      }
    ?>

    <!-- Facet Boxes 1..n -->
    <?php
      foreach ($AC->result->F_boxes as $i => $box)
      {
    ?>
      <div id='autocomplete_F_boxes_<?php echo $i;?>' style='padding-bottom:6px;display:<?php echo (sizeof($box) > 0) ? "block" : "none";?>'>
        <table border="1" width="100%">
          <tr><td bgcolor="#ffffcc">
            <img name="progress_image_F" src="images/no_progress_F.gif" style="display:none;float:right;margin-top:2px;">
            <div id='autocomplete_F_boxes_<?php echo $i;?>_title'>
              <?php echo (sizeof($box) > 0) ? stripslashes($box['title']) : "";?>
            </div>
          </td></tr>

          <tr><td>
            <div id='autocomplete_F_boxes_<?php echo $i;?>_body'>
              <?php echo (sizeof($box) > 0) ? stripslashes($box['body']) : "";?>
            </div>
          </td></tr>
        </table>
      </div>
    <?php
      }
    ?>

    <!-- Holger's connection speed measurement (experimental) -->
    <!-- <div id="connection_speed" style="font-size: 80%; color:#eeeeee; float:right"></div> -->

    <!-- If this exists, clients sends us time spent in his/her javascript -->
    <div id="client_measurement"></div>

    <!-- Area for notifications -->
    <table border="1" width="100%" id="autocomplete_notes_box" style="display:none">
      <tr><td bgcolor="#ffffcc" id="autocomplete_notes_title">Info:</td></tr>
      <tr><td id="autocomplete_notes"></td></tr>
    </table>
  </div>

  <!-- <div id="maincontent" style="overflow:hidden;"> -->
  <div id="right">
    <!-- Hits -->
    <div id="autocomplete_H_boxes_1_body">
    <?php echo $AC->result->H_boxes[1]['body']; ?>
    <?php
      //// NEW 18.01.07 (Markus): intro
      //if ($AC->state == "start" || $AC->query->query_string == "")
      //{
      //  echo file_get_contents("info.html");
      //  //echo sprintf($AC->get_text("hit_box_at_beginning"), $AC->documents_count, $AC->settings->service_name, $AC->settings->server_port);
      //} 
      //else if ($AC->result->hit_box == "") 
      //{
      //  echo file_get_contents("info.html");
      //  //echo "<p style='font-style: italic; font-size: 16pt; color: rgb(200,200,200); margin: 24px;'>" . sprintf($AC->get_text("hit_box_empty_query"), $AC->documents_count) . "</p>";
      //} else 
      //{
      //  //echo file_get_contents("info.html");
      //  echo $AC->result->hit_box['body'];
      //}
    ?>
    </div>
  </div>

  <div id="autocomplete_H_boxes_1_title" style="text-align:center;margin-right:320px;margin-top:6px;">
    <?php echo $AC->result->H_boxes[1]['title']; ?>
  </div>

  <!-- JAVASCRIPT FOR INITIALIZATION (focus, etc.) -->
  <script type="text/javascript">
  <!--
    // Set the cursor at the end of the input field
    // and remove the "?", "!" from the query string.
    // The order is important here so that the cursor gets positioned at the end
      //setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
      //document.getElementById('autocomplete_query').focus();
      //document.getElementById('autocomplete_query').value = document.getElementById('autocomplete_query').value.replace(/[\!\?]$/,'');

    node = document.getElementById("submit");
    if (node != null) {
      node.style.display = "none";
    }

    //if (AC.show_progress_images) {
    //  var qt = AC.query_types;
    //  for (var j=0; j < qt.length; j++) {
    //    nodes = document.getElementsByName ("progress_image_" + qt.charAt(j));
    //    if (nodes != undefined) {
    //      for (var i=0; i<nodes.length; i++) {
    //        nodes[i].style.display = "block";
    //      }
    //    }
    //  }
    //}

    // AC.refreshNotes();
  //-->
  </script>
  <!--[if IE]>
    <iframe id="historyFrame" src="<?php echo $AC->settings->autocomplete_path; ?>/ie_history_frame.html" style="visibility:none; height:0px; width:0px">
      <html></html></iframe>
  <![endif]-->
</body>
</html>
