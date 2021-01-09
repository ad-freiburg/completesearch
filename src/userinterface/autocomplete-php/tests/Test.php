<?php

error_reporting(E_ALL);

//include "../autocomplete/log.php";

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
    "testChangingLanguage" => "switch language from deutsch to English",
    "testHitsNavigation" => "navigating hits using page down/up links, keys and more link",
    "testHistory" => "browser back/forward buttons",
    "testCookie" => "storing options via cookies",
    "testChangingOptions" => "changing options and check some effects",
    "testMinMaxBox" => "moving boxes to right and back (min-/maximize)",
    "testBasicSearch" => "basic search interaction",
    "testTopXOfSingleBoxes" => "top X links of a single facet box",
    "testISO_8859_1" => "special characters like umlauts in ISO-8859-1 encoded collection",
    "testUTF_8" => "special characters like umlauts in UTF-8 encoded collection"
    );

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


  /**
   * Count the number of entries (completions) in a the {$index}th box of type $type.
   * Not for H box (hits), there is an own function countOfHits()
   *
   * @param string $type
   * @param string $index
   * @return integer the number of entries
   */
  protected function countOfBoxEntries($type, $index)
  {
    // This don't work with SeleniumRC 1 beta 1 (probably in beta 2, try again!)
    // return $this->selenium->getXpathCount("//div[@id='autocomplete_F_boxes_$index_body']/div") - 1;

    // The following attempt to use getText() fails for large texts (returned text is sometimes incomplete (broken))
    // Compute number of entries (count number of '()')
    // return preg_match_all('/\(\d+\)/', $this->selenium->getText("autocomplete_F_boxes_" . $index), $this->matches);

    // So let javascript parse the text so that getEval delivers directly the amount of completions
    $exp = "/\(\d+\)/g";
		return $this->selenium->getEval("this.browserbot.findElement('autocomplete_{$type}_boxes_{$index}_body').innerHTML.match($exp).length");
  }


  /**
   * Count the number of hits in the H box
   *
   * @return unknown
   */
  protected function countOfHits()
  {
    // Compute number of entries (count number of '()')
    preg_match('/Treffer.*\b\d+\b.*-.*\b(\d+)\b.*\bvon\b/', $this->selenium->getText("hits_title"), $this->matches);
    return $this->matches[1];
  }


  /**
   * Count the number of "more" links in the hit box (should be zero or one)
   *
   * @return integer the number of "more" links
   */
  protected function countOfMoreHits()
  {
    // Compute number of more links in hits
    // This don't work with SeleniumRC 1 beta 1 (probably in beta 2, try again!)
    // return $this->selenium->getXpathCount("/dl[@id='more_hits']");

    // The more complex regular expression testing against "id=..." don't work in IE (why?)
		if ($this->sharedFixture['browserId'] == "i") {
  		$exp = "/more_hits/g";
		}
		else {
			$quote = "'" . '"';
			$exp = "/dl id=[$quote]more_hits[$quote]/g";
		}

		return $this->selenium->getEval("this.browserbot.findElement('autocomplete_H_boxes_1_body').innerHTML.match($exp).length");

    // The following attempt to use getEval() to get the innerHTML and parse this by PHP failed, because the result of getEval() was incomplete (broken) for large hits
    // So let javascript parse the innerHTML so that getEval delivers directly the amount of more links
    // $html = $this->selenium->getEval('this.browserbot.findelement("autocomplete_h_boxes_1_body").innerhtml');
    // return preg_match_all($exp, $html, $this->matches);
  }


  protected function waitForElement($locator, $timeout)
  {
    $s = "selenium.isElementPresent('$locator');";
    $this->selenium->WaitForCondition($s, $timeout);
  }


  /**
   * Waits for an element to show $text
   *
   * @param string $locator the locator of the element
   * @param string $text text which should be shown
   */
  protected function waitForText($locator, $text)
  {
    $wait = 100000; // time to wait between two steps in milliseconds
    $steps = 1000000 / $wait; // max number of steps to perform
    $duration = $this->sharedFixture['waitForExecution'];

    for ($intervall = 0; ; $intervall++) {
      if ($intervall > $steps * $duration) $this->fail("timeout");
      try {
        if ($this->selenium->getText($locator) == $text) break;
      } catch (Exception $e) {}
      usleep($wait);
    }
  }


  protected function waitForElement2($locator)
  {
    $wait = 100000; // time to wait between two steps in milliseconds
    $steps = 1000000 / $wait; // max number of steps to perform
    $duration = $this->sharedFixture['waitForExecution'];

    for ($intervall = 0; ; $intervall++) {
      if ($intervall > $steps * $duration) $this->fail("timeout");
      try {
        if ($this->selenium->isElementPresent($locator)) break;
      } catch (Exception $e) {}
      usleep($wait);
    }
  }


  /**
   * This function performs the start sequence to initialize and open the test page
   * VERY VERY IMPORTANT: in autocomplete_config.php $config->javascript must be set to true!!
   * Otherwise the most of the tests will fail!
   *
   */
  protected function init($_url = "index.php?language=de")
  {
    global $url;
    $this->selenium->open($url . $_url);
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);  // milliseconds

    // By setting cookies and click the reset link we can initialize the user options
    $this->selenium->createCookie("hpp=5", "");
    $this->selenium->createCookie("qt=HWF", "");
    $this->selenium->createCookie("mo=10", "");
    $this->selenium->click("link=reset");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);  // milli seconds

    // Because index.php is called the first time and all completion and more/less links are non-Javascript links
    //  the index page is reloaded by the Javascript function init().
    //  So the test has to do another sleep()
    // NOT necessary if $config->javascript == true
    // sleep(2 * $this->sharedFixture['waitForExecution']);
  }


  /**
   * Testing more/less links of a single facet box
   *
   */
  function testTopXOfSingleBoxes()
  {
    $this->init();

    $this->myAssertEquals("4", $this->countOfBoxEntries("F", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [Top 250] [alle 278]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("Top 250", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[2]"));
    $this->myAssertEquals("alle 278", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[3]"));

    $this->myAssertEquals("4", $this->countOfBoxEntries("F", 2));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 63]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_2_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_F_boxes_2_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("alle 63", $this->selenium->getText("//div[@id='autocomplete_F_boxes_2_body']/div/span[@class='more_link']/a[2]"));

    $this->myAssertEquals("4", $this->countOfBoxEntries("F", 3));
    $this->myAssertEquals("[Top 4] [alle 21]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_3_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("alle 21", $this->selenium->getText("//div[@id='autocomplete_F_boxes_3_body']/div/span[@class='more_link']/a[1]"));

    $this->selenium->click("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]");
    $this->waitForText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]", "Top 4");
//    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("50", $this->countOfBoxEntries("F", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [Top 250] [alle 278]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 4", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("Top 250", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[2]"));
    $this->myAssertEquals("alle 278", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[3]"));

    $this->selenium->click("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[2]");
    $this->waitForText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[2]", "Top 50");
//    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("250", $this->countOfBoxEntries("F", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [Top 250] [alle 278]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 4", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[2]"));
    $this->myAssertEquals("alle 278", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[3]"));

    $this->selenium->click("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[3]");
    $this->waitForText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[3]", "Top 250");
//    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("278", $this->countOfBoxEntries("F", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [Top 250] [alle 278]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 4", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[2]"));
    $this->myAssertEquals("Top 250", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[3]"));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 63]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_2_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_F_boxes_2_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("alle 63", $this->selenium->getText("//div[@id='autocomplete_F_boxes_2_body']/div/span[@class='more_link']/a[2]"));
    $this->myAssertEquals("[Top 4] [alle 21]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_3_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("alle 21", $this->selenium->getText("//div[@id='autocomplete_F_boxes_3_body']/div/span[@class='more_link']/a[1]"));

    $this->selenium->click("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]");
    $this->waitForText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]", "Top 50");
//    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("4", $this->countOfBoxEntries("F", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [Top 250] [alle 278]", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("Top 250", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[2]"));
    $this->myAssertEquals("alle 278", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div/span[@class='more_link']/a[3]"));

    // Now we check the W boxes
    $this->selenium->type("autocomplete_query", "int");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("4", $this->countOfBoxEntries("W", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 213]", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("alle 213", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[2]"));

    $this->selenium->click("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]");
    $this->waitForText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]", "Top 4");
//    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("50", $this->countOfBoxEntries("W", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 213]", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 4", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("alle 213", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[2]"));

    $this->selenium->click("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[2]");
    $this->waitForText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[2]", "Top 50");
//    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("213", $this->countOfBoxEntries("W", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 213]", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 4", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[2]"));

    $this->selenium->click("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]");
    $this->waitForText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]", "Top 50");
//    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("4", $this->countOfBoxEntries("W", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 213]", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 50", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("alle 213", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[2]"));
  }


  /**
   * Change user preferences (options page) and test some of the changes (by counting the entries of W box, for example).
   *
   * This test is collection independant, but your collection must provide at least 5 completions for the queries W, C, F
   * and 3 hits for the query "inf".
   *
   * First initialize alle user preferences and then types in 'i', 'n', 'f',
   * go to options page, changes all values and return to main page.
   * There check count of boxes W, C, F and of the hits against the values we set the user preferences in the step before.
   * Finally go to the options page again and check all values there against the values we set the user preferences in the step before.
   *
   */
  function testChangingOptions()
  {
    $this->selenium->open("index.php?language=de");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);

    // max_completions_show is (mcs) now replaced by the first element of top_hits
    // $this->selenium->createCookie("mcs=4", "");
    $this->selenium->createCookie("mcsr=40", "");
    $this->selenium->createCookie("hpp=5", "");
    $this->selenium->createCookie("mcl=40", "");
    $this->selenium->createCookie("eph=3", "");
    $this->selenium->createCookie("er=5", "");
    $this->selenium->createCookie("dm=1", "");
    $this->selenium->createCookie("ll=4", "");
    $this->selenium->createCookie("qt=H", "");
    $this->selenium->createCookie("mo=10", "");
    $this->selenium->click("reset_link");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);

    // We need a test query with a lot of results
    $this->selenium->type("autocomplete_query", "inf");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("autocomplete_options_title");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);
    $this->selenium->type("mcsr", "30");
    $this->selenium->type("hpp", "3");
    $this->selenium->type("mcl", "30");
    $this->selenium->type("eph", "4");
    $this->selenium->type("er", "4");
    $this->selenium->type("dm", "2");
    $this->selenium->type("ll", "3");
    $this->selenium->type("qt", "HWF");
    $this->selenium->type("mo", "5");
    $this->selenium->click("ok");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);

    // Now test the number of box entries and hits
    $this->myAssertEquals($this->countOfHits(), 3);
    $this->myAssertEquals($this->countOfBoxEntries("W", 1), 4);
    $this->myAssertEquals($this->countOfBoxEntries("F", 1), 4);
    $this->selenium->click("autocomplete_options_title");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);

//    $this->myAssertEquals("5", $this->selenium->getValue("id=mcs"));
    $this->myAssertEquals("30", $this->selenium->getValue("id=mcsr"));
    $this->myAssertEquals("3", $this->selenium->getValue("id=hpp"));
    $this->myAssertEquals("4", $this->selenium->getValue("id=eph"));
    $this->myAssertEquals("4", $this->selenium->getValue("id=er"));
    $this->myAssertEquals("2", $this->selenium->getValue("id=dm"));
    $this->myAssertEquals("3", $this->selenium->getValue("id=ll"));
    $this->myAssertEquals("HWF", $this->selenium->getValue("id=qt"));
  }


  /**
   * Set cookies for all user preferences and compare them against the options page.
   *
   * This test is collection independant.
   *
   * First initialize alle user preferences and then go to options page
   * and compare the values with those ones we set the cookies to.
   *
   */
  function testCookie()
  {
    $this->selenium->open("index.php?language=de");
    $this->selenium->createCookie("mcsr=40", "");
    $this->selenium->createCookie("hpp=5", "");
    $this->selenium->createCookie("mcl=40", "");  // max_completion_length
    $this->selenium->createCookie("eph=3", "");
    $this->selenium->createCookie("er=5", "");
    $this->selenium->createCookie("dm=1", "");
    $this->selenium->createCookie("ll=4", "");
    $this->selenium->createCookie("qt=HWF", "");
    $this->selenium->createCookie("mo=10", "");
    $this->selenium->click("reset_link");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);

    $this->selenium->click("autocomplete_options_title");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);

  	// settings->max_completions_show is now defined by the first member of the settings->top_hits array (new "top X" concept)
    // $this->myAssertEquals("6", $this->selenium->getValue("id=mcs"));
    $this->myAssertEquals("40", $this->selenium->getValue("id=mcsr"));
    $this->myAssertEquals("5", $this->selenium->getValue("id=hpp"));
    $this->myAssertEquals("40", $this->selenium->getValue("id=mcl"));
    $this->myAssertEquals("3", $this->selenium->getValue("id=eph"));
    $this->myAssertEquals("5", $this->selenium->getValue("id=er"));
    $this->myAssertEquals("1", $this->selenium->getValue("id=dm"));
    $this->myAssertEquals("4", $this->selenium->getValue("id=ll"));
    $this->myAssertEquals("HWF", $this->selenium->getValue("id=qt"));
  }


  /**
   * Test moving of W and F box.
   *
   * This test is collection independant, but your collection must provide at least $mcsr=40 completions for the queries W and F.
   * If not so, reduce the value of $mcsr.
   *
   * Initialize cookies for hpp, mcs, mcsr and qt.
   * Click F box to move it to the right, count the number of entries of F box and compare the result with initiate value of mcsr.
   * Send query "inf" and repeat what you did for F box with W and C boxes.
   *
   */
  function testMinMaxBox()
  {
    $matches = array();
    // Number of entries in the right boxes used for this test case
    $mcsr = 40;

    $this->selenium->open("index.php?language=de");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);
    $this->selenium->createCookie("hpp=5", "");
    $this->selenium->createCookie("mcsr=$mcsr", "");
    $this->selenium->createCookie("qt=HWF", "");
    $this->selenium->click("reset_link");
    $this->selenium->waitForPageToLoad(2000 * $this->sharedFixture['waitForExecution']);

    $this->selenium->click("move_F_boxes_1");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertTrue($this->selenium->isVisible("detail"));
    // Compute number of entries (count number of '()')
    $n = preg_match_all('/\(\d+\)/', $this->selenium->getText("autocomplete_F_boxes_1"), $matches);
    $this->myAssertEquals($n, $mcsr);
    $this->selenium->click("move_F_boxes_1");
    $this->myAssertFalse($this->selenium->isVisible("detail"));

    $this->selenium->type("autocomplete_query", "inf");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("move_W_boxes_1");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertTrue($this->selenium->isVisible("detail"));
    $this->myAssertEquals($this->countOfBoxEntries("W", 1), $mcsr);
    $this->selenium->click("move_W_boxes_1");
    $this->myAssertFalse($this->selenium->isVisible("detail"));
  }


  /**
   * Test some basic search interactions.
   *
   * This test is collection dependant.
   *
   * Types in 'i', 'n', 'fo', <backspace>, <backspace>, <backspace>, <backspace> until empty input field and tests for existence/no existence of
   * all significants page elements like hits_title, subtitle etc.
   *
   */
  function testBasicSearch()
  {
    $this->init();

    $this->selenium->type("autocomplete_query", "i");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Geben Sie bitte mindestens \d Zeichen ein/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));

    // Firefox can simulate typing single keys
    if ($this->sharedFixture['browser'] == "f") {
      $this->selenium->typeKeys("autocomplete_query", "n");
    }
    else $this->selenium->type("autocomplete_query", "in");
    $this->selenium->keyUp("autocomplete_query", "\\");

    sleep($this->sharedFixture['waitForExecution'] * 2);
    $this->myAssertTrueRegEx('/herangezoomt auf \d+ Dokumente/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));

    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*-.*\b5\b.*\bin\b/',$this->selenium->getText("hits_title"));

    // Firefox can simulate typing single keys
    if ($this->sharedFixture['browser'] == "f") {
      $this->selenium->typeKeys("autocomplete_query", "fo");
    }
    else $this->selenium->type("autocomplete_query", "info");
    $this->selenium->keyUp("autocomplete_query", "\\");

    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));
    $this->myAssertTrueRegEx('/herangezoomt auf \d+ Dokumente/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));
    $this->myAssertTrueRegEx('/Verfeinere nach WORT/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
    $this->myAssertTrueRegEx('/Verfeinere nach AUTHOR/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
    $this->myAssertTrueRegEx('/A Simple Algorithm for Merging Two Disjoint Linearly-Ordered Sets.*Frank K. Hwang, Shen Lin.*Linear Algorithms to Recognize Interval Graphs and Test for the Consecutive Ones Property/s', $this->selenium->getText("autocomplete_H_boxes_1_body"));
//    $this->myAssertTrueRegEx('/A probabilistic model.*Karen Sparck Jones, Steve Walker, Stephen E. Robertson.*Information Processing and Management/s',$this->selenium->getText("autocomplete_H_boxes_1_body"));

    $this->selenium->click("next_hits");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*6.*-.*10.*info/',$this->selenium->getText("hits_title"));
    $this->myAssertTrueRegEx('/Indexing by Latent Semantic Analysis.*Scott C. Deerwester, Susan T. Dumais, Thomas K. Landauer, George W. Furnas, Richard A. Harshman.*The Use of Phrases and Structured Queries in Information Retrieval/s',$this->selenium->getText("autocomplete_H_boxes_1_body"));
//    $this->myAssertTrueRegEx('/Document Language Models, Query Models, and Risk Minimization for Information Retrieval.*John D. Lafferty, ChengXiang Zhai/s',$this->selenium->getText("autocomplete_H_boxes_1_body"));

    $this->selenium->click("link=Bild");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*1.*-.*5.*info/',$this->selenium->getText("hits_title"));

    // Firefox can simulate backspace
    if ($this->sharedFixture['browser'] == "f") {
      $this->selenium->keyPress("autocomplete_query", "\\8");
    }
    else $this->selenium->type("autocomplete_query", "inf");
    $this->selenium->keyUp("autocomplete_query", "\\");

    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*-.*\b5\b.*\binf\b/',$this->selenium->getText("hits_title"));
    $this->myAssertTrueRegEx('/herangezoomt auf \d+ Dokumente/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));
    $this->myAssertTrueRegEx('/Verfeinere nach WORT/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
    $this->myAssertTrueRegEx('/Verfeinere nach AUTHOR/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
    $this->myAssertTrueRegEx('/A Simple Algorithm for Merging Two Disjoint Linearly-Ordered Sets.*Frank K. Hwang, Shen Lin.*Linear Algorithms to Recognize Interval Graphs and Test for the Consecutive Ones Property/s',$this->selenium->getText("autocomplete_H_boxes_1_body"));
//    $this->myAssertTrueRegEx('/A probabilistic model.*Karen Sparck Jones, Steve Walker, Stephen E. Robertson.*Information Processing and Management/s',$this->selenium->getText("autocomplete_H_boxes_1_body"));

    // Firefox can simulate backspace
    if ($this->sharedFixture['browser'] == "f") {
      	$this->selenium->keyPress("autocomplete_query", "\\8");
    }
    else {
    	$this->selenium->type("autocomplete_query", "in");
    }
   	$this->selenium->keyUp("autocomplete_query", "\\");

    sleep($this->sharedFixture['waitForExecution']);

    // Firefox can simulate backspace
    if ($this->sharedFixture['browser'] == "f") {
      	$this->selenium->keyPress("autocomplete_query", "\\8");
    }
    else {
    	$this->selenium->type("autocomplete_query", "i");
    }
	  $this->selenium->keyUp("autocomplete_query", "\\");

    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*-.*\b5\b.*\b(in|inf)\b/',$this->selenium->getText("hits_title"));
    $this->myAssertTrueRegEx('/Geben Sie bitte mindestens \d Zeichen ein/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));
    $this->myAssertTrueRegEx('/Verfeinere nach WORT/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
    $this->myAssertTrueRegEx('/Verfeinere nach AUTHOR/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
    $this->myAssertTrueRegEx('/A Simple Algorithm for Merging Two Disjoint Linearly-Ordered Sets.*Frank K. Hwang, Shen Lin.*Linear Algorithms to Recognize Interval Graphs and Test for the Consecutive Ones Property/s',$this->selenium->getText("autocomplete_H_boxes_1_body"));
//    $this->myAssertTrueRegEx('/The String B-tree: A New Data Structure for String Search in External Memory and Its Applications.*Paolo Ferragina, Roberto Grossi/s',$this->selenium->getText("autocomplete_H_boxes_1_body"));

    // Firefox can simulate backspace
    if ($this->sharedFixture['browser'] == "f") {
      $this->selenium->keyPress("autocomplete_query", "\\8");
    }
    else $this->selenium->type("autocomplete_query", "");

    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    // Das kommt, sobald subtitle aus H_boxes draußen ist (denn so wie bisher klappt das sichbar/unsichtbar machen nicht)
//    $this->assertFalse($this->selenium->isVisible("autocomplete_H_boxes_1_title"),
    $this->assertFalse($this->selenium->isElementPresent("hits_title"),
      "Hits title is present (should be invisible)");

    $this->assertTrue($this->selenium->isVisible("autocomplete_F_boxes_1_title"),
      "F box title is not present (should be visible)");

    $this->assertFalse($this->selenium->isVisible("autocomplete_W_boxes_1_title"),
      "W box title is present (should be invisible)");
  }


  /**
   * Test navigating through completions using the key commands cursor down/up.
   *
   * This test is collection dependant.
   *
   */
  function testCompletionNavigation()
  {
    if ($this->sharedFixture['browserId'] == "o") {
      $this->sharedFixture['listener']->addSkippedTest($this, new Exception("SKIPPED (don't work with Opera)"));
      return;
    }
//    $this->assertFalse($this->sharedFixture['browserId'] == "o",
//      "This test don't work with Opera (because of keyUp/keyPress don't work with Opera)");

    $this->init();

    if ($this->sharedFixture['browserId'] == "f") {
      $this->selenium->typeKeys("autocomplete_query", "inf");
    }
    else $this->selenium->type("autocomplete_query", "inf");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    if ($this->sharedFixture['browserId'] == "f") {
      $this->selenium->keyPress("autocomplete_query", "o");
    }
    else $this->selenium->type("autocomplete_query", "info");
    $this->selenium->keyUp("autocomplete_query", "\\");

    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    // Navigate through the completions using cursor down/up
    $this->selenium->keyUp("autocomplete_query", "\\40");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("information", $this->selenium->getValue("autocomplete_query"));

    // Hits has to be unchanged
    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    // Sollte "info" nicht am Anfang der Liste stehen? Hannah: ist ok
    $this->selenium->keyUp("autocomplete_query", "\\40");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("info", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\40");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("infocom", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\40");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("informal", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\40");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("informal", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\38");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("infocom", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\38");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("info", $this->selenium->getValue("autocomplete_query"));

    $this->selenium->keyUp("autocomplete_query", "\\38");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("information", $this->selenium->getValue("autocomplete_query"));

    // Now back to the first completion
    $this->selenium->keyUp("autocomplete_query", "\\38");
    $this->selenium->keyUp("autocomplete_query", "\\38");
    $this->selenium->keyUp("autocomplete_query", "\\38");

    $this->myAssertEquals("info", $this->selenium->getValue("autocomplete_query"));

    // We are at the beginning of completions; further cursor up events have to result in unchanged input in autocomplete_query
    $this->selenium->keyUp("autocomplete_query", "\\38");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals("info", $this->selenium->getValue("autocomplete_query"));
  }


 /**
   * Test changing language using the links for English/deutsch.
   *
   * This test is collection independant
   * if you get at least one hit for search string 'info'
   *
   */
  function testChangingLanguage()
  {
    $this->init();

    $this->myAssertTrueRegEx('/Suche Ã¼ber \d+ Dokumente/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));

    $this->selenium->type("autocomplete_query", "info");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    // Change language to English by clicking at 'deutsch' link
    $this->selenium->click("link=English");
    $this->waitForText("id=autocomplete_options_title", "Options");

    $this->myAssertTrueRegEx('/Hits.*1.*for.*info/',$this->selenium->getText("hits_title"));
    $this->myAssertTrueRegEx('/zoomed in on \d+ documents/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));
    $this->myAssertTrueRegEx('/Refine by WORD/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
    $this->myAssertTrueRegEx('/Refine by/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
    $this->myAssertEquals("[top 4] [top 50] [all 65]", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']"));

    // Change language to deutsch by clicking at 'deutsch' link
    $this->selenium->click("link=deutsch");
    $this->waitForText("id=autocomplete_options_title", "Einstellungen");

    $this->myAssertTrueRegEx('/Treffer.*1.*fÃ¼r.*info/',$this->selenium->getText("hits_title"));
    $this->myAssertTrueRegEx('/herangezoomt auf/',$this->selenium->getText("autocomplete_H_boxes_1_subtitle"));
    $this->myAssertTrueRegEx('/Verfeinere nach/',$this->selenium->getText("autocomplete_W_boxes_1_title"));
    $this->myAssertTrueRegEx('/Verfeinere nach/',$this->selenium->getText("autocomplete_F_boxes_1_title"));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 65]", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']"));
  }


  /**
   * Test whether the summary line in access.log is written.
   *
   * This test is collection independant, but needs the path to the access.log file
   * (which should be provided in testsConfig.php by $this->myLogPath)
   *
   */
  function testSummaryLog()
  {
    $testQuery = "xyzdf";
    $this->init();

    $this->selenium->type("autocomplete_query", $testQuery);
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    // Depending on type of file descriptor use file() or fopen()+fseek() (the last one for performance reasons)
    if (preg_match('/http.*/', $this->sharedFixture['logPath'])) {
      // Get the last summary line
      $t = file($this->sharedFixture['logPath']);
      $this->assertTrue((bool)$t !== false,
          "\nCould not load log file '" . $this->sharedFixture['logPath'] . "' from server");
      $summaryLine = $t[sizeof($t) - 1];
    }
    else {
      // Get the block containing last summary line
      $handle = fopen($this->sharedFixture['logPath'], "r");
      $this->assertTrue((bool)$handle !== false,
          "\nLog file '" . $this->sharedFixture['logPath'] . "' not found");
      // Seek to the end
      fseek($handle, -300, SEEK_END);
      $summaryLine = fread($handle, 300);
    }
    //	echo "line: $summaryLine --> ";
    $this->assertTrue((bool)preg_match("/.*\b$testQuery\b.*\bC\dH\dF\d\b.*\b\d+ msecs \([ ]*\d+%[ ]*\d+%[ ]*\d+%\)/", $summaryLine) == 1,
        "Summary line don't look like expected: " . $summaryLine);
  }


  /**
   * Test navigating through hits using the links for page down/up, the key commands page down/up and the more link.
   *
   * This test is collection independant
   * if we get for search string 'info' at least 15 hits
   *
   */
  function testHitsNavigation()
  {
    if ($this->sharedFixture['browserId'] == "o") {
      $this->sharedFixture['listener']->addSkippedTest($this, new Exception("SKIPPED (don't work with Opera)"));
      return;
    }
//    $this->assertFalse($this->sharedFixture['browserId'] == "o",
//      "This test don't work with Opera (because of keyUp/keyPress don't work with Opera)");

    $this->init();

    $this->selenium->type("autocomplete_query", "info");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    // Test whether there is a more_hits link
    $this->myAssertEquals($this->selenium->getText("//dl[@id='more_hits']"), "[mehr]");
//    $this->write("A: ".$this->selenium->click("//a[@id='a_more_hits']"));

    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    // First navigation by clicking page up/down links
    $this->selenium->click("next_hits");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    $this->selenium->click("next_hits");
    sleep($this->sharedFixture['waitForExecution']);

    $this->assertTrue((bool)preg_match('/Treffer.*\b11\b.*-.*\b15\b.*\binfo\b/',$this->selenium->getText("hits_title")));

    $this->selenium->click("link=Bild");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    $this->selenium->click("link=Bild");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    // Now the navigation with page up/down
    $this->selenium->keyUp("autocomplete_query", "\\34");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    $this->selenium->keyUp("autocomplete_query", "\\34");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b11\b.*-.*\b15\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    $this->selenium->keyUp("autocomplete_query", "\\33");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b6\b.*-.*\b10\b.*\binfo\b/',$this->selenium->getText("hits_title"));

    $this->selenium->keyUp("autocomplete_query", "\\33");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertTrueRegEx('/Treffer.*\b1\b.*-.*\b5\b.*\binfo\b/',$this->selenium->getText("hits_title"));
    $this->myAssertEquals($this->countOfHits(), 5);

    $this->selenium->click("//dl[@id='more_hits']/a");
//    $this->selenium->click("//div[@id='autocomplete_H_boxes_1_body']/div/dl/a[@class='more_link']");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals($this->countOfHits(), 10);

    $this->selenium->click("//dl[@id='more_hits']/a");
    sleep($this->sharedFixture['waitForExecution']);
    $this->myAssertEquals($this->countOfHits(), 15);

    // The more link is only allowed once in the hits list
    $this->myAssertEquals($this->countOfMoreHits(), 1);
  }


  /**
   * Test navigating through history using the browser back/forward buttons.
   *
   * This test is collection dependant
   *
   */
  function testHistory()
  {
    $this->init();

    $this->myAssertTrueRegEx("/Suche Ã¼ber \d+ Dokumente/", $this->selenium->getText("id=autocomplete_H_boxes_1_subtitle"));

    $this->selenium->click("link=SIGIR");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("link=2004");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("link=Tao Tao");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("author:taotao year:2004 venue:sigir", $this->selenium->getValue("id=autocomplete_query"));
    $this->myAssertTrueRegEx('/Ein einzelner Treffer/', $this->selenium->getText("hits_title"));

    $this->selenium->goBack();
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("year:2004 venue:sigir", $this->selenium->getValue("id=autocomplete_query"));
    $this->myAssertTrueRegEx('/Treffer.*\b1.*\b4.*\b4/', $this->selenium->getText("id=hits_title"));

    $this->myAssertEquals("4", $this->countOfBoxEntries("F", 1));
    $this->myAssertEquals("1", $this->countOfBoxEntries("F", 2));
    $this->myAssertEquals("1", $this->countOfBoxEntries("F", 3));

    $this->selenium->goBack();
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("venue:sigir", $this->selenium->getValue("autocomplete_query"));
    $this->myAssertTrueRegEx('/Treffer.*1.*5.*22/', $this->selenium->getText("hits_title"));

    $this->selenium->goBack();
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("", $this->selenium->getValue("autocomplete_query"));
    $this->assertFalse($this->selenium->isElementPresent("hits_title"), "hits_title is present (should be not)");

    $this->selenium->type("autocomplete_query", "int");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("next_hits");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("link=mehr");
    sleep($this->sharedFixture['waitForExecution']);

    // Go to external link by clicking the first hit and come back
    $this->selenium->click("//div[@id='autocomplete_H_boxes_1_body']/dl[1]/dt/a/font");
    $this->selenium->waitForPageToLoad(20000 * $this->sharedFixture['waitForExecution']);  // milliseconds
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->goBack();
    $this->selenium->waitForPageToLoad(20000 * $this->sharedFixture['waitForExecution']);  // milliseconds
//    $this->waitForElement2("hits_title");
    sleep($this->sharedFixture['waitForExecution']);

    if ($this->sharedFixture['browserId'] == "f") {
      $this->sharedFixture['listener']->addIncompleteTest($this, new Exception("PASSED, but incomplete (cache problem in Firefox)"));
    }
    else {
      $this->myAssertEquals("int", $this->selenium->getValue("id=autocomplete_query"));
    }

    $this->waitForElement("hits_title", 5000);

    $this->write("hits_title", $this->selenium->getText("hits_title"));
    $this->write("hits_title", $this->selenium->getValue("id=hits_title"));
    $this->myAssertTrueRegEx("/Treffer.*\b6.*\b15.*\b126\b.*\bint/", $this->selenium->getText("hits_title"));

    $this->selenium->goBack();
    $this->selenium->waitForPageToLoad(200000 * $this->sharedFixture['waitForExecution']);  // milliseconds
    sleep($this->sharedFixture['waitForExecution']);
//    $this->waitForElement("hits_title", 5000);
    $this->myAssertTrueRegEx("/Treffer.*\b6.*\b10.*\b126\b.*\bint/", $this->selenium->getText("hits_title"));

    if ($this->sharedFixture['browserId'] == "i") {
      $this->sharedFixture['listener']->addIncompleteTest($this, new Exception("PASSED, but incomplete (IE uses Iframe for history)"));
    }
    else {
      $this->selenium->goBack();
      sleep($this->sharedFixture['waitForExecution']);
      $this->myAssertTrueRegEx("/Treffer.*\b1.*\b5.*\b126\b.*\bint/", $this->selenium->getText("hits_title"));
    }
  }


  /**
   * Checking whether history remains valid after browser back from external page.
   *
   * This test is collection dependant and has the following constraints due to the different browser limitations:
   * IE: this test works only until the first back step after return from an external page
   * (because IE's history machanism is destroyed by follow links to external pages due to its history implementation by an iframe).
   * Firefox: this test passes but is incomplete because Firefox forgets content of the query input after return from an external page (why?)
   * Opera: this test will skip because Opera forgets *all* except of the query input (although Opera works well if used by a human user)
   *
   */
  function testBackFromExternalPage()
  {
    if ($this->sharedFixture['browserId'] == "o") {
      $this->sharedFixture['listener']->addSkippedTest($this, new Exception("SKIPPED (don't work with Opera)"));
      return;
    }

    $this->init();

    $this->selenium->type("autocomplete_query", "int");
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("next_hits");
    sleep($this->sharedFixture['waitForExecution']);

    $this->selenium->click("link=mehr");
    sleep($this->sharedFixture['waitForExecution']);

    // Go to external link by clicking the first hit and come back
    $this->selenium->click("//div[@id='autocomplete_H_boxes_1_body']/dl[1]/dt/a/font");
    sleep($this->sharedFixture['waitForExecution']);
    $this->selenium->goBack();
    sleep($this->sharedFixture['waitForExecution']);

    if ($this->sharedFixture['browserId'] == "f") {
      $this->sharedFixture['listener']->addIncompleteTest($this, new Exception("PASSED, but incomplete (cache problem in Firefox)"));
//      $this->sharedFixture['listener']->addComment("**partially** ");
    }
    else {
      $this->myAssertEquals("int", $this->selenium->getValue("id=autocomplete_query"));
    }

    $this->myAssertTrueRegEx("/Treffer.*\b6.*\b15.*\b126\b.*\bint/", $this->selenium->getText("hits_title"));

    $this->myAssertEquals("50", $this->countOfBoxEntries("W", 1));
    $this->myAssertEquals("[Top 4] [Top 50] [alle 213]", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']"));
    $this->myAssertEquals("Top 4", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[1]"));
    $this->myAssertEquals("alle 213", $this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span[@class='more_link']/a[2]"));

    $this->selenium->goBack();
    sleep($this->sharedFixture['waitForExecution']);
  }


  /**
   * Test whether special characters like German umlauts are working with an ISO-8859-1 encoded collection
   *
   */
  function testISO_8859_1()
  {
    $this->init("index2.php?language=de");

    $this->selenium->type("autocomplete_query", utf8_encode("mül"));
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("müller", utf8_decode($this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div[1]/span/a")));
    $this->myAssertEquals("müll", utf8_decode($this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div[2]/span/a")));
    $this->myAssertEquals("mülheim", utf8_decode($this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div[3]/span/a")));
    $this->myAssertEquals("müllers", utf8_decode($this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div[4]/span/a")));
    $this->myAssertEquals("Mann", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[1]/span/a"));
    $this->myAssertEquals("Deutscher", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[2]/span/a"));
    $this->myAssertEquals("Frau", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[3]/span/a"));
    $this->myAssertEquals("US Amerikaner", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[4]/span/a"));
    $this->myAssertTrueRegEx("/Der Anschnitt/",$this->selenium->getText("//div[@id='autocomplete_H_boxes_1_body']"));
    $this->myAssertTrueRegEx("/Hold You Tight/",$this->selenium->getText("//div[@id='autocomplete_H_boxes_1_body']"));
  }

  
  
  /**
   * Test whether special characters like German umlauts are working with an UTF-8 encoded collection
   *
   */
  function testUTF_8()
  {
    $this->init();

    $this->selenium->type("autocomplete_query", utf8_encode("mül"));
    $this->selenium->keyUp("autocomplete_query", "\\");
    sleep($this->sharedFixture['waitForExecution']);

    $this->myAssertEquals("müller", utf8_decode($this->selenium->getText("//div[@id='autocomplete_W_boxes_1_body']/div/span/a[1]")));
    $this->myAssertEquals("Henning Müller", utf8_decode($this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[1]/span/a")));
    $this->myAssertEquals("Wolfgang Müller", utf8_decode($this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[2]/span/a")));
    $this->myAssertEquals("David McG. Squire", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[3]/span/a"));
    $this->myAssertEquals("Thierry Pun", $this->selenium->getText("//div[@id='autocomplete_F_boxes_1_body']/div[4]/span/a"));
    $this->myAssertTrueRegEx("/Content-based query of image databases: inspirations from text retrieval./", $this->selenium->getText("//div[@id='autocomplete_H_boxes_1_body']"));
    $this->myAssertTrueRegEx("/Wolfgang Müller/", utf8_decode($this->selenium->getText("//div[@id='autocomplete_H_boxes_1_body']")));
  }


    function HistoryTest()
    {
      $this->selenium->open("/markus.tests/index.php");
      $this->selenium->createCookie("hpp=5", "");
      $this->selenium->createCookie("qt=HWF", "");
      $this->selenium->click("reset_link");
      $this->selenium->waitForPageToLoad("30000");
      $this->selenium->click("link=SIGIR");
      sleep($this->sharedFixture['waitForExecution']);
      $this->selenium->click("link=2004");
      sleep($this->sharedFixture['waitForExecution']);
      $this->selenium->click("link=Tao Tao");
      sleep($this->sharedFixture['waitForExecution']);
      
      $this->myAssertEquals("author:taotao year:2004 venue:sigir", $this->selenium->getValue("autocomplete_query"));
      $this->selenium->goBack();
      sleep($this->sharedFixture['waitForExecution']);
      $this->myAssertEquals("year:2004 venue:sigir", $this->selenium->getValue("autocomplete_query"));
      $this->assertFalse($this->selenium->isVisible("//div[@id='autocomplete_W_boxes_1_body']"));
      $this->myAssertEquals("5", $this->selenium->getXpathCount("//div[@id='autocomplete_F_boxes_1_body']/div"));
      $this->myAssertEquals("1", $this->selenium->getXpathCount("//div[@id='autocomplete_F_boxes_2_body']/div"));
      $this->myAssertEquals("1", $this->selenium->getXpathCount("//div[@id='autocomplete_F_boxes_3_body']/div"));
      $this->selenium->goBack();
      sleep($this->sharedFixture['waitForExecution']);
      $this->myAssertEquals("venue:sigir", $this->selenium->getValue("autocomplete_query"));
      $this->selenium->goBack();
      sleep($this->sharedFixture['waitForExecution']);
      $this->selenium->type("autocomplete_query", "int");
      $this->selenium->keyUp("autocomplete_query", "\\");
      sleep($this->sharedFixture['waitForExecution']);
      $this->selenium->click("next_hits");
      sleep($this->sharedFixture['waitForExecution']);
      $this->selenium->click("link=mehr");
      sleep($this->sharedFixture['waitForExecution']);
      $this->selenium->click("//div[@id='autocomplete_H_boxes_1_body']/a[1]/p");
      $this->selenium->waitForPageToLoad("30000");
      $this->selenium->goBack();
      sleep($this->sharedFixture['waitForExecution']);
      $this->myAssertEquals("int", $this->selenium->getValue("autocomplete_query"));
      $this->myAssertTrueRegEx("/Treffer.*\b6.*\b15.*\b126 für int/",$this->selenium->getText("hits_title"));
      $this->selenium->goBack();
      sleep($this->sharedFixture['waitForExecution']);
      $this->myAssertTrueRegEx("/Treffer.*\b6.*\b10.*\b126 für int/",$this->selenium->getText("hits_title"));
      $this->selenium->goBack();
      sleep($this->sharedFixture['waitForExecution']);
      $this->myAssertTrueRegEx("/Treffer.*\b1.*\b5.*\b126 für int/",$this->selenium->getText("hits_title"));
      $this->myAssertEquals("5", $this->selenium->getXpathCount("//div[@id='autocomplete_W_boxes_1_body']/div"));
    }
}

// Alternative for form submit (which don't work in Selenium-IDE 1.0 beta 2
// getEval(this.browserbot.getCurrentWindow().document.getElementById('autocomplete_form').submit());
?>