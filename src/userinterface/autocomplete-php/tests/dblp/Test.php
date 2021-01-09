<?php

error_reporting(E_ALL);

include "../autocomplete/log.php";

//$path = 'C:/Dokumente und Einstellungen/www/PEAR/';
//$path = 'C:/Program Files/Entwicklung/PHP5/PEAR/';
//set_include_path(get_include_path() . PATH_SEPARATOR . $path);
$path = 'C:/Program Files/Entwicklung/SeleniumRC/php/PEAR/';
set_include_path(get_include_path() . PATH_SEPARATOR . $path);

require_once 'Testing/Selenium.php';
require_once 'PHPUnit/Framework.php';
require_once 'PHPUnit/Framework/TestCase.php';



class Test extends PHPUnit_Framework_TestCase
{
  public $testMapping = array(
    "testMoreLess" => "more/less completions",
    "testSummaryLog" => "summary line in log file",
    "testCompletionNavigation" => "navigating completions using key commands cursor down/up",
    "testHitsNavigation" => "navigating hits using page down/up links, keys and more link",
    "testHistory" => "back/forward buttons",
    "testBasicSearch" => "basic search interaction",
    "testMoreLessOfSingleBoxes" => "more/less links of a single facet box");

  // Used by the preg_match calls of some functions
  protected $matches;
  // Used for logging function
  private $fp;

  const regExTrueFailurePattern = "regular expression \"%s\" don't match \"%s\"";
  const regExFalseFailurePattern = "regular expression \"%s\" match \"%s\" (must not match)";
  const trueFailurePattern = "\"%s\" is not true";
  const falseFailurePattern = "\"%s\" is not false";
  const equalsFailurePattern = "\"%s\" don't equals \"%s\"";


  protected  function myAssertTrueRegEx($regEx, $pattern)
  {
    $this->assertTrue((bool) preg_match($regEx, $pattern),
        sprintf(self::regExTrueFailurePattern, $regEx, $pattern) . " " . $this->sharedFixture['browser']);
  }


  protected  function myAssertFalseRegEx($regEx, $pattern)
  {
    $this->assertFalse((bool) preg_match($regEx, $pattern),
        sprintf(self::regExFalseFailurePattern, $regEx, $pattern));
  }


  protected  function myAssertTrue($value)
  {
    $this->assertTrue($value, sprintf(self::trueFailurePattern, $value));
  }


  protected  function myAssertFalse($value)
  {
    $this->assertFalse($value, sprintf(self::falseFailurePattern, $value));
  }


  protected  function myAssertEquals($value1, $value2)
  {
    $this->assertEquals($value1, $value2,
        sprintf(self::equalsFailurePattern, $value1, $value2));
  }


  protected function write($text)
  {
    fwrite($this->fp, "\n" . $text);
  }


  function setUp()
  {
    if (! file_exists("log")) {
      mkdir("log");
    }
    $this->fp = fopen('log/debug.txt', "a+");

    $this->matches = array();
    $this->verificationErrors = array();
    $this->selenium = new Testing_Selenium($this->sharedFixture['browser'], $this->sharedFixture['url']);
    $result = $this->selenium->start();
    $this->write('Selenium is starting ...');
  }


  function tearDown()
  {
    $this->selenium->stop();

    if(count($this->verificationErrors) > 0 )
    {
//       echo count($this->verificationErrors);
//       $this->fail(implode("\n",$this->verificationErrors));
    }

    fclose($this->fp);
  }


  protected function countOfWBoxEntries()
  {
    // Compute number of entries (count number of '()')
    // Here $n-1 has to be used because the title of W box contains '()' too
    // 10-12-07 (Markus): no longer
    return preg_match_all('/\(\d+\)/', $this->selenium->getText("autocomplete_W_boxes_1"), $this->matches);
//    return preg_match_all('/\(\d+\)/', $this->selenium->getText("autocomplete_W_boxes_1"), $this->matches) - 1;
  }


  protected function countOfCBoxEntries()
  {
    // Compute number of entries (count number of '()')
    return preg_match_all('/\(\d+\)/', $this->selenium->getText("autocomplete_C_boxes_1"), $this->matches);
  }


  protected function countOfFBoxEntries($index = 1)
  {
    // Compute number of entries (count number of '()')
    return preg_match_all('/\(\d+\)/', $this->selenium->getText("autocomplete_F_boxes_" . $index), $this->matches);
  }


  protected function countOfHits()
  {
    // Compute number of entries (count number of '()')
    preg_match('/Hits.*\b\d+\b.*-.*\b(\d+)\b.*\bof\b/', $this->selenium->getText("hits_title"), $this->matches);
    return $this->matches[1];
  }


  protected function countOfMoreHits()
  {
    // Compute number of more links in hits
    $quote = "'\"";
    $exp = "/id=[$quote]more_hits[$quote]/g";
    return $this->selenium->getEval("this.browserbot.findElement('autocomplete_hit_box_body').innerHTML.match($exp).length");

    // The following attempt to use getEval() to get the innerHTML and parse this by PHP failed, because the result of getEval() was uncomplete (broken) for large hits
    // So let javascript parse the innerHTML so that getEval delivers directly the amount of more links
//    $html = $this->selenium->getEval('this.browserbot.findElement("autocomplete_hit_box_body").innerHTML');
//    return preg_match_all($exp, $html, $this->matches);
  }


//
//  /**
//   * Testing more/less links of a single facet box
//   *
//   */
//  function testMoreLessOfSingleBoxes()
//  {
//    $this->selenium->open("index.php?language=en");
//    $this->selenium->createCookie("hpp=5", "");
//    $this->selenium->createCookie("mcs=4", "");
//    $this->selenium->createCookie("qt=HWFC", "");
//    $this->selenium->createCookie("mo=4", "");
//    $this->selenium->click("reset_link");
//    $this->selenium->waitForPageToLoad("30000");
//
//    $this->selenium->type("autocomplete_query", "inf");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//    sleep(2 * $this->sharedFixture['waitForExecution']);
//
//    // The first click at "more" of third F_box
//    $this->selenium->click("//a[contains(@href, \"javascript:AC.more_link('F',3,8);\")]");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfWBoxEntries(), 4);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 8);
//
//    // The second click at "more" of third F_box
//    $this->selenium->click("//a[contains(@href, \"javascript:AC.more_link('F',3,12);\")]");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfWBoxEntries(), 4);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 12);
//
//    // The first click at "more" of first F_box
//    $this->selenium->click("more_link_F");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfWBoxEntries(), 4);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 8);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 12);
//
//    // Click at "less" of third F_box
//    $this->selenium->click("//a[contains(@href, \"javascript:AC.more_link('F',3,8);\")]");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfWBoxEntries(), 4);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 8);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 8);
//
//    // Navigating through hits must not change the box sizes
//    $this->selenium->click("next_hits");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfWBoxEntries(), 4);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 8);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 8);
//
//    // Click at "less" of first F_box
//    $this->selenium->click("//a[contains(@href, \"javascript:AC.more_link('F',1,4);\")]");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfWBoxEntries(), 4);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 8);
//
//    // Firefox can simulate typing single keys
//    if ($this->sharedFixture['browser'] == "f") {
//      $this->selenium->typeKeys("autocomplete_query", "o");
//    }
//    else $this->selenium->type("autocomplete_query", "info");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//
//    sleep($this->sharedFixture['waitForExecution']);
//
//    // Now all boxes must have their initiate size values
//    $this->myAssertEquals($this->countOfWBoxEntries(), 4);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 4);
//
//    // Now the W_box
//    $this->selenium->click("//a[contains(@href, \"javascript:AC.more_link('W',1,8);\")]");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfWBoxEntries(), 8);
////    $this->myAssertEquals($this->countOfCBoxEntries(), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(1), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(2), 4);
//    $this->myAssertEquals($this->countOfFBoxEntries(3), 4);
//  }
//
//
//  /**
//   * Test some basic search interactions.
//   *
//   * This test is collection dependant.
//   *
//   * Types in 'i', 'n', 'fo', <backspace>, <backspace>, <backspace>, <backspace> until empty input field and tests for existence/no existence of
//   * all significants page elements like hits_title, subtitle etc.
//   *
//   */
//  function testBasicSearch()
//  {
//    $this->selenium->open("index.php?language=en");
//    $this->selenium->createCookie("hpp=5", "");
//    $this->selenium->createCookie("qt=HWF", "");
//    $this->selenium->click("reset_link");
//    $this->selenium->waitForPageToLoad("30000");
//
//    $this->selenium->type("autocomplete_query", "i");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//    sleep(4 * $this->sharedFixture['waitForExecution']);
//
//    // Für Live-DBLP
////    $this->myAssertTrueRegEx('/Type at least \d characters per word/',$this->selenium->getText("autocomplete_hit_box_subtitle"));
//
//    // Firefox can simulate typing single keys
//    if ($this->sharedFixture['browser'] == "f") {
//      $this->selenium->typeKeys("autocomplete_query", "n");
//    }
//    else $this->selenium->type("autocomplete_query", "inf");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//
//    sleep($this->sharedFixture['waitForExecution'] * 2);
//    $this->myAssertTrueRegEx('/zoomed in on \d+ documents/',$this->selenium->getText("autocomplete_hit_box_subtitle"));
//
//    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\binf\b/',$this->selenium->getText("hits_title"));
//
//    // Firefox can simulate typing single keys
//    if ($this->sharedFixture['browser'] == "f") {
//      $this->selenium->typeKeys("autocomplete_query", "o");
//    }
//    else $this->selenium->type("autocomplete_query", "info");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    $this->myAssertTrueRegEx('/zoomed in on \d+ documents/',$this->selenium->getText("autocomplete_hit_box_subtitle"));
//
//    $this->myAssertTrueRegEx('/Refine by WORD/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
//
//    $this->myAssertTrueRegEx('/Refine by AUTHOR/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
//
//    $this->myAssertTrueRegEx('/Li Da Xu, A. Min Tjoa, Sohail S. Chaudhry/s',$this->selenium->getText("autocomplete_hit_box_body"));
//
//    $this->selenium->click("next_hits");
//
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*6.*-.*10.*info/',$this->selenium->getText("hits_title"));
//
//    $this->selenium->click("next_hits");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*1.*-.*5.*info/',$this->selenium->getText("hits_title"));
//
//    // Firefox can simulate backspace
//    if ($this->sharedFixture['browser'] == "f") {
//      $this->selenium->keyPress("autocomplete_query", "\\8");
//    }
//    else $this->selenium->type("autocomplete_query", "inf");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\binf\b/',$this->selenium->getText("hits_title"));
//
//    $this->myAssertTrueRegEx('/zoomed in on \d+ documents/',$this->selenium->getText("autocomplete_hit_box_subtitle"));
//
//    $this->myAssertTrueRegEx('/Refine by WORD/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
//
//    $this->myAssertTrueRegEx('/Refine by AUTHOR/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
//
//    $this->myAssertTrueRegEx('/Bioinformatics 24.*October 14-16, 2007, Beijing/s',$this->selenium->getText("autocomplete_hit_box_body"));
//
//    // Firefox can simulate backspace
//    if ($this->sharedFixture['browser'] == "f") {
//      	$this->selenium->keyPress("autocomplete_query", "\\8");
//    }
//    else {
//    	$this->selenium->type("autocomplete_query", "in");
//    }
//   	$this->selenium->keyUp("autocomplete_query", "\\");
//
//    // Firefox can simulate backspace
//    if ($this->sharedFixture['browser'] == "f") {
//      	$this->selenium->keyPress("autocomplete_query", "\\8");
//    }
//    else {
//    	$this->selenium->type("autocomplete_query", "i");
//    }
//	  $this->selenium->keyUp("autocomplete_query", "\\");
//
//    sleep($this->sharedFixture['waitForExecution'] * 3);
//
//    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\bi\b/',$this->selenium->getText("hits_title"));
////    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\b(in|inf)\b/',$this->selenium->getText("hits_title"));
//
//    // Für Live-DBLP
////    $this->myAssertTrueRegEx('/Type at least \d characters per word/',$this->selenium->getText("autocomplete_hit_box_subtitle"));
//
//    $this->myAssertTrueRegEx('/Refine by WORD/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
//
//    $this->myAssertTrueRegEx('/Refine by AUTHOR/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
//
//    $this->myAssertTrueRegEx('/200852775EEDamian W. I. Rouson, Robert Rosenberg, Xiaofeng Xu.*Navier-Stokes equations in Fortran 95/s',$this->selenium->getText("autocomplete_hit_box_body"));
////    $this->myAssertTrueRegEx('/Fabien JourdanRainer BreitlingMichael P. BarrettDavid GilbertMetaNetter.*2008articleSema KachaloRanran ZhangEduardo/s',$this->selenium->getText("autocomplete_hit_box_body"));
//
//    // Firefox can simulate backspace
//    if ($this->sharedFixture['browser'] == "f") {
//      $this->selenium->keyPress("autocomplete_query", "\\8");
//    }
//    else $this->selenium->type("autocomplete_query", "");
//
//    $this->selenium->keyUp("autocomplete_query", "\\");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->assertFalse($this->selenium->isElementPresent("hits_title"),
//      "Hits title is present (should be invisible)");
//
//    $this->assertTrue($this->selenium->isVisible("autocomplete_F_boxes_1_title"),
//      "F box title is not present (should be visible)");
//
////    $this->assertFalse($this->selenium->isVisible("autocomplete_W_boxes_1_title"),
////      "W box title is present (should be invisible)");
//
////    $this->assertFalse($this->selenium->isVisible("autocomplete_C_boxes_1_title"),
////      "C box title is present (should be invisible)");
//  }
//


  /**
   * Test navigating through completions using the key commands cursor down/up.
   *
   * This test is collection dependant.
   *
   */
  function testCompletionNavigation()
  {
    $this->assertFalse($this->sharedFixture['browserId'] == "o",
      "This test don't work with Opera (because of keyUp/keyPress don't work with Opera)");

    $this->selenium->open("index.php?language=en");
    $this->selenium->createCookie("hpp=5", "");
    $this->selenium->createCookie("mcs=5", "");
    $this->selenium->createCookie("qt=HW", "");
    $this->selenium->click("reset_link");
    $this->selenium->waitForPageToLoad("30000");

    if ($this->sharedFixture['browserId'] == "f") {
      $this->selenium->typeKeys("autocomplete_query", "info");
    }
    else $this->selenium->type("autocomplete_query", "info");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    if ($this->sharedFixture['browserId'] == "f") {
      $this->selenium->keyPress("autocomplete_query", "x");
    }
    else $this->selenium->type("autocomplete_query", "infox");
    $this->selenium->keyUp("autocomplete_query", "\\");

    sleep($this->sharedFixture['waitForExecution']);

    //  Navigate through the completions using cursor down/up
    $this->selenium->keyUp("autocomplete_query", "\\40");
    sleep($this->sharedFixture['waitForExecution'] * 2);

    $this->myAssertTrueRegEx('/Hits.*\b1\b.*\b2\b.*\binfox\b/',$this->selenium->getText("hits_title"));

    $this->myAssertEquals("infoxes", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\40");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("infoxp", $this->selenium->getValue("autocomplete_query"));
//    $this->myAssertEquals("infoxenios", $this->selenium->getValue("autocomplete_query"));

//    $this->selenium->keyUp("autocomplete_query", "\\40");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("infoxinfo", $this->selenium->getValue("autocomplete_query"));
//
//    $this->selenium->keyUp("autocomplete_query", "\\40");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("infoxinfo", $this->selenium->getValue("autocomplete_query"));

//    $this->selenium->keyUp("autocomplete_query", "\\38");
    $this->selenium->keyUp("autocomplete_query", "\\38");
    $this->selenium->keyUp("autocomplete_query", "\\38");

    $this->myAssertEquals("infox", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\38");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("infox", $this->selenium->getValue("autocomplete_query"));
  }

//
//  /**
//   * Test the more/less links of W, C, F box and the corresponding functionality with delete/insert key.
//   *
//   * This test is collection dependant.
//   *
//   */
//  function testMoreLess()
//  {
//    $this->assertFalse($this->sharedFixture['browserId'] == "o",
//      "This test don't work with Opera (because of keyUp/keyPress don't work with Opera)");
//
//    $this->selenium->open("index.php?language=en");
//    $this->selenium->createCookie("qt=WCF", "");
//    $this->selenium->createCookie("hpp=5", "");
//    $this->selenium->createCookie("mcs=5", "");
//    $this->selenium->click("reset_link");
//    $this->selenium->waitForPageToLoad("30000");
//
//    $this->selenium->type("autocomplete_query", "info");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    // Verify that W, C, F box have a more link and no less link
//    $this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//		$this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_F_boxes_1_body"));
//
//		$this->myAssertFalseRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//		$this->myAssertFalseRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_F_boxes_1_body"));
//
//		// Click more and verify that more and less links are present, click less and verify that more is present and less is not
//		$this->selenium->click("link=more");
//	  sleep($this->sharedFixture['waitForExecution']);
//
//		$this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//		$this->myAssertTrueRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//		$this->selenium->click("link=less");
//		sleep($this->sharedFixture['waitForExecution']);
//
//		$this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//		$this->myAssertFalseRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//	  // Now the same procedure with F box
//		$this->selenium->click("more_link_F");
//	  sleep($this->sharedFixture['waitForExecution']);
//
//	  $this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_F_boxes_1_body"));
//
//	  $this->myAssertTrueRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_F_boxes_1_body"));
//
//	  $this->selenium->click("link=less");
//	  sleep($this->sharedFixture['waitForExecution']);
//
//	  $this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_F_boxes_1_body"));
//
//	  $this->myAssertFalseRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_F_boxes_1_body"));
//
//	  // Now the previous test with W box, but instead of clicking more/less link use the delete/insert key
//		$this->selenium->keyUp("autocomplete_query", "\\46");
//	  sleep($this->sharedFixture['waitForExecution']);
//
//	  $this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//	  $this->myAssertTrueRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//	  $this->selenium->keyUp("autocomplete_query", "\\45");
//	  sleep($this->sharedFixture['waitForExecution']);
//
//	  $this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//	  $this->myAssertFalseRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//		// Type x and verify that W box have no more and no less link
//	  // Firefox can simulate backspace
//	  if ($this->sharedFixture['browser'] == "f") {
//	    $this->selenium->keyPress("autocomplete_query", "x");
//	  }
//	  else {
//	    $this->selenium->type("autocomplete_query", "infox");
//	  }
//	  $this->selenium->keyUp("autocomplete_query", "\\");
//	  sleep($this->sharedFixture['waitForExecution']);
//
//
//	  $this->myAssertFalseRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//	  $this->myAssertFalseRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//		$this->selenium->controlKeyDown();
//		$this->selenium->keyUp("autocomplete_query", "\\37");
//		$this->selenium->controlKeyUp();
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/\[.*more.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//
//    $this->myAssertFalseRegEx('/\[.*less.*\]/', $this->selenium->getText("autocomplete_W_boxes_1_body"));
//	}
//
//
//  /**
//   * Test whether the summary line in access.log is written.
//   *
//   * This test is collection independant, but needs the path to the access.log file
//   * (which should be provided in testsConfig.php by $this->myLogPath)
//   *
//   */
//  function testSummaryLog()
//  {
//    $testQuery = "xyzdf";
//    $this->selenium->open("index.php?language=en");
//    $this->selenium->createCookie("hpp=5", "");
//    $this->selenium->createCookie("qt=HWF", "");
//    $this->selenium->click("reset_link");
//    $this->selenium->waitForPageToLoad("30000");
//    $this->selenium->type("autocomplete_query", $testQuery);
//    $this->selenium->keyUp("autocomplete_query", "\\");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    // Depending on type of file descriptor use file() or fopen()+fseek() (the last one for performance reasons)
//    if (preg_match('/http.*/', $this->sharedFixture['logPath'])) {
//      // Get the last summary line
//      $t = file($this->sharedFixture['logPath']);
//      $this->assertTrue((bool)$t !== false,
//          "\nCould not load log file '" . $this->sharedFixture['logPath'] . "' from server");
//      // CHANGED 30-01-08 (Markus): was $t[sizeof($t) - 2]
//      $summaryLine = $t[sizeof($t) - 1];
////      $this->write("\nText from log: " . print_r($t, true));
//    }
//    else {
//      // Get the block containing last summary line
//      $handle = fopen($this->sharedFixture['logPath'], "r");
//      $this->assertTrue((bool)$handle !== false,
//          "\nLog file '" . $this->sharedFixture['logPath'] . "' not found");
//      // Seek to the end
//      fseek($handle, -300, SEEK_END);
//      $summaryLine = fread($handle, 300);
//    }
////    $this->write("\nSummary line from log: " . $summaryLine);
//    $this->assertTrue((bool)preg_match("/.*\b$testQuery\b.*\bC\dH\dF\d\b.*\b\d+ msecs \([ ]*\d+%[ ]*\d+%[ ]*\d+%\)/", $summaryLine) == 1,
//        "Summary line don't look like expected: " . $summaryLine);
//  }
//
//
//  /**
//   * Test navigating through hits using the links for page down/up, the key commands page down/up and the more link.
//   *
//   * This test is collection independant
//   * if we get for search string 'info' at least 15 hits
//   *
//   */
//  function testHitsNavigation()
//  {
//    $this->assertFalse($this->sharedFixture['browserId'] == "o",
//      "This test don't work with Opera (because of keyUp/keyPress don't work with Opera)");
//
//    $this->selenium->open("index.php?language=en");
//    $this->selenium->createCookie("hpp=5", "");
//    $this->selenium->createCookie("qt=HWF", "");
//    $this->selenium->click("reset_link");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->selenium->type("autocomplete_query", "info");
//    $this->selenium->keyUp("autocomplete_query", "\\");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    // Test whether there is a more_hits link
//    $this->myAssertEquals($this->selenium->getText("//a[@id='more_hits']"), "more");
//
//    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    // First navigation by clicking page up/down links
//    $this->selenium->click("next_hits");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    $this->selenium->click("next_hits");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->assertTrue((bool)preg_match('/Hits.*\b11\b.*-.*\b15\b.*\binfo\b/',$this->selenium->getText("hits_title")));
//
//    $this->selenium->click("link=PageUp");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    $this->selenium->click("link=PageUp");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    // Now the navigation with page up/down
//    $this->selenium->keyUp("autocomplete_query", "\\34");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    $this->selenium->keyUp("autocomplete_query", "\\34");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b11\b.*-.*\b15\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    $this->selenium->keyUp("autocomplete_query", "\\33");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//
//    $this->selenium->keyUp("autocomplete_query", "\\33");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertTrueRegEx('/Hits.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));
//    $this->myAssertEquals($this->countOfHits(), 5);
//
//    $this->selenium->click("more_hits");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfHits(), 10);
//
//    $this->selenium->click("more_hits");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->myAssertEquals($this->countOfHits(), 15);
//
//    // The more link is only allowed once in the hits list
//    $this->myAssertEquals($this->countOfMoreHits(), 1);
//  }
//
//
//  /**
//   * Test navigating through history using the key commands CTRL + cursor left/right.
//   *
//   * This test is collection dependant
//   *
//   */
//  function testHistory()
//  {
//    $this->selenium->open("index.php?language=en");
//    $this->selenium->createCookie("hpp=20", "");
//    $this->selenium->click("reset_link");
//    $this->selenium->waitForPageToLoad("3000");
//
//    $this->myAssertTrueRegEx("/searching in \d+ documents/", $this->selenium->getText("id=autocomplete_hit_box_subtitle"));
//
//    $this->selenium->click("link=ISCAS");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->selenium->click("link=2004");
//    sleep($this->sharedFixture['waitForExecution']);
//    $this->selenium->click("link=Kari Halonen");
//    sleep($this->sharedFixture['waitForExecution']);
//
//    // History back
//    $this->selenium->controlKeyDown();
//		$this->selenium->keyUp("autocomplete_query", "\\37");
//		$this->selenium->controlKeyUp();
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("year:2004 venue:iscas", $this->selenium->getValue("id=autocomplete_query"));
//
//    $this->myAssertTrueRegEx('/Hits.*1.*20.*1262/',$this->selenium->getText("id=hits_title"));
//
//    // History back
//    $this->selenium->controlKeyDown();
//		$this->selenium->keyUp("autocomplete_query", "\\37");
//		$this->selenium->controlKeyUp();
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("venue:iscas", $this->selenium->getValue("autocomplete_query"));
//
//    $this->myAssertTrueRegEx('/Hits.*1.*20.*11201/',$this->selenium->getText("id=hits_title"));
//
//    // History back
//    $this->selenium->controlKeyDown();
//		$this->selenium->keyUp("autocomplete_query", "\\37");
//		$this->selenium->controlKeyUp();
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("", $this->selenium->getValue("autocomplete_query"));
//
//    $this->assertFalse($this->selenium->isElementPresent("hits_title"),
//      "hits_title is present (should be not)");
//
//    // History forward
//		$this->selenium->controlKeyDown();
//		$this->selenium->keyUp("autocomplete_query", "\\39");
//		$this->selenium->controlKeyUp();
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("venue:iscas", $this->selenium->getValue("autocomplete_query"));
//
//    $this->myAssertTrueRegEx('/Hits.*1.*20.*11201/',$this->selenium->getText("id=hits_title"));
//
//    // History forward
//		$this->selenium->controlKeyDown();
//		$this->selenium->keyUp("autocomplete_query", "\\39");
//		$this->selenium->controlKeyUp();
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("year:2004 venue:iscas", $this->selenium->getValue("id=autocomplete_query"));
//
//    $this->myAssertTrueRegEx('/Hits.*1.*20.*1262/',$this->selenium->getText("id=hits_title"));
//
//    // History forward
//		$this->selenium->controlKeyDown();
//		$this->selenium->keyUp("autocomplete_query", "\\39");
//		$this->selenium->controlKeyUp();
//    sleep($this->sharedFixture['waitForExecution']);
//
//    $this->myAssertEquals("author:karihalonen year:2004 venue:iscas", $this->selenium->getValue("id=autocomplete_query"));
//
//    $this->myAssertTrueRegEx('/Hits.*1.*11.*11/',$this->selenium->getText("id=hits_title"));
//  }

}


?>