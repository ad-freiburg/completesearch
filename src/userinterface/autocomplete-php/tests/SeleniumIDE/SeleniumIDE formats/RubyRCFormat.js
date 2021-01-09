/*
 * Format for Selenium Remote Control Ruby client using Bromine
 */

load('remoteControl.js');

this.name = "ruby-rc";

function testMethodName(testName) {
	return "test_" + underscore(testName);
}

function assertTrue(expression) {
	return "assert " + expression.toString();
}

function assertFalse(expression) {
	return "assert " + expression.invert().toString();
}

function verify(statement) {
	return "begin\n" +
		indent(1) + statement + "\n" +
		"rescue Test::Unit::AssertionFailedError\n" +
		indent(1) + "@verification_errors << $!\n" + 
		"end";
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
	if (expression.negative) {
		return "assert !60.times{ break unless (" + expression.invert().toString() + " rescue true); sleep 1 }"
	} else {
		return "assert !60.times{ break if (" + expression.toString() + " rescue false); sleep 1 }"
	}
}

function assertOrVerifyFailure(line, isAssert) {
	return "assert_raise(Kernel) { " + line + "}";
}

Equals.prototype.toString = function() {
	return this.e1.toString() + " == " + this.e2.toString();
}

Equals.prototype.assert = function() {
	return "assert_equal " + this.e1.toString() + ", " + this.e2.toString();
}

Equals.prototype.verify = function() {
	return verify(this.assert());
}

NotEquals.prototype.toString = function() {
	return this.e1.toString() + " != " + this.e2.toString();
}

NotEquals.prototype.assert = function() {
	return "assert_not_equal " + this.e1.toString() + ", " + this.e2.toString();
}

NotEquals.prototype.verify = function() {
	return verify(this.assert());
}

RegexpMatch.prototype.toString = function() {
	return "/" + this.pattern.replace(/\//g, "\\/") + "/ =~ " + this.expression;
}

RegexpNotMatch.prototype.toString = function() {
	return "/" + this.pattern.replace(/\//g, "\\/") + "/ !~ " + this.expression;
}

EqualsArray.useUniqueVariableForAssertion = false;

EqualsArray.prototype.length = function() {
	return this.variableName + ".size";
}

EqualsArray.prototype.item = function(index) {
	return this.variableName + "[" + index + "]";
}

function pause(milliseconds) {
	return "sleep " + (parseInt(milliseconds) / 1000);
}

function echo(message) {
	return "p " + xlateArgument(message);
}

function statement(expression) {
	expression.noBraces = true;
	return expression.toString();
}

function array(value) {
	var str = '[';
	for (var i = 0; i < value.length; i++) {
		str += string(value[i]);
		if (i < value.length - 1) str += ", ";
	}
	str += ']';
	return str;
}

CallSelenium.prototype.toString = function() {
	var result = '';
	if (this.negative) {
		result += '!';
	}
	if (options.receiver) {
		result += options.receiver + '.';
	}
	result += underscore(this.message);
	if (!this.noBraces && this.args.length > 0) {
		result += '(';
	} else if (this.args.length > 0) {
		result += ' ';
	}
	for (var i = 0; i < this.args.length; i++) {
		result += this.args[i];
		if (i < this.args.length - 1) {
			result += ', ';
		}
	}
	if (!this.noBraces && this.args.length > 0) {
		result += ')';
	}
	return result;
}

function formatComment(comment) {
	return comment.comment.replace(/.+/mg, function(str) {
			return "# " + str;
		});
}

this.options = {
	receiver: "@selenium",
	header: 
		'require "RC/Drivers/rc-ruby/selenium"\n' +
		'require "test/unit"\n' +
		'require "pp"\n' +
		'require "open-uri"\n' +
		'require "cgi"\n' +
		'$host = ARGV[0];\n' +
		'$port = ARGV[1];\n' +
		'$browser = ARGV[2];\n' +
		'$sitetotest = ARGV[3];\n' +
		'$u_id = ARGV[4];\n' +
		'$t_id = ARGV[5];\n' +
		'$datafile = ARGV[6];\n' +
		'\n' +
		'class ${className} < Test::Unit::TestCase\n' +
		'  def setup\n' +
		'    @verification_errors = []\n' +
		'    if $selenium\n' +
		'      @selenium = $selenium\n' +
		'    else\n' +
		'      brows2 = $browser+","+$u_id\n' +
                '     @selenium = Selenium::SeleniumDriver.new($host, $port,brows2, $sitetotest);\n' +
		'      @selenium.start\n' +
		'    end\n' +
		'    @selenium.set_context("${methodName}")\n' +
		'  end\n' +
		'  \n' +
		'  def teardown\n' +
		'    @selenium.stop unless $selenium\n' +
		'    assert_equal [], @verification_errors\n' +
		'  end\n' +
		'  \n' +
		'  def ${methodName}\n',
	footer:
		"  end\n" +
                'def custom_command(cmdName, status, var1, var2) \n' +
                '    url = "http://127.0.0.1/selenium-server/driver/index.php?cmd=customCommand&cmdName="+CGI.escape(cmdName)+"&status="+CGI.escape(status)+"&var1="+CGI.escape(var1)+"&var2="+CGI.escape(var2)+"&t_id="+CGI.escape($t_id)+"&u_id="+CGI.escape($u_id)\n' +
                '     Kernel.open(url) do |f|\n' +
                '        p f.read\n' +
                '     end\n' +
                'end\n' +
		"end\n",
	indent: "2",
	initialIndents: "2"
};

this.configForm = 
	'<description>Variable for Selenium instance</description>' +
	'<textbox id="options_receiver" />' +
	'<description>Header</description>' +
	'<textbox id="options_header" multiline="true" flex="1" rows="4"/>' +
	'<description>Footer</description>' +
	'<textbox id="options_footer" multiline="true" flex="1" rows="4"/>' +
	'<description>Indent</description>' +
	'<menulist id="options_indent"><menupopup>' +
	'<menuitem label="Tab" value="tab"/>' +
	'<menuitem label="1 space" value="1"/>' +
	'<menuitem label="2 spaces" value="2"/>' +
	'<menuitem label="3 spaces" value="3"/>' +
	'<menuitem label="4 spaces" value="4"/>' +
	'<menuitem label="5 spaces" value="5"/>' +
	'<menuitem label="6 spaces" value="6"/>' +
	'<menuitem label="7 spaces" value="7"/>' +
	'<menuitem label="8 spaces" value="8"/>' +
	'</menupopup></menulist>';

