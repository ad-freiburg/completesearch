<?php
require_once "PHPUnit/Framework/TestListener.php";
require_once "AllTests.php";


class MyListener implements PHPUnit_Framework_TestListener
{
  private $suite;
  private $summaryLine = '';
  private $testCounter = -1;
  private $logName;
  private $fp;
  private $browserLogName;


  public function __construct()
  {
    if (! file_exists("log")) {
      mkdir("log");
    }
//    $this->logName = "log/tests" . ".log";
    $this->logName = "log/tests_" . date("dMy_Hi") . ".log";
    $this->fp = fopen($this->logName, "w");
  }


  public function __destruct()
  {
    fclose($this->fp);
  }


  public function addError(PHPUnit_Framework_Test $test, Exception $e, $time)
  {
//    $this->message .= "Error in	".$test->getName()."\n";
//    $this->message .= "Error message: ".$e->getMessage()."\n";
  }


  public function addFailure(PHPUnit_Framework_Test $test, PHPUnit_Framework_AssertionFailedError $e, $time)
  {
//    $this->message .= "Failure in ".$test->getName()."\n";
//    $this->message .= "Error message:\n".$e->getMessage()."\n";
  }


  public function startTest(PHPUnit_Framework_Test $test)
  {
    $this->testCounter++;
    // Computes the arity of the test count
    $z = 1 + floor(log($this->suite->count(), 10));

    $testName = array_key_exists($test->getName(), $test->testMapping) ? $test->testMapping[$test->getName()] : $test->getName();
    $this->summaryLine = sprintf("\n% {$z}s / %0{$z}s  %-2s  %-60s  ", $this->testCounter, $this->suite->count(),
      $this->browserLogName, $testName);
  }


  public function endTest(PHPUnit_Framework_Test $test, $time)
  {
    if ($test->hasFailed()) {
      $this->summaryLine .= "FAILED";
    }
    else {
      $this->summaryLine .= "OK";
    }
    echo $this->summaryLine;
    $this->write($this->summaryLine);
  }


  public function addIncompleteTest(PHPUnit_Framework_Test $test, Exception $e, $time)
  {
  }


  /**
     * Skipped test.
     *
     * @param  PHPUnit_Framework_Test $test
     * @param  Exception              $e
     * @param  float                  $time
     * @access public
     * @since  Method available since Release 3.0.0
     */
  public function addSkippedTest(PHPUnit_Framework_Test $test, Exception $e, $time)
  {
  }


  /**
     * A test suite started.
     *
     * @param  PHPUnit_Framework_TestSuite $suite
     * @access public
     * @since  Method available since Release 2.2.0
     */
  public function startTestSuite(PHPUnit_Framework_TestSuite $suite)
  {
    $this->suite = $suite;
    // Hack
    if ($this->testCounter < 0)
    {
      $this->testCounter = 0;
      $this->write("CompletionSearch tests " . date("d-M-y H:i") . "\n");
    }
  }


  /**
     * A test suite ended.
     *
     * @param  PHPUnit_Framework_TestSuite $suite
     * @access public
     * @since  Method available since Release 2.2.0
     */
  public function endTestSuite(PHPUnit_Framework_TestSuite $suite)
  {
  }


  /**
     * @param  PHPUnit_Framework_TestResult $result
     * @access public
     */
  public function printResult(PHPUnit_Framework_TestResult $result)
  {
    $this->printHeader($result->time());
    $this->printErrors($result);
    $this->printFailures($result);
    //
    //        if ($this->verbose) {
    //            $this->printIncompletes($result);
    //            $this->printSkipped($result);
    //        }
//    echo "\nSummary:";
    $this->printFooter($result);
  }


  /**
     * @param  float   $timeElapsed
     * @access protected
     */
  protected function printHeader($timeElapsed)
  {
    $this->write("\n\nTime: " . PHPUnit_Util_Timer::secondsToTimeString($timeElapsed) . "\n\n");
  }


  /**
    * @param  PHPUnit_Framework_TestResult  $result
    * @access protected
    */
  protected function printFooter(PHPUnit_Framework_TestResult $result)
  {
    $okCount = count($result) - $result->errorCount() - $result->failureCount();

    echo "\n\n" . count($result) . " test" . (count($result) == 1 ? "" : "s") . ", "
      . $okCount . " ok, "
      . $result->failureCount() . " failed";

    if ($result->errorCount() > 1) {
      echo "\nThere are " . $result->errorCount() . " errors";
    }
    else if ($result->errorCount() == 1) {
      echo "\nThere is 1 error";
    }

    echo "\nSee file $this->logName for detailed information\n";


    // Now the log file
    $this->write("\n\n" . count($result) . " test" . (count($result) == 1 ? "" : "s") . ", "
      . $okCount . " ok, "
      . $result->failureCount() . " failed");

    if ($result->errorCount() > 1) {
      $this->write("\nThere are " . $result->errorCount() . " errors");
    }
    else if ($result->errorCount() == 1) {
      $this->write("\nThere is 1 error");
    }

    /*
    if ($result->wasSuccessful() &&
    $result->allCompletlyImplemented() && $result->noneSkipped()) {
      $this->write(
      sprintf(
      "\nOK (%d test%s)\n",

      count($result),
      (count($result) == 1) ? '' : 's'
      )
      );
    }

    else if ((!$result->allCompletlyImplemented() ||
    !$result->noneSkipped())&&
    $result->wasSuccessful()) {
      $this->write(
      sprintf(
      "\nOK, but incomplete or skipped tests!\n" .
      "Tests: %d%s%s.\n",

      count($result),
      $this->getCountString($result->notImplementedCount(), 'Incomplete'),
      $this->getCountString($result->skippedCount(), 'Skipped')
      )
      );
    }

    else {
      $this->write(
      sprintf(
      "\nFAILURES!\n" .
      "Tests: %d%s%s%s%s.\n",

      count($result),
      $this->getCountString($result->failureCount(), 'Failures'),
      $this->getCountString($result->errorCount(), 'Errors'),
      $this->getCountString($result->notImplementedCount(), 'Incomplete'),
      $this->getCountString($result->skippedCount(), 'Skipped')
      )
      );
    }
    */
  }


  /**
     * @param  array   $defects
     * @param  integer $count
     * @param  string  $type
     * @access protected
     */
  protected function printDefects(array $defects, $count, $type)
    {
        if ($count == 0) {
            return;
        }

        $this->write(
          sprintf(
            "There %s %d %s%s:\n",

            ($count == 1) ? 'was' : 'were',
            $count,
            $type,
            ($count == 1) ? '' : 's'
          )
        );

        $i = 1;

        foreach ($defects as $defect) {
            $this->printDefect($defect, $i++);
        }
    }


  /**
     * @param  PHPUnit_Framework_TestFailure $defect
     * @param  integer                       $count
     * @access protected
     */
  protected function printDefectHeader(PHPUnit_Framework_TestFailure $defect, $count)
  {
    $failedTest = $defect->failedTest();

    if ($failedTest instanceof PHPUnit_Framework_SelfDescribing) {
      $testName = $failedTest->toString();
    } else {
      $testName = get_class($failedTest);
    }

    $this->write(sprintf("\n%d) %s\n", $count, $testName));
  }


  /**
     * @param  PHPUnit_Framework_TestFailure $defect
     * @access protected
     */
  protected function printDefectTrace(PHPUnit_Framework_TestFailure $defect)
  {
    $e = $defect->thrownException();

    if ($e instanceof PHPUnit_Framework_SelfDescribing) {
      $buffer = $e->toString();

      if (!empty($buffer)) {
        $buffer .= "\n";
      }

      if ($e instanceof PHPUnit_Framework_ExpectationFailedException) {
        $comparisonFailure = $e->getComparisonFailure();

        if ($comparisonFailure !== NULL) {
          if ($comparisonFailure->identical()) {
            if ($comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_Object) {
              $buffer .= "Failed asserting that two variables reference the same object.\n";
            } else {
              $buffer .= $comparisonFailure->toString() . "\n";
            }
          } else {
            if ($comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_Scalar) {
              $buffer .= sprintf(
              "Failed asserting that %s matches expected value %s.\n",

              PHPUnit_Util_Type::toString($comparisonFailure->getActual()),
              PHPUnit_Util_Type::toString($comparisonFailure->getExpected())
              );
            }

            else if ($comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_Array ||
            $comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_Object ||
            $comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_String) {
              $buffer .= sprintf(
              "Failed asserting that two %ss are equal.\n%s\n",

              strtolower(substr(get_class($comparisonFailure), 36)),
              $comparisonFailure->toString()
              );
            }

            if ($this->verbose &&
            !$comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_Array &&
            !$comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_Object &&
            !$comparisonFailure instanceof PHPUnit_Framework_ComparisonFailure_String) {
              $buffer .= $comparisonFailure->toString() . "\n";
            }
          }
        } else {
          $buffer .= $e->getDescription() . "\n";
        }
      }
    }

    else if ($e instanceof PHPUnit_Framework_Error) {
      $buffer = $e->getMessage() . "\n";
    }

    else {
      $buffer = get_class($e) . ': ' . $e->getMessage() . "\n";
    }

    $this->write(
      $buffer .
      PHPUnit_Util_Filter::getFilteredStacktrace(
      $defect->thrownException(),
      FALSE
      )
    );
  }


  /**
     * @param  PHPUnit_Framework_TestResult  $result
     * @access protected
     */
  protected function printErrors(PHPUnit_Framework_TestResult $result)
  {
    $this->printDefects($result->errors(), $result->errorCount(), 'error');
  }


  /**
     * @param  PHPUnit_Framework_TestResult  $result
     * @access protected
     */
  protected function printFailures(PHPUnit_Framework_TestResult $result)
  {
    $this->printDefects($result->failures(), $result->failureCount(), 'failure');
  }


  /**
     * @param  PHPUnit_Framework_TestFailure $defect
     * @param  integer                       $count
     * @access protected
     */
  protected function printDefect(PHPUnit_Framework_TestFailure $defect, $count)
  {
    $this->printDefectHeader($defect, $count);
    $this->printDefectTrace($defect);
  }


  protected function write($text)
  {
    fwrite($this->fp, $text);
  }


  public function resetCounter()
  {
    $this->testCounter = 0;
  }


  public function setBrowserLogName($browserLogName)
  {
    $this->browserLogName = $browserLogName;
  }
}
?>