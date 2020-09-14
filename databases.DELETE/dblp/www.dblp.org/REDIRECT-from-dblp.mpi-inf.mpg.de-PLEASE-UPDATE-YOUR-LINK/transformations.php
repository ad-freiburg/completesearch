<?php

function transform_hit_type(&$type, $hit)
{
  switch($type)
  {
    case "inproceedings": $type = "#ccccff"; break;
    case "article": $type = "#ffcccc"; break;
    case "book": $type = "#ccffff"; break;
    case "proceedings": $type = "#ccffff"; break;
    default: $type = "#ccccff";
  }

  if (preg_match("|journals/corr/|", $hit->title['venue_url'])) {
    $type = "#cccccc";
  }
}


function transform_hit_title_ee(&$object, $hit)
{
  if ($object == "") {
    $object = "<td>&nbsp;&nbsp;&nbsp;</td>";
  }
  else {
    $object = "<td bgcolor=\"#ccffcc\" valign=\"top\"><a href=\"$object\">EE</a></td>";
  }
}


function transform_hit_author(&$object, $hit)
{
  // Special treatment for suffixes like 0002
  $object = preg_replace("| (\d\d\d\d)$|", "_$1", $object);
  
  // Link authors back to Michaels site
  $link = htmlentities(utf8_decode($object));
  $pos = strrpos($link, " ");
  
  if ($pos != false) $link = substr($link, $pos + 1) . ":" . substr($link, 0, $pos);
  $link = preg_replace("| |", "_", $link);
  $link = preg_replace("|[^\w:]|", "=", $link);
  $base = "http://www.dblp.org/db/indices/a-tree/";
  $link = $base . strtolower(substr($link, 0, 1)) . "/" . $link . ".html";
  
  // Remove suffixes like 0002 from the author name
  $object = preg_replace("|_\d\d\d\d$|", "", $object);
  $object = "<a href=\"$link\">$object</a>";
}

?>
