/**
 * Format for Selenium Remote Control PHP client using Bromine.
 * Thanks to Sonic for the original source.
 */

load('remoteControl.js');

this.name = "php-rc-bromine";

function testMethodName(testName) {
    return "test" + capitalize(testName);
}

function assertTrue(expression) {
    return "if(" + expression.toString() + " != true){throw new Exception('fejl 40');}";
}

function assertFalse(expression) {
    return "if(" + expression.toString() + " == true){throw new Exception('fejl 40');}";
}

function verify(statement) {
    return statement;
}

function verifyTrue(expression) {
    return verify(assertTrue(expression));
}

function verifyFalse(expression) {
    return verify(assertFalse(expression));
}

function assignToVariable(type, variable, expression) {
    return variable + " = " + expression.toString();
}
function waitFor(expression) {
    return "for ($second = 0; ; $second++) {\n" +
        indent(1) + 'if ($second >= 60) $this->fail("timeout");\n' +
        indent(1) + "try {\n" +
        indent(2) + (expression.setup ? expression.setup() + " " : "") +
        indent(2) + "if (" + expression.toString() + ") break;\n" +
        indent(1) + "} catch (Exception $e) {}\n" +
        indent(1) + "sleep(1);\n" +
        "}\n";
}

function assertOrVerifyFailure(line, isAssert) {
    var message = '"expected failure"';
    var failStatement = isAssert ? "$this->fail(" + message  + ");" :
        "array_push($this->verificationErrors, " + message + ");";
    return "try { \n" +
        line + "\n" +
        failStatement + "\n" +
        "} catch (Exception $e) {}\n";
}

Equals.prototype.toString = function() {
    return this.e1.toString() + " == " + this.e2.toString();
}

Equals.prototype.assert = function() {
    return "if(" + this.e1.toString() + " != " + this.e2.toString() + "){throw new Exception('fejl 40');}";
}

Equals.prototype.verify = function() {
    return verify(this.assert());
}

NotEquals.prototype.toString = function() {
    return this.e1.toString() + " != " + this.e2.toString();
}

NotEquals.prototype.assert = function() {
    return "if(" + this.e1.toString() + " == " + this.e2.toString() + "){throw new Exception('fejl 40');}";
}

NotEquals.prototype.verify = function() {
    return verify(this.assert());
}

RegexpMatch.prototype.toString = function() {
  return "(bool)preg_match(/" + this.pattern.replace(/\//g, "\\/") + "/," + this.expression + ");";
}

RegexpNotMatch.prototype.toString = function() {
    return "(bool)preg_match(/" + this.pattern.replace(/\//g, "\\/") + "/," + this.expression + ");";
}

EqualsArray.prototype.length = function() {
    return this.variableName + ".length";
}

EqualsArray.prototype.item = function(index) {
    return this.variableName + "[" + index + "]";
}

function pause(milliseconds) {
    return "sleep(" + (parseInt(milliseconds) / 1000) + ");";
}

function echo(message) {
    return "print(" + xlateArgument(message) + ' . "\\n");'
}

function statement(expression) {
    return expression.toString() + ';';
}

function array(value) {
    var str = 'array(';
    for (var i = 0; i < value.length; i++) {
        str += string(value[i]);
        if (i < value.length - 1) str += ", ";
    }
    str += ')';
    return str;
}

CallSelenium.prototype.toString = function() {
    var result = '';
    if (this.negative) {
        result += '!';
    }
    if (options.receiver) {
        result += options.receiver + '->';
    }
    result += this.message;
    result += '(';
    for (var i = 0; i < this.args.length; i++) {
        result += this.args[i];
        if (i < this.args.length - 1) {
            result += ', ';
        }
    }
    result += ')';
    return result;
}

function formatComment(comment) {
    return comment.comment.replace(/.+/mg, function(str) {
            return "// " + str;
        });
}

this.options = {
    receiver: "$this->selenium",
    header: 
        '<?php\n' +
        '\n' +
        'set_include_path(get_include_path() . PATH_SEPARATOR . "RC/Drivers/rc-php");\n' +
        'require_once "Testing/Selenium.php";\n' +
        '\n' +
        'class BromineTest\n' +
        '{\n' +
        '  function setUp($host, $port, $browser, $sitetotest, $u_id, $t_id)\n' +
        '  {\n' +
        '    $this->u_id = $u_id;\n' +
        '    $this->t_id = $t_id;\n' +
        '    $this->selenium = new Testing_Selenium($browser, $sitetotest, $host ,$port);\n' +
        '    $result = $this->selenium->start();\n' +
        '  }\n' +
        '\n' +
        '  function tearDown()\n' +
        '  {\n' +
        '    $this->selenium->stop();\n' +
        '  }\n' +
        '  function testMyTestCase()\n' +
        '  {\n' +
        '    try{\n',

    footer:
        '    }\n' +
        '    catch(Exception $e){}\n' +
        '  }\n' +
        '  function customCommand($cmdName, $status, $var1, $var2) \n' +
        '  {\n' + 
        '    $url =  "http://127.0.0.1/selenium-server/driver/index.php?cmd=customCommand&cmdName=$cmdName&status=$status&var1=$var1&var2=$var2&t_id=$this->t_id&u_id=$this->u_id";\n' +
        '    $url=str_replace(" ","%20",$url);\n' +
        '    if($h = fopen($url, "r")){\n'+
        '      fclose($h);\n' + 
        '    }\n' + 
        '  }\n' + 
        '}\n\n\n' +
    '$host = $argv[1];\n'+
    '$port = $argv[2];\n'+
    '$browser = $argv[3];\n'+
    '$sitetotest = $argv[4];\n'+
    '$u_id = $argv[5];\n'+
    '$t_id = $argv[6];\n'+
    '$brows2 = $browser.",".$u_id;\n'+
    '$datafile = $argv[7];\n'+
    
    '$t = new BromineTest();\n'+
    '$t->setUp($host, $port, $brows2, $sitetotest, $u_id, $t_id);\n'+
    '$t->testMyTestCase();\n'+
    '$t->tearDown();\n'+
        "?>",
    indent: "3",
    initialIndents: '3'
};


this.configForm = 
    '<description>Variable for Selenium instance</description>' +
    '<textbox id="options_receiver" />';

