/*
 * A pool for Ajax requests
 *
 * The only public method ist Ajax.send(...)
 *
 */


_Ajax = function()
{
	this.XMLHttpRequestContainers = new Array();
	// Saves a container as free (so we get instantly an free container with the next request without iterating over the whole container array)
	this.freeContainer = -1;	// index of a free container
	this.request_id = 0;
}



/**
 * Sends an Ajax request.
 *
 * This is the only public method.
 */
_Ajax.prototype.send = function(serverFile, parameter, callback, callbackOnError, callbackOnTimeout, timeout)
{
	var requestContainer = this.getFreeContainer();
  var xmlHttpRequest = requestContainer.XMLHttpRequest;

  requestContainer.callback = callback;

  if (typeof(callbackOnTimeout) !== "undefined" && typeof(timeout) !== "undefined")
//  if (callbackOnTimeout !== undefined && timeout !== undefined)
  {
	  requestContainer.callbackOnTimeout = callbackOnTimeout;
		requestContainer.timer = setTimeout("Ajax.stop(" + this.request_id + ")", timeout);
//		info("Timeout for request #" + this.request_id + " set to " + timeout, "ajax", 5);
  }

  if (callbackOnError !== undefined)
  {
	  requestContainer.callbackOnError = callbackOnError;
  }
//	info("#" + this.request_id + ": callback=" + callback);
  xmlHttpRequest.open( "POST", serverFile, true );
  xmlHttpRequest.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send query
  xmlHttpRequest.send("rid=" + this.request_id + "&" + parameter);
//  info("Sent #" + this.request_id + " / " + (typeof(callbackOnTimeout) == "undefined" ? "no timeout" : (timeout + "ms")) + " (" + serverFile + ")", "ajax", 4);
//  info("Sent #" + this.request_id + " / " + timeout + "ms (" + serverFile + ")", "ajax", 4);
}




/*
 * Private members *********************************************************************
 *
 */



/**
 * Create a XMLHttpRequest.
 */
_Ajax.prototype.createRequest = function ()
{
  var xmlHttp;
  // Microsoft Internet Explorer uses ActiveXObject
  if (window.ActiveXObject)
  {
    try
    {
      xmlHttp = new ActiveXObject("Msxml12.XMLHTTP");
    }
    catch (e)
    {
      try
      {
        xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
      }
      catch (e) {
				info("Could not create the ActiveXObject object", "ajax", 1);
      }
    }
  }
  // All other browsers use XMLHttpRequest
  else if (window.XMLHttpRequest)
  {
    try
    {
      xmlHttp = new XMLHttpRequest();
    }
    catch (e) {
			info("Could not create the XMLHttpRequest object", "ajax", 1);
    }
  }
  return xmlHttp;
}


/**
 * Stop the request with ID id
 */
_Ajax.prototype.stop = function(id)
{
//	info ("Stopping request #" + id, "ajax", 3);
	for (var i = 0; i < this.XMLHttpRequestContainers.length; i++)
	{
		if (this.XMLHttpRequestContainers[i].request_id == id)
		{
			// Clear the timer for this request
			clearTimeout(this.XMLHttpRequestContainers[i].timer);

			// Abort the running request
			this.XMLHttpRequestContainers[i].XMLHttpRequest.abort();

			// Set the current request ID to -1 (that means no running request)
			this.XMLHttpRequestContainers[i].request_id = -1;

			// Save this container as free (so we get instantly an free container with the next request without iterating over the whole container array)
			this.freeContainer = i;
			this.XMLHttpRequestContainers[i].callbackOnTimeout("timeout");
		}
	}
}


/**
 * Delivers an idle XMLHttpRequest object which can run a request.
 * First look whether the XMLHttpRequest container pool contains a idle object (which can run the request).
 * If there is not such an object add a new one to the pool and deliver this new one.
 * The type of the query is passed by parameter query_type and is stored in the container.
 */
_Ajax.prototype.getFreeContainer = function ()
{
	var freeContainer = null;

	// First case: the freeContainer which points to a free container has a valid index
	//
	if (this.freeContainer > -1)
	{
		freeContainer = this.XMLHttpRequestContainers[this.freeContainer];
		info (this.XMLHttpRequestContainers.length + " request object(s) are in pool, using free container #" + this.freeContainer, "ajax", 3);
		this.freeContainer = -1;
	}
	else
	{
		// Second case: we look for a free container in the array of all containers
		for (var i = 0; i < this.XMLHttpRequestContainers.length; i++)
		{
			// Look for an idle object (menas current request ID is -1)
			if (this.XMLHttpRequestContainers[i].request_id < 0)
			{
				// Currently no request is using this XMLHttpRequest object, object is idle
				freeContainer = this.XMLHttpRequestContainers[i];
//				info (this.XMLHttpRequestContainers.length + " request object(s) are in pool, using object #" + i, "ajax", 3);
				break;
			}
		}
	}

	// Third case: we have not got a free container in the both previous steps.
	// That means no one of the XMLHttpRequests is idle, so create a new one and put it into the array
	if (freeContainer == null)
	{
		freeContainer = new XMLHttpRequestContainer();

		freeContainer.XMLHttpRequest = this.createRequest();

		// Add the new conatiner to the array of all containers
		this.XMLHttpRequestContainers.push(freeContainer);

//		info ("New request object created for request #" + (this.request_id + 1), "ajax", 3);
//		info ("Now " + this.XMLHttpRequestContainers.length + " request object(s) are in pool", "ajax", 3);
	}

	// Now we have a free container: freeContainer
	// ATTENTION: to avoid problems use the abort() function.
	// NOTE: Even when abort()is not used the callback function has to be set in every of the three cases, because when an ajax request returns its onreadystatechange property is set to null
	freeContainer.XMLHttpRequest.abort();
	freeContainer.XMLHttpRequest.onreadystatechange = _Ajax_callback;

	freeContainer.request_id = ++this.request_id;
	freeContainer.callbackOnTimeout = defaultCallbackOnTimeout;
	freeContainer.callbackOnError = defaultCallbackOnError;
	freeContainer.timer = null;
	freeContainer.timeout = 0;

	return freeContainer;
}



/**
 * Container for the XMLHttpRequest objects
 *
 */
XMLHttpRequestContainer = function ()
{
	this.request_id = -1;	// -1 means currently no request running
	this.XMLHttpRequest = null;
	this.callback = null;
	this.callbackOnTimeout = null;
	this.timer = null;
}


XMLHttpRequestContainer.prototype.toString = function ()
{
	return "request #" + this.request_id + ", callback: " + this.callback;
}


/**
 * Global
 *
 */
var Ajax = new _Ajax();


/**
 * Die generische Callback-Funktion.
 *
 * Sie mu� global definiert werden (als Member von _Ajax klappt's nicht).
 *
 */
_Ajax_callback = function()
{
	// Check all request containers whether they are running and have finished their request
	for (var i = 0; i < Ajax.XMLHttpRequestContainers.length; i++)
	{
//		info (i + ": " + Ajax.XMLHttpRequestContainers[i].request_id + ", " + Ajax.XMLHttpRequestContainers[i].XMLHttpRequest.readyState, "ajax", 5);
		// Test whether this XMLHttpRequest has a running query
		if (Ajax.XMLHttpRequestContainers[i].request_id > -1)
		{
			var xmlHttpRequest = Ajax.XMLHttpRequestContainers[i].XMLHttpRequest;

			try
			{
				if (xmlHttpRequest.readyState == 4)
				{
						// Request returned, return code is ok
						if (typeof(xmlHttpRequest.status) !== "undefined" && xmlHttpRequest.status == 200)
						{
							// Clear the timer as soon as possible
							clearTimeout(Ajax.XMLHttpRequestContainers[i].timer);
							var container_request_id = Ajax.XMLHttpRequestContainers[i].request_id;
							// It's very important to set the request_id already here (before the callback is executed)
							Ajax.XMLHttpRequestContainers[i].request_id = -1;
							// Neu 20-06-08 (Markus): Da ob_clean() auf PHP-Seite nicht auf allen Plattformen funktioniert (z.B. auf meinem lokalen Windows XP),
							//	m�ssen m�gliche Leerzeichen hier entfernt werden
							var r = trim(xmlHttpRequest.responseText);
//							info("r: " + r, "ajax", 5);
				      var rp = r.split('</rid>');
//							info("rp: " + rp, "ajax", 5);
				      var request_id = parseInt(rp[0].substring(5));
//				      info("rid: " + request_id, "ajax", 5);
							if (request_id == container_request_id)
							{
//								info("Callback #" + request_id + ": " + rp[1] + ", callback=" + Ajax.XMLHttpRequestContainers[i].callback, "ajax", 5);
								Ajax.XMLHttpRequestContainers[i].callback(rp[1]);
							}
							else {
								info("Callback failed: id #" + Ajax.XMLHttpRequestContainers[i].request_id + " of finished container don't match #" + request_id + " of returned script", "ajax", 2);
								Ajax.XMLHttpRequestContainers[i].callbackOnTimeout(rp[1]);
							}
							// Request gefunden und Callback ausgef�hrt, also verlassen wir nun die Schleife
							break;
						}
						else
						{
							// Return code of web server is not ok
							info("Callback failed for request #: " + request_id + " [HTTP error " + xmlHttpRequest.status + "]", "ajax", 2);
							Ajax.XMLHttpRequestContainers[i].callbackOnError(rp[1]);
						}
				}
			}
			catch (e)
			{
//				info("Error in callback : " + xmlHttpRequest.responseText + " [" + e + "]", "ajax", 1);
//				return;
				continue;
			}
		}
	}

//	if (i < Ajax.XMLHttpRequestContainers.length) {
//		info("Ok, corresponding container found");
//	} else {
//		info("No corresponding container found (probably because of a request timeout)");
//	}
}


function defaultCallbackOnTimeout(response)
{
	info("Default timeout callback (no own timeout callback defined): " + response, "ajax", 4);
}


function defaultCallbackOnError(response)
{
	info("Got error from server (this message is generated by the default error callback; you can define an own error callback function): " + response, "ajax", 4);
}


/**
 * Some help functions
 */
function info(text, module, level)
{
	if (typeof(level) == "undefined") {
		level = 1; // error
	}
	if (typeof(module) == "undefined") {
		module = "all"; // all modules
	}
	if (typeof(console) !== "undefined") {
		console.log(text);
	}
}


/**
 *  Javascript trim, ltrim, rtrim
 *  http://www.webtoolkit.info/
 */

function trim(str, chars)
{
    return ltrim(rtrim(str, chars), chars);
}

function ltrim(str, chars)
{
    chars = chars || "\\s";
    return str.replace(new RegExp("^[" + chars + "]+", "g"), "");
}

function rtrim(str, chars)
{
    chars = chars || "\\s";
    return str.replace(new RegExp("[" + chars + "]+$", "g"), "");
}

//console.log("ajaxPool.js geladen");