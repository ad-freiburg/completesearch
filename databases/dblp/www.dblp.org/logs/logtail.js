/* an ajax log file tailer / viewer
copyright 2007 john minnihan.
 
http://freepository.com
 
Released under these terms
1. This script, associated functions and HTML code ("the code") may be used by you ("the recipient") for any purpose.
2. This code may be modified in any way deemed useful by the recipient.
3. This code may be used in derivative works of any kind, anywhere, by the recipient.
4. Your use of the code indicates your acceptance of these terms.
5. This notice must be kept intact with any use of the code to provide attribution.
*/

function init()
{
  initLog('start', '/var/log/dblp/dblp.log', '50', 'dblp');
  initLog('start', '/var/log/dblp/access.log', '50', 'access');
  initLog('start', '/var/log/apache2/dblp-access.log', '50', 'dblp-access');
}

function initLog(timer, path, lines, div)
{
  var request = createRequest(); 
  requests[div] = request;
  numLines[div] = '0';
  getLog(timer, path, lines, div, position);
  //position++;
}
 
function getLog(timer, path, lines, div, pos) {
var url = "http://www.dblp.org/logs/logtail.php?path=" + path + "&lines=" + numLines[div]; ///var/log/dblp/dblp.log.1&lines=20";
requests[div].open("GET", url, true);
requests[div].send(null);
requests[div].onreadystatechange = function() { updatePage(div, pos); };
//alert(requests.length + " " + numLines.length);
startTail(timer, path, lines, div, pos);
}
 
function startTail(timer, path, lines, div, pos) {
if (timer == "stop") {
stopTail(div, pos);
} else {
t= setTimeout('var path = "' + path + 
              '"; var lines = "' + lines + 
              '"; var timer = "' + timer +
              '"; var div = "' + div +
              '"; var pos = "' + pos +
              '"; getLog(timer, path, lines, div, pos)', 400);
}
}
 
function stopTail(div, pos) {
clearTimeout(t);
numLines[div] = '0';
//var pause = "The log viewer has been paused. To begin viewing again, click the Start Viewer button.\n";
//logDiv = document.getElementById(div);
//var newNode=document.createTextNode(pause);
//logDiv.replaceChild(newNode,logDiv.childNodes[0]);
}
 
function updatePage(div, pos) {
if (requests[div].readyState == 4) {
if (requests[div].status == 200) {
var logValue = requests[div].responseText;
//logValue.replace("\t", "XXXXX");
var currentLogValue = logValue.split("\n");
eval(currentLogValue);
logDiv = document.getElementById(div);
numLines[div] = currentLogValue[0];
var maxLength = 4000;
var logline = '';

//logDiv.innerHTML = "<pre>";
for (i=2; i < currentLogValue.length - 1; i++) {
  logline += currentLogValue[i] + "<br/>\n";
  if (logDiv.innerHTML.length + logline.length >= maxLength)
  {
    logDiv.innerHTML = logDiv.innerHTML.substr(logDiv.innerHTML.length - maxLength - logValue.length);
  }
}
logDiv.innerHTML += logline;
if (logDiv.innerHTML.length <= 1) logDiv.innerHTML = currentLogValue[1] + "<br/>\n";
logDiv.scrollTop = logDiv.scrollHeight;
}
//else alert("Error! Request status is " + requests[div].status);
}
}

function createRequest() {
 var request = null;
  try {
   request = new XMLHttpRequest();
  } catch (trymicrosoft) {
   try {
     request = new ActiveXObject("Msxml2.XMLHTTP");
   } catch (othermicrosoft) {
     try {
      request = new ActiveXObject("Microsoft.XMLHTTP");
     } catch (failed) {
       request = null;
     }
   }
 }
 
 if (request == null) {
   alert("Error creating request object!");
 } else {
   return request;
 }
}

var position = 0;
var requests = new Array();
var numLines = new Array();
