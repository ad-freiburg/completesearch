// Danke an selfhtml, siehe http://aktuell.de.selfhtml.org/artikel/javascript/utf8b64/utf8.htm
function utf8_encode(string)
{
	string = string.replace(/\r\n/g,"\n");
	var utftext = "";

	for (var n = 0; n < string.length; n++) {

		var c = string.charCodeAt(n);

		if (c < 128) {
			utftext += String.fromCharCode(c);
		}
		else if((c > 127) && (c < 2048)) {
			utftext += String.fromCharCode((c >> 6) | 192);
			utftext += String.fromCharCode((c & 63) | 128);
		}
		else {
			utftext += String.fromCharCode((c >> 12) | 224);
			utftext += String.fromCharCode(((c >> 6) & 63) | 128);
			utftext += String.fromCharCode((c & 63) | 128);
		}
	}
	return utftext;
}
    
    
function utf8_decode(utftext)
{
	var plaintext = ""; var i=0; var c=c1=c2=0;
	// while-Schleife, weil einige Zeichen uebersprungen werden
	while(i<utftext.length)
	{
		c = utftext.charCodeAt(i);
		if (c<128) {
			plaintext += String.fromCharCode(c);
			i++;}
			else if((c>191) && (c<224)) {
				c2 = utftext.charCodeAt(i+1);
				plaintext += String.fromCharCode(((c&31)<<6) | (c2&63));
				i+=2;}
				else {
					c2 = utftext.charCodeAt(i+1); c3 = utftext.charCodeAt(i+2);
					plaintext += String.fromCharCode(((c&15)<<12) | ((c2&63)<<6) | (c3&63));
					i+=3;}
	}
	return plaintext;
}


function getBrowser()
{
  var result = new Object();
  result.version = "0";
  
  switch (navigator.appName)
  {
    case 'Netscape': result.name = "firefox";
      if (/Firefox[\/\s](\d+\.\d+)/.test(navigator.userAgent)) {    
        result.version = new Number(RegExp.$1);
      }
      break;
    case 'Microsoft Internet Explorer': result.name = "ie";
      if (/MSIE (\d+\.\d+);/.test(navigator.userAgent)) { //test for MSIE x.x;
        result.version = new Number(RegExp.$1);
      }
      break;
    case 'Opera': result.name = "opera";
      break;
    default: result.name = "unknown";
  }
  
  return result;
}


function getCompatibilityMode()
{
  if (document.compatMode == 'CSS1Compat') return 'standard'; // standard mode
  if (document.compatMode == 'BackCompat') return 'quirk'; // quirks mode
}


function getStyle(element, style)
{
  // Computing padding needs distinguihing between IE and Firefox (Safari < 3 is not supported)
  if (element.currentStyle != undefined)
  {
    // IE
    var p = parseInt(element.currentStyle[style]);
  }
  else if (window.getComputedStyle != undefined)
  {
    // Firefox and Opera and Safari 3
    var p = parseInt(window.getComputedStyle(element,null)[style]);
  }
  else
  {
    var p = 0;
  }
  return p;
}


/**
 *  Returns the inner height of the browser window.
 *  Note: That can differ from the height of subordinated elements of the window (in case of scrollable elements, for example)
 */
function getWindowHeight() 
{
  if (typeof(window.innerHeight) == 'number') 
  {
    //Non-IE
    return window.innerHeight;
  } 
  else if (document.documentElement && document.documentElement.clientHeight) 
  {
    //IE 6+ in 'standards compliant mode'
    return document.documentElement.clientHeight;
  } 
  else if (document.body && document.body.clientHeight) 
  {
    //IE 4 compatible
    return document.body.clientHeight;
  }
}


/**


 *  Returns the current vertical position of the top of the visible part of window object.
 *  Note: In case of scrollable elements, this indicates how much the window is scrolled down.
 */
function getPageYOffset() 
{
  if (typeof(window.pageYOffset) == 'number') 
  {
    //Non-IE
    return window.pageYOffset;
  } 
  else if (document.documentElement && document.documentElement.scrollTop) 
  {
    //IE 6+ in 'standards compliant mode'
    return document.documentElement.scrollTop;
  } 
  else if (document.body && document.body.scrollTop) 
  {
    //IE 4 compatible
    return document.body.scrollTop;
  }
}


/**
 *  Returns the inner width of the browser window.
 *  Note: That can differ from the width of subordinated elements of the window (in case of scrollable elements, for example)
 */
function getWindowWidth() 
{
  if (typeof(window.innerWidth) == 'number') 
  {
    //Non-IE
    return window.innerWidth;
  } 
  else if (document.documentElement && document.documentElement.clientWidth) 
  {
    //IE 6+ in 'standards compliant mode'
    return document.documentElement.clientWidth;
  } 
  else if( document.body && document.body.clientWidth ) 
  {
    //IE 4 compatible
    return document.body.clientWidth;


  }
}


/**
  * Workarounds for the following Mozilla bug:
  * Form inputs in a DIV that overlaps a DIV with position "fixed" don't show the cursor.
  * In our case the query input is part of the DIV "left" which has z-index greater than the z-index of DIV "left_bg" 
  * ("left" overlaps "left_bg").
  */
function mozillaWorkaroundForAbsentCursorOnFocus()
{
	// NEW Markus / 11-01-09
  if (getBrowser().name == "firefox") {
//  if (getBrowser() == "firefox") {
    document.getElementById('left_bg').style.position='absolute';
  }
}


function mozillaWorkaroundForAbsentCursorOnBlur()
{
	// NEW Markus / 11-01-09
  if (getBrowser().name == "firefox") {
//  if (getBrowser() == "firefox") {
    document.getElementById('left_bg').style.position='fixed';
  }
}


function getTime()
{
	now = new Date();
	hour = now.getHours();
	min = now.getMinutes();
	sec = now.getSeconds();
	if (sec <= 9) { sec = "0" + sec; }
	if (min <= 9) { min = "0" + min; }
	if (hour <= 9) { hour = "0" + hour; }
	return hour + ":" + min + ":" + sec;
}


function objectAsString(object)
{
	var r = "{";
	for (q in object)
	{
		r += "'" + q + "' : '" + object[q] + "',";
	}
	return r.slice(0, r.length - 1) + "}";
}


function objectArrayAsString(objectArray)
{
	var r = "{";
	for(object in objectArray)
	{
		r += object + "->" + objectAsString(objectArray[object]) + ",";
	}
	return r != "{" ? r.slice(0, r.length - 1) : r + "}";
}


function info(text, level)
{
	if (typeof(level) == "undefined") {
		level = 1; // error
	}
    if (typeof(AC) !== "undefined" && typeof(AC.log) !== "undefined") {
		if (AC.log.level >= level) {
			if (typeof(console) !== "undefined") console.log(text);
		}
	}
}