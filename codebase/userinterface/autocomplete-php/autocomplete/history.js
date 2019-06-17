// NEW 14-07-08 (Markus): when creating the hash for IE (by creating a hidden iframe) we have to prevent the onFrameLoaded method of this iframe
//	from calling the query (which called create_hash() itself) again. So we use the variable ie_block and set it in create_hash() to true.
//	In onFrameLoaded() a value of true of ie_block prevent calling parse_parameters(hash).
var ie_block = false;

var last_hash = "";



/**
 *	Create the URL hash accordingly to the current queries (delievered by getQueryParametersAsString())
 *
 */
function create_hash()
{
	hash = "query=" + AC.getQueryInput() + "&qp=" + AC.getQueryParametersAsString();
	notes.add("create_hash: " + hash, 0);
	notes.add("Browser: " + getBrowser().name + getBrowser().version, 1);
	// NEW Markus / 11-01-09
	if (getBrowser().name == "ie" && getBrowser().version < 8)
//	if (getBrowser() == "ie")
	{
		// Set ie_block = true to prevent onFrameLoaded() from calling parse_parameters(hash)
		ie_block = true;
		var doc = document.getElementById("historyFrame").contentWindow.document;
		if (doc)
		{
			doc.open("javascript:'<html></html>'");
			doc.write("<html><head><script type=\"text/javascript\">parent.onFrameLoaded('" + hash + "');<\/script></head><body></body></html>");
			doc.close();
			notes.add("IE history, wrote to iframe: " + hash, 1);
		}
		else notes.add("<b>Error: IE history frame don't exist</b>", 0);
	}
	else {
    if (location.search != "" && history.pushState)
    {
      var tmphash = location.search.substr(1);
      var newUrl = location.protocol + "//" + location.hostname + location.pathname;
      history.pushState({}, "", newUrl)
      location.hash = tmphash;
		  last_hash = location.hash;
    }
    location.hash = hash;
		// Markus / 28-10-08: It's essential that last_hast begins with a '#', therefore set it to hash is not right
//		last_hash = hash;
		last_hash = location.hash;
		notes.add("non-IE history, append hash to url: " + hash, 1);
	}
}


/**
 *	Parse an URL hash and launch a corresponding query
 *
 */
function parse_parameters(hash)
{
	var get = new Array();
	var qp = new Array();
	get['query'] = "";
	get['qp'] = "";

	if (hash.charAt(0) == "#") {
		hash = hash.substring(1);
	}

	// NEW 08-07-08 (Markus): now the query parameters are set by default. So in case of empty hash we have a query with default parameters, 
	//	and in case of non-empty hash we get a query with parameters specified by the hash and the remaining parameter are set by default.
	//	This is important we come from facetboxes.php, for example, where the completion links define only those parameters which differs from defaults
	qp = AC.getDefaultQueryParameters();
	
	if (hash !== "")
	{
		// Query not empty to begin to parse
		parameters = hash.split('&');

		for (var i = 0; i < parameters.length; i++)
		{
			av = parameters[i].split('=');
			if (av != undefined) {
				var key = av[0];
				get[av[0]] = av[1];
//				info(av[0] + " = " + av[1]);
			}
		}

		parameters = get['qp'].split(':');

		for (var i = 0; i < parameters.length; i++)
		{
//			info(parameters[i]);

			r = parameters[i].match(/([HWFCY])(\d+)\.(\d+)\.?(\d*)\.?(\d*)/);
			if (r != null)
			{
				if (r[4] == "") r[4] = 1;
				if (r[5] == "") r[5] = r[4];
				//			info ("fh=" + r[4] + ", fhs=" + r[5]);
				qp[r[1] + r[2]] = new Query(r[1], r[2], r[3], r[4], r[5]);
//				qp.push(new Query(r[1], r[2], r[3], r[4], r[5]));
			}
		}
	}

	// Set the query input to the query value contained in the hash; note that we must url decode (unescape) the value
	AC.setQueryInput(unescape(get['query']));

	AC.box_navigation_mode = "R";

	notes.add("Parameters parsed, launching query: " + get['query'], 0);
	AC.launchQuery(get['query'], qp, true, "history");
}


// This function is used by non-IE browsers to check whether hash is changed
function check_hash()
{
//	info("Browser: " + getBrowser().name + getBrowser().version, 1);

	if (last_hash != location.hash)
	{
//		info("check_hash: " + last_hash + " <--> " + location.hash + "  " + AC.result.navigation_mode);
		last_hash = location.hash;
		// The following if condition is no longer necessary (instead it is false to use it!) 
		//  because in create_hash() now the last_hash is set to location.hash after a user interaction
		//  and so we only will arrive here after a browser navigation with back / forth 
		//  or when coming from an external link or browser bookmark
		// TODO 03-01-09: das da oben scheint falsch zu sein, denn es wird dann eine HWF-Anfrage abgeschickt
		// 06-01-09: scheint doch richtig? ;-)
//		if (AC.result.navigation_mode == "history")
		{
			notes.add("<b>User navigated via back/forward</b>", 0);
			notes.add("Hash changed, re-compute all boxes ...", 1);
//			info("Hash changed, re-compute all boxes ...");
			parse_parameters(location.hash);
		}
//		else {
//			AC.result.navigation_mode = "history";
//		}
	}
}


// This function is used by internet explorer's iframe onload event
// When navigating with IE's back/forward buttons the browser steps through the iframes saved in its history.
// Each of this saved iframes represents a history state of user interaction.
function onFrameLoaded(hash)
{
	hash = hash.replace(/&amp;/, "&");
	if (last_hash != hash)
	{
		notes.add("IE: hash changed to " + hash, 1);
		location.hash = hash;
		last_hash = hash;
		
		// If ie_block is true the iframe is loaded why set by the navigation mechanismen, not by a user history navigation.
		//	So we must not call parse_parameters()
		if (ie_block) {
			ie_block = false;
		}
		else parse_parameters(hash);
	}
}
