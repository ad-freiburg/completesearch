<?php
// Commands are tested in Selenium RC build, and all commands are generated using XSLT.
// So this tests only if basic commands work.
if (!defined('PHPUnit_MAIN_METHOD')) {
    define('PHPUnit_MAIN_METHOD', 'AllTests::main');
}

$path = 'C:\Programme\Entwicklung\PHP\PHP5.2.4/PEAR/';
set_include_path(get_include_path() . PATH_SEPARATOR . $path);
//$path = 'C:/Program Files/Entwicklung/PHP5/PEAR/';
//set_include_path(get_include_path() . PATH_SEPARATOR . $path);

require_once 'PHPUnit/Framework/TestSuite.php';
require_once 'PHPUnit/TextUI/TestRunner.php';

require_once 'Test.php';
require_once 'Listener.php';


class AllTests extends PHPUnit_Framework_TestSuite
{

  public static function main()
  {
    global $url, $browser, $waitForExecution, $logPath, $browserDefs;

//    echo "\nReading config.php ...";
    include "config.php";

//		$parameters = array("filter" => "testMoreLess");
//		$parameters = array("xmlLogfile" => "log.txt", "verbose" => false);

//  The following don't work with multi-browser tests (failures of a previous browser are not deleted,
//  that means all following browser will fail with the same tests as the preceding browser)
//    $suite = new AllTests('CompletionServer - Selenium tests');
//    $suite->addTestSuite('Test');
//
//    $suite->setSharedFixture('url', $url);
//    $suite->setSharedFixture('logPath', $logPath);
//    $suite->setSharedFixture('waitForExecution', $waitForExecution);

    $result = new PHPUnit_Framework_TestResult;
    $listener = new MyListener;
    $result->addListener($listener);

    for ($i=0; $i < strlen($browser); $i++)
    {
      $suite = new AllTests('CompletionServer - Selenium tests');
      $suite->addTestSuite('Test');

      $suite->setSharedFixture('url', $url);
      $suite->setSharedFixture('logPath', $logPath);
      $suite->setSharedFixture('waitForExecution', $waitForExecution);
      $suite->setSharedFixture('browser', $browserDefs[$browser[$i]]->seleniumName);
      $suite->setSharedFixture('browserId', $browserDefs[$browser[$i]]->id);
      $listener->setBrowserLogName($browserDefs[$browser[$i]]->logName);
      $suite->run($result);
      $listener->resetCounter();
    }
    $listener->printResult($result);
  }

  protected function setUp()
  {
  }

  protected function tearDown()
  {
  }

  public function setSharedFixture($attribute, $value)
  {
    $this->sharedFixture[$attribute] = $value;
  }

}


class Browser
{
	public
	$id,
	$name,
	$seleniumName,
	$logName;

	function __construct ($id, $name, $seleniumName, $logName)
	{
		$this->id = $id;
		$this->name = $name;
		$this->seleniumName = $seleniumName;
		$this->logName = $logName;
	}
}

if (PHPUnit_MAIN_METHOD == 'AllTests::main') {
    AllTests::main();
}

?>
