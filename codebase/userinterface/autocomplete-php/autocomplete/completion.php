<?php

/**
 * Class for representing a completion object
 *
 */
class Completion
{
  var $id;
  var $string;

  // Beachte dass $occurrence_count >= $document_count, aber in der Regel größer, weil ein Dokument ein 
  //  Wort mehr als einmal enthalten kann, und $occurrence_count zählt jedes einzelne Vorkommen
  var $document_count;    // Anzahl der Treffer, in denen dieses Wort vorkommt
  var $occurrence_count;  // Gesamtanzahl der Vorkommen dieses Wortes in der Treffermenge
  var $score;
}