/*
 * Format for Selenium Remote Control Java client.
 */

load('remoteControl.js');

this.name = "java-rc";

function useSeparateEqualsForArray() {
	return true;
}

function testMethodName(testName) {
	return "test" + capitalize(testName);
}

function assertTrue(expression) {
	return "assertTrue(" + expression.toString() + ");";
}

function verifyTrue(expression) {
	return "verifyTrue(" + expression.toString() + ");";
}

function assertFalse(expression) {
	return "assertFalse(" + expression.toString() + ");";
}

function verifyFalse(expression) {
	return "verifyFalse(" + expression.toString() + ");";
}

function assignToVariable(type, variable, expression) {
	return type + " " + variable + " = " + expression.toString();
}

function ifCondition(expression, callback) {
    return "if (" + expression.toString() + ") {\n" + callback() + "}";
}

function waitFor(expression) {
	return "for (int second = 0;; second++) {\n" +
		"\tif (second >= 60) fail(\"timeout\");\n" +
		"\ttry { " + (expression.setup ? expression.setup() + " " : "") +
		"if (" + expression.toString() + ") break; } catch (Exception e) {}\n" +
		"\tThread.sleep(1000);\n" +
		"}\n";
	//return "while (" + not(expression).toString() + ") { Thread.sleep(1000); }";
}

function assertOrVerifyFailure(line, isAssert) {
	var message = '"expected failure"';
    var failStatement = "fail(" + message + ");";
	return "try { " + line + " " + failStatement + " } catch (Throwable e) {}";
}

Equals.prototype.toString = function() {
    if (this.e1.toString().match(/^\d+$/)) {
        // int
	    return this.e1.toString() + " == " + this.e2.toString();
    } else {
        // string
	    return this.e1.toString() + ".equals(" + this.e2.toString() + ")";
    }
}

Equals.prototype.assert = function() {
	return "assertEquals(" + this.e1.toString() + ", " + this.e2.toString() + ");";
}

Equals.prototype.verify = function() {
	return "verifyEquals(" + this.e1.toString() + ", " + this.e2.toString() + ");";
}

NotEquals.prototype.toString = function() {
	return "!" + this.e1.toString() + ".equals(" + this.e2.toString() + ")";
}

NotEquals.prototype.assert = function() {
	return "assertNotEquals(" + this.e1.toString() + ", " + this.e2.toString() + ");";
}

NotEquals.prototype.verify = function() {
	return "verifyNotEquals(" + this.e1.toString() + ", " + this.e2.toString() + ");";
}

RegexpMatch.prototype.toString = function() {
	if (this.pattern.match(/^\^/) && this.pattern.match(/\$$/)) {
		return this.expression + ".matches(" + string(this.pattern) + ")";
	} else {
		return "Pattern.compile(" + string(this.pattern) + ").matcher(" + this.expression + ").find()";
	}
}

EqualsArray.prototype.length = function() {
	return this.variableName + ".length";
}

EqualsArray.prototype.item = function(index) {
	return this.variableName + "[" + index + "]";
}

function pause(milliseconds) {
	return "Thread.sleep(" + parseInt(milliseconds) + ");";
}

function echo(message) {
	return "System.out.println(" + xlateArgument(message) + ");";
}

function statement(expression) {
	return expression.toString() + ';';
}

function array(value) {
	var str = 'new String[] {';
	for (var i = 0; i < value.length; i++) {
		str += string(value[i]);
		if (i < value.length - 1) str += ", ";
	}
	str += '}';
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
	receiver: "selenium",
	packageName: "com.example.tests",
	superClass: "SeleneseTestCase",
    indent:	'tab',
    initialIndents:	'2'
};

options.header =
	"import com.thoughtworks.selenium.*;\n" +
	"import java.util.regex.Pattern;\n" +
	"\n" +
    "public class ${className} extends ${superClass} {\n" + 
    "\tprivate static String t_id;\n" +
    "\tprivate static String u_id;\n" +
    "\tprivate static String datafile;\n" +
    "\tprivate CustomCommand customCommand;\n\n" +
    "\tpublic void setUp(String host, int port, String brows, String sitetotest) throws Exception {\n" +
    '\tselenium = new DefaultSelenium(host, port, brows, sitetotest);\n' +
    "\tselenium.start();\n" + 
    "\tcustomCommand = new CustomCommand(t_id, u_id);\n" + 
    "\t}\n\n" +
    "\tpublic void tearDown() throws Exception {\n" +
    "\t\tselenium.stop();\n" + 
    "\t}\n" +
    "\tpublic void ${methodName}() throws Exception {\n"
    

    
                
    

options.footer =
    "\t}\n" + 
    '\tpublic static void main(String[] args) {\n' + 
    '\t    try{\n' + 
    '\t        String host = args[0];\n' + 
    '\t        int port = Integer.parseInt(args[1]);\n' + 
    '\t        String brows = args[2];\n' + 
    '\t        String sitetotest = args[3];\n' + 
    '\t        u_id = args[4];\n' + 
    '\t        t_id = args[5];\n' + 
    '\t        if(args.length == 7){\n' + 
    '\t            datafile = args[6];\n' + 
    '\t        }\n' + 
    '\t        String brows2 = brows+","+u_id;\n' + 
    "\t        NewTest t = new NewTest ();\n" + 
    '\t        t.setUp(host, port, brows2, sitetotest);\n' + 
    '\t        t.testNew();\n' + 
    '\t        t.tearDown();\n' + 
    '\t    }\n' + 
    '\t    catch(Exception e){\n' + 
    '\t    }\n' + 
    '\t}\n' + 

	"\t}\n"

this.configForm = 
	'<description>Variable for Selenium instance</description>' +
	'<textbox id="options_receiver" />';

