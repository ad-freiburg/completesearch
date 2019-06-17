<?php
  // NEW 02Nov07 (Holger): only do something for certain IP addresses;
  //
  //   TODO: once Michael has inserted the javascript include into every static
  //   page, ask Guido to remove his filter and rewrite (but keep the
  //   /~ley/dblp.mpi-inf.mpg.de/ rewrite to cope with the cross-scripting)
  //if (!isset($_SERVER["HTTP_X_FORWARDED_FOR"])) return;
  //if (!preg_match("/^139\.19\./", $_SERVER["HTTP_X_FORWARDED_FOR"])) return;
  //if (!preg_match("/^139\.19\.(10|14|64)\./", $_SERVER["HTTP_X_FORWARDED_FOR"])) return;
  error_reporting(E_ALL);
  require("facetboxes.autocomplete_config.php");
  require($_SERVER['DOCUMENT_ROOT'] . $config->autocomplete_path . "/autocomplete.php");
?>

<!--
<style>.hits_number { padding-left: 0.3em; }</style>
<style>.completion { float: left; clear: both; }</style>
<style>.more_link { float: left; clear: both; }</style>
<style>.hits_number { float: right; }</style>
<link rel=stylesheet type="text/css" href="autocomplete.css">
-->

<!-- Facet Boxes 1..n -->
<?php
  // Markus / 11-07-09: Was before (what was not sufficient):
  // Note: Only use session_unset() for older deprecated code that does not use $_SESSION
  // session_unset();

  // Unset all of the session variables.
  $_SESSION = array();

  // If it's desired to kill the session, also delete the session cookie.
  // Note: This will destroy the session, and not just the session data!
  if (isset($_COOKIE[session_name()])) {
      setcookie(session_name(), '', time() - 42000, '/');
  }

  // Finally, destroy the session.
  if ($_SESSION)  session_destroy();

  // NEW 06Sep12 (baumgari): Added index checking for 'body' and 'title' 
  // (array_key_exists), since sometimes some notices were thrown and written 
  // into the dblp error log. This happens in case of empty facetboxes, which as 
  // far as I could notice, just occur in case of wrong encoded special 
  // characters. I don't know the reason for that, but it doesn't influence the 
  // result (at some point it's converted to the right encoding and the 
  // facetboxes get filled correctly (and automatically)).
  // This php file is called in facetboxes.js every time somebody calls 
  // a static dblp site. 
  // NEW 26Sep12 (baumgari): Added a more significant log line in case of empty 
  // facetboxes. I tried to use the log class 
  // (autocomplete-php/autocomplete/log.php) by using $AC->log... or a self 
  // instantiated log, but I didn't get it to work, so I 
  // just used trigger_error instead, which actually does the same.
  $path_info = pathinfo(__FILE__);
  define("SCRIPT_FACETBOXES", $path_info["basename"]);
  foreach ($AC->result->F_boxes as $i => $box)
  {
    if (!(array_key_exists('body', $box) && array_key_exists('title', $box)))
      trigger_error("in line " . __LINE__ . " of " . SCRIPT_FACETBOXES 
        . ": Empty facetboxes, probably caused by wrong encoded special characters: " . $AC->query->query_string, E_USER_WARNING);
    if (array_key_exists('body', $box))
      $box['body'] = preg_replace("|facetboxes.php|", "index.php", $box['body']);
?>
  <div id='autocomplete_F_boxes_<?php echo $i;?>' style='padding-bottom:6px;display:<?php echo (sizeof($box) > 0) ? "block" : "none";?>'>
    <table border="1" width="100%">
      <tr><td bgcolor="#ffffcc">
        <img name="progress_image_F" src="images/no_progress_F.gif" style="display:none;float:right;margin-top:2px;">
        <div id='autocomplete_F_boxes_<?php echo $i;?>_title'>
          <?php echo (sizeof($box) > 0 && array_key_exists('title', $box)) ? stripslashes($box['title']) : "";?>
        </div>
      </td></tr>

      <tr><td>
        <div id='autocomplete_F_boxes_<?php echo $i;?>_body'>
          <?php echo (sizeof($box) > 0 && array_key_exists('body', $box)) ? stripslashes($box['body']) : "";?>
        </div>
      </td></tr>
    </table>
  </div>
<?php
  }
?>

<div id="hide" style="float:right">
  <small><a href="javascript:hideFacetBoxes()" style="color:#999999">hide facet boxes</a></small>
</div>

