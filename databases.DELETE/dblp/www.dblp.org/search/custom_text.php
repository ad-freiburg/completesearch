<?php

// Custom text blocks, overwriting those in autocomplete-php/text.php
 
$welcome_message_en = 
"<div style=\"display:block;text-align:left\">".
"  <h2>Features</h2>".
"  <ul>".
"    <li><b>fast search as you type:</b> a search is triggered after each keystroke, with instant response times; if not, your network connection is lame ;-)</li>".
"    <li><b>prefix search:</b> e.g., <a href=\"index.php?query=sig\">sig</a> matches SIGIR (the search is always case-insensitive, too); no prefix search for single letters!</li>".
"    <li><b>error-tolerant search:</b> append a tilda, e.g., <a href=\"index.php?query=probabi~\">probabi~</a> will also match \"probalistic\"</li>".
"    <li><b>faceted search:</b> at any time, the right sidebar gives a breakdown of the current result set (initially: all entries) by various categories; click an item to refine</li>".
"    <li><b>exact word match:</b> end word with a dollar, e.g.  <a href=\"index.php?query=graph$\">graph$</a></li>".
"    <li><b>previous/next query:</b> just use the browser back/forward buttons; typically Alt-left/right work for this, too!</li>".
"    <li><b>boolean or:</b> put a pipe between words, e.g., <a href=\"index.php?query=graph|network\">graph|network</a></li>".
 "    <li><b>boolean not:</b> put a minus before a word, e.g., <a href=\"index.php?query=-venue:corr kernelization\">-venue:corr kernelization</a></li>".
"    <li><b>phrase search:</b> put a dot between words, e.g., <a href=\"index.php?query=information.retrieval\">information.retrieval</a></li>".
"    <li><b>specify number of authors:</b> add \"na:1\" to your query to restrict your search to single-authored papers, or \"na:2\" etc.</li>".
"    <li><b>only first-authored papers:</b> prepend \"first\" to the author token, as in <a href=\"index.php?query=firstauthor:michael_stonebraker\">firstauthor:michael_stonebraker</a></li>".
// "    <li><b>advanced interface:</b> the default interface is modelled after the plain and functional DBLP design, for a richer GUI with more functionality, click <a href=\"http://dblp.mpi-inf.mpg.de/new/index.php?query=\">here</a></li>".
"  </ul>".
"  <ul>".
"    <li><b>publications:</b> click <a href=\"/search/index.php?query=author:hannah_bast autoc|compl|ester\">here</a>".
"      for publications on the underlying system, data structures, algorithms.</br>".
"    <li><b>people currently involved:</b> Hannah Bast, Marjan Celikik, Ina Baumgarten.</br>".
"    <li><b>people involved in the past:</b> Ingmar Weber, Daniel Fischer, Markus Tetzlaff.</br>".
"  </ul>".
"</div>";

# Expert tip from 18Aug11.
$expert_tip_en =
"  <table border=\"1\" width=\"100%\">" .
"      <tr>" .
"        <td bgcolor=\"#ffffcc\" style=\"font-weight:bold;padding:3px\">" .
"        NOTE: The DBLP search has moved</td>" .
"      </tr>" .
"      <tr>" .
"        <td style=\"padding:3px;text-align:left\"><small>CompleteSearch DBLP" .
"        has moved to the new domain <b>www.dblp.org</b>. Please update any links or " .
"        bookmarks you might have set accordingly. The pages under www. informatik.uni-trier.de " .
"        will eventually be moved there, too. There will be a separate " .
"        notification about that.</small></td>" .
"      </tr>" .
"    </table>";

# Expert tip from 20Sep10.
# $expert_tip_en =
# "  <table border=\"1\" width=\"100%\">" .
# "      <tr>" .
# "        <td bgcolor=\"#ffffcc\" style=\"font-weight:bold;padding:3px\">" .
# "        EXPERT TIP: Refine by publication type</td>" .
# "      </tr>" .
# "      <tr>" .
# "        <td style=\"padding:3px;text-align:left\"><small>To get a breakdown of" .
# "        your result set by publication type (conference, journal, editor, book," .
# "        other), just append <i>type:</i> to your query. Then click on anyone" .
# "        type to refine. Thanks to Kim Skak Larsen for reminding me of this" .
# "        feature.</small></td>" .
# "      </tr>" .
# "    </table>";

$AC->text["expert_tip"] = array
(
  "en" => $expert_tip_en
);

$AC->text["hit_box_at_beginning"] = array
(
  "en" => $welcome_message_en
);

$AC->text["hit_box_empty_query"] = array
(
  "en" => $welcome_message_en
);

?>
