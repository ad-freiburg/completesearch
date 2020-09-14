/*
	The complete javascript code for the autocompletion functionality.
	Notice: this is used as a namespace for the complete code. It is created in autocomplete.php.
*/
var keyevent_counter = 0;
var mouseTileL = false;
var mouseTileR = false;

var notes = new Notes();


/*
	Class Query
*/
Query = function ()
{
	// Public members
	// Note: it's important to set the following values initially
	this.type = "";
	this.index = 1;
	this.count = 0;
	this.first_hit = 1;
	this.first_hit_shown = 1;
}


/*
	Class Query: Constructor
*/
Query = function (type, index, count, first_hit, first_hit_shown)
{
	// Public members
	// Note: it's important to set the following values initially
	this.type = type;
	this.index = index;
//	if (typeof(count) == "undefined") {
	if (count == null) {
		this.count = 0;
	}
	else {
		this.count = count;
	}
	// first_hit is optional
	if (first_hit != null && first_hit != "") {
	  this.first_hit = first_hit;
	}
	else {
  	this.first_hit = 1;
	}
	// first_hit_shown is optional
	if (first_hit_shown != null && first_hit_shown != "") {
	  this.first_hit_shown = first_hit_shown;
	}
	else {
  	this.first_hit_shown = 1;
	}
}


/*
	Class Result
*/
Result = function ()
{
	// Public members
	// Note: it's important to set the following values initially
	this.subtitle = "";
	this.completions = new Array();
	this.completions_total = "";
	this.completions_computed = "";
	this.completions_sent = "";
	this.hits_count_per_completion = "";
	this.hit_title_box = "";
	// NEW 20-08-08 / Markus: very important: the boxes have to be initialized with "new Array()", not with empty string (as before)
	this.H_boxes = new Array();
	this.W_boxes = new Array();
	this.C_boxes = new Array();
	this.F_boxes = new Array();
	this.J_boxes = new Array();
	this.R_boxes = new Array();
	this.Y_boxes = new Array();
	this.T_boxes = new Array();
	this.hits_total = "";
	this.hits_computed = "";
//	this.hits_sent = "";
	this.response_time = 0;					// Time (duration) of the last request processing
	this.response_size = 0;					// Size of the server response in characters
	this.server_respone_time = 0;		// Time (duration) of the last request processing (without transfer, value is provided by the completion server)
	this.time_A = 0;
	this.time_B = 0;
	this.time_C = 0;
	this.time_D = 0;
	this.time_E = 0;
	this.time_D0 = 0;
	this.time_E0 = 0;
	this.options_title = "";
	this.error_message_title = "";			// Optional error message
	this.error_message = "";			// Optional error message
	this.do_continue = "";					// If true coninue in case of error, if not stop the current search processing
	// Temporary 20-08-08 / Markus: until the history object can be removed
	this.elements = new Array();
	
	// NEW 26.07.06
	this.request_id = 0;
	this.query_string = "";
	this.query_types = "";
  
	// NEW 04-04-08 (Markus): the last query interaction, can be "user" or "history" (the last one is caused by browser back or forward)
	this.navigation_mode = "user";

	// Private members
	this.time = 0;							// For purposes of time measuring
	this.too_many = true;         		// Indicates that there are too many completions for the query
	this.completion_index = -1;       	// Index of completion selected
	this.completion_start_index = -1;
}


/*
	Holds the values for a query to performe; needed to start a query by setTimeout()
*/
QueryTask = function ()
{
	this.words = "";
	this.query_parameters = "";
	this.force = "";
}


QueryTask.prototype.init = function (words, query_parameters, force)
{
	this.words = words;
	this.query_parameters = query_parameters;
	this.force = force;
}


QueryTask.prototype.perform = function ()
{
	AC.launchQuery(this.words, this.query_parameters, this.force, "user");
}



/*
	Constructor for class AC_Class
*/
AC_Class = function (name)
{
	// NEW 13Jul06 (Holger): Name of the object
	this.name = name;

	// NEW 14Jul06 (Holger): create dedicated function for each object, e.g. AC_requestReturned (just calls AC.requestReturned)
	// NEW 24-10-07 (Markus): moved to own function AC_requestReturned (using here leads to an error of non-defined AC object)
    //eval(name + "_requestReturned = function() { " + name + ".requestReturned() }");

	// Create an instance of Result class in namespace this
	this.result = new Result();
	// This is for the Mozilla bug workaround:
	// it is used to indicate that a new XMLHttpRequest is created while the last one is still running
	this.isAbort = false;

	// Indicates whether the user works with a Mozilla browser
	// We need this for example because of different AJAX handling of browsers
	this.isMozilla = (navigator.appName.substring(0,8) == "Netscape");

	// Name of browser, e.g. "Opera", "Netscape"
	this.browser = navigator.appName.substring(0,8);

	// The XMLHttpRequest object
	// Is still used for other ajax requests like JSNotification
	this.xmlHttp = null;

	// XMLHttpRequest objects pool implemented as array of XMLHttpRequestContainers
	this.XMLHttpRequestContainers = new Array();

	// Ok, let's go, create the XMLHttpRequest object
	this.createRequest();

	// NEW 14-04-08 (Markus): the count of requests belonging to the current query
	//	Is necessary to check whether all requests of the query are terminated
	this.request_count = 0;

	// Unique identifier during the current session
	this.request_id = 0;

	// Unique identifier for a query during the current session
	// Note: a query may have several request (W request, F request, for example)
	this.query_id = 0;
	this.query_id_of_type = {'A': 0, 'F': 0, 'C': 0, 'J': 0, 'N': 0};

  // NEW 19-11-07 (Markus)
	this.box_navigation_mode = "A";
  // NEW 02-12-07 (Markus)
//	this.box_navigation_first_hit = 0;

	//! NEW 4Aug06 (Holger): query history
	this.history = new History();
//	this.time = 0;
//	this.duration = 0;

	// Number of all documents in the database
	this.documents_count = "";

	//! NEW 17Sep06 (Holger): remember last query launched, to avoid repeats
//	this.last_query_string = "XXX";

	// NEW: The content of the notes area
//	this.notes = "";

	// Contains those text pattern from text.php which are needed in javascript after a language change
	this.text = new Array();

	// For handling the detail_box
	this.detail_box = new DetailBox();

	// Holds the values for a query to performe; needed to start a query by setTimeout()
	this.query_task = new QueryTask();
	
  // Optional array declaring which facets (in the specified order) are shown
	// NEW Markus / 10-01-09
	this.facets_to_show = new Object();
	
  this.hits_autoscroll_threshold = "";	
  
  // NEW Markus / 29-06-09: missed
  this.delays = new Array();
}



// Class for input history (search requests)
History = function ()
{
	this.elements = new Array("");
	this.pos = 0;
	this.time = 0;	// for timestamping the current user interaction (to compute the duration of user inactivity)
	this.inactivity = 0;	// duration of user inactivity before the last key stroke
}


// Class for handling the detail_box
DetailBox = function ()
{
  this.visible = false;
  this.contains = null;
  this.query_type = null;
  this.max_completions_show = null;
  // Stores the next sibling of a box in the boxes list
  // It's used by function minmaximize() to restore the positions of moved boxes
	this.next_sibling = null;
}


// Container for the XMLHttpRequest objects
XMLHttpRequestContainer = function ()
{
	this.request_id = -1;	// -1 menas currently no request running
	this.query_id = 0;
	this.query_type = "";
	this.XMLHttpRequest = null;
	this.timer = null;
}


XMLHttpRequestContainer.prototype.toString = function ()
{
	return "request #" + this.request_id + ", query #" + this.query_id + ", type " + this.query_type;
}


// Show the progress of the request which belongs to this request container
XMLHttpRequestContainer.prototype.showProgress = function ()
{

	var nodes = document.getElementsByName("progress_image_" + this.query_type);
	for (var i=0; i<nodes.length; i++)
	{
		nodes[i].src="images/progress_" + this.query_type + ".gif";
	}

	// First integer is the rgb color to fade to, second the delay of fading in ms
	this.timer = setTimeout ("fade_out('autocomplete_H_boxes_1_subtitle,more_link_" + this.query_type + ",box_" + this.query_type + "', " + 200 + ", " + keyevent_counter + ")", AC.boxes_fade_out);
}


XMLHttpRequestContainer.prototype.hideProgress = function ()
{
	var nodes = document.getElementsByName("progress_image_" + this.query_type);
	for (var i=0; i<nodes.length; i++)
	{
		nodes[i].src="images/no_progress_" + this.query_type + ".gif";
	}
	setColor("autocomplete_H_boxes_1_subtitle,more_link_" + this.query_type + ",box_" + this.query_type, 0, 0, 0);
}


/**
	Delivers an idle XMLHttpRequest object which can run a request.
	First look whether the XMLHttpRequest container pool contains a idle object (which can run the request).
	If there is not such an object add a new one to the pool and deliver this new one.
	The type of the query is passed by parameter query_type and is stored in the container.
*/
AC_Class.prototype.getXMLHttpRequestContainer = function (query_type)
{
	for (var i = 0; i < this.XMLHttpRequestContainers.length; i++)
	{
		// Look for an idle object (menas current request ID is -1)
		if (this.XMLHttpRequestContainers[i].request_id < 0)
		{
			// Currently no request is using this XMLHttpRequest object, object is idle
			this.log_write (this.XMLHttpRequestContainers.length + " request object(s) are in pool, using object #" + i, this.log.levels.DEBUG);
			this.XMLHttpRequestContainers[i].request_id = this.request_id;
			this.XMLHttpRequestContainers[i].query_id = this.query_id;
			this.XMLHttpRequestContainers[i].query_type = query_type;
			return this.XMLHttpRequestContainers[i];
		}
	}

	// No one of the XMLHttpRequests is idle, so create a new one and put it into the array
	this.XMLHttpRequestContainers[i] = new XMLHttpRequestContainer();

	this.XMLHttpRequestContainers[i].XMLHttpRequest = this.createRequest();
	this.XMLHttpRequestContainers[i].request_id = this.request_id;
	this.XMLHttpRequestContainers[i].query_id = this.query_id;
	this.XMLHttpRequestContainers[i].query_type = query_type;

	this.log_write ("New request object created for query #" + this.XMLHttpRequestContainers[i].query_id + ", request #" + this.XMLHttpRequestContainers[i].request_id, this.log.levels.DEBUG);
	this.log_write ("Now " + this.XMLHttpRequestContainers.length + " request object(s) are in pool", this.log.levels.DEBUG);

	return this.XMLHttpRequestContainers[i];
//	return this.XMLHttpRequestContainers[i].XMLHttpRequest;
}


/**
	Setting object which is running request with ID request_id to state idle
*/
AC_Class.prototype.setXMLHttpRequestIdle = function (request_id)
{
//	this.log_write ("Setting object for request #" + request_id + " to state idle", this.log.levels.DEBUG);
	for (i = 0; i < this.XMLHttpRequestContainers.length; i++)
	{
		if (this.XMLHttpRequestContainers[i].request_id == request_id)
//		if (this.XMLHttpRequestContainers[i].request_id > -1 && this.XMLHttpRequestContainers[i].request_id == request_id)
		{
			notes.add("Request object #" + i + " for query #" + this.XMLHttpRequestContainers[i].query_id + ", request #" + this.XMLHttpRequestContainers[i].request_id + " set idle", 2);
			this.log_write ("Request object #" + i + " for query #" + this.XMLHttpRequestContainers[i].query_id + ", request #" + this.XMLHttpRequestContainers[i].request_id + " set idle", this.log.levels.DEBUG);
			// Set the current request ID to -1 (that means no running request)
			this.XMLHttpRequestContainers[i].request_id = -1;
			notes.add(this.XMLHttpRequestContainers.length + " request object(s) are in pool", 2);
 			return;
		}
	}
	notes.add("Error", 0);
	notes.add("Setting object for request #" + request_id + " to idle failed", 1);
	this.log_write ("Setting object for request #" + request_id + " to idle failed", this.log.levels.ERROR);
}


//! CLONE AN OBJECT (e.g., write AC2 = new cloneObject(AC))
AC_Class.prototype.cloneObject = function (what)
{
	for (i in what) {
		this[i] = what[i];
	}
}


//! NEW 04Oct06 (Ingmar): Global variables to OR with with keycode (to get one keycode for combinations)
ALT_KEYCODE = 256*256;
CTRL_KEYCODE = 2*256*256;
SHIFT_KEYCODE = 4*256*256;


//! PROCESS QUERY (called whenever something has been typed, see main search page)
//
//   triggers a call to autocomplete.php (with a short delay, so that nothing is called
//	 when the time between two keystrokes is less than that delay)
//
AC_Class.prototype.processInput = function (event)
{
	var force = false;
        var onclick = false;
	var completions_count = this.result.completions.length - 1;
	var prev_words = this.query_string;
	// Get query string and remove separators at the end of string
	//	words = document.getElementById('autocomplete_query').value.replace(new RegExp('[' + this.separators + ']+$'), '');
	words = document.getElementById('autocomplete_query').value;
	this.query_string = words;

	// Get last word; empty if query string ends with a separator
	last_word = words.match(new RegExp('[^' + this.separators + ']*$')).pop();

	// get keyCode; this works at least for IE, Mozilla, Firefox, and Opera
	k = event.keyCode

	// NEW 04Oct06 (Ingmar): Create artificial key codes here, so that a combination becomes a single keycode
	// NEW 09Oct06 (Holger): preserve old keycode (needed below)
	kk = k;
	if (event.altKey) { kk |= ALT_KEYCODE; }
	//if (event.shiftKey) { kk |= SHIFT_KEYCODE; }
	if (event.ctrlKey) { kk |= CTRL_KEYCODE; }
  //document.getElementById('autocomplete_subtitle').innerHTML = "PROCESS INPUT KEY '" + String.fromCharCode(k) + "' (key code = " + kk + ")";

	switch (kk)
	{
	  case 19: // pause
	     this.debug_mode = this.debug_mode == 0 ? 1 : 0;
//	     info("debug_mode: " + this.debug_mode);
	     break;
	     
	  case 33: // PgUp
		  fh = this.result.H_boxes['1']['fh'];
		  if (fh >= this.hits_per_page_while_typing) {
		    this.first_hit = fh - this.hits_per_page_while_typing;
		  }
		  else {
		    this.first_hit = 1;
		  }
	  	// UNDONE 14-04-08 (Markus): the following is false if first hit is less than hits_per_page_while_typing (which can happen with bookmarked jump to complete search)
			// CHANGED 09-04-08 (Markus): the following lines transferred in this "then" clause
			//	(was before outside the if statement and so it was executed even if there was nothing to do because we already were at the beginning of the hits)
	    this.box_navigation_mode = "R";
	    // NEW 02-12-07 (Markus): "more" mechanism has to known the first shown hit
	    this.first_hit_shown = this.first_hit;
		  // NEW 16-04-07 (Markus): new third parameter 'force': true force a request even if query string is unchanged
	  	// NEW 09-04-08 (Markus): now we use an array for the query parameters instead of only the query type
		  this.launchQuery(words, new Array(new Query("H", 1, this.hits_per_page_while_typing, this.first_hit, this.first_hit)), true);
	  break;

		case 34: // PgDown
      fh = this.result.H_boxes['1']['fh'];
      if (fh <= this.result.H_boxes['1']['total'] - this.hits_per_page_while_typing)
//		if (fh <= this.result.hits_count - this.hits_per_page_while_typing)
		  {
  			// NEW 14-04-08 (Markus): use "sent" of returned H boxes result
  			//	because in case of a bookmarked jump to complete search the amount of shown hits can vary from hits_per_page_while_typing
  //			this.first_hit = fh + parseInt(this.result.H_boxes['1']['count']);
  			this.first_hit = fh + parseInt(this.result.H_boxes['1']['sent']);
  			// CHANGED 09-04-08 (Markus): the following lines transferred in this "then" clause
  			//	(was before outside the if statement and so it was executed even if there was nothing to do because we already were at the end of the hits)
  	    this.box_navigation_mode = "R";
  	    // NEW 02-12-07 (Markus): "more" mechanism has to known the first shown hit
  	    this.first_hit_shown = this.first_hit;
  			// NEW 16-04-07 (Markus): new third parameter 'force': true force a request even if query string is unchanged
  		  // NEW 09-04-08 (Markus): now we use an array for the query parameters instead of only the query type
  		  this.launchQuery(words, new Array(new Query("H", 1, this.hits_per_page_while_typing, this.first_hit, this.first_hit)), true);
			}
		break;
/*
		UNCOMMENTED 09-04-08 (Markus): ALT + l/m are shortcuts of Firefox and IE
*/
		case (76 | ALT_KEYCODE): // 'ALT+l'
			console.log("USER ACTION: Alt+l -> decrease excerpt radius")
		  positionCursorAtEnd();
		  if (this.excerpt_radius >= 6)
		  {
		  	this.excerpt_radius = Math.round(this.excerpt_radius/2);
  			// NEW 16-04-07 (Markus): new third parameter 'force': true force a request even if query string is unchanged
  		  // NEW 09-04-08 (Markus): now we use an array for the query parameters instead of only the query type
  		  this.launchQuery(words, new Array(new Query("H", 1, this.hits_per_page_while_typing, this.first_hit, this.first_hit)), true);
		  }
		  break;

		case (77 | ALT_KEYCODE): // 'ALT+m'
			console.log("USER ACTION: Alt+m -> increase excerpt radius")
		  positionCursorAtEnd();
		  if (this.excerpt_radius <= 1000)
		  {
		  	this.excerpt_radius *= 2
  			// NEW 16-04-07 (Markus): new third parameter 'force': true force a request even if query string is unchanged
  		  // NEW 09-04-08 (Markus): now we use an array for the query parameters instead of only the query type
  		  this.launchQuery(words, new Array(new Query("H", 1, this.hits_per_page_while_typing, this.first_hit, this.first_hit)), true);
		  }
		  break;

// */

		case 38: // arrow up
  //		positionCursorAtEnd();
  		setFocusAndMoveCursorToEnd(document.getElementById('autocomplete_query'));
  		this.result.completion_index = this.result.completion_index * 1 - 1;
  		this.completeInput();
  		break;

		case 40: // arrow down
  //		positionCursorAtEnd();
 		setFocusAndMoveCursorToEnd(document.getElementById('autocomplete_query'));
  		this.result.completion_index = this.result.completion_index * 1 + 1;
  		this.completeInput();
  		break;
/*
    15-10-08 / Markus: not longer supported
		case (37 | CTRL_KEYCODE): // arrow left+CTRL
		this.history_back();
		break;

		case (39 | CTRL_KEYCODE): // arrow right + CTRL
		this.history_forward();
		break;
*/
		case 45: // insert
  		if (this.query_types.indexOf('W') > -1 )
  		{
  		  // NEW 23-04-07 (Markus): similiar behaviour like more_link functionality of the W box (completions)
  		  // Decrease the amount of shown entries in the W box by more_offset
  		  // NEW 09-04-08 (Markus): now we use an array for the query parameters instead of only the query type
  		  if (completions_count > parseInt(this.max_completions_show) + parseInt(this.more_offset)) {
  			  this.launchQuery(words, new Array(new Query("W", 1, completions_count - parseInt(this.more_offset))), true);
  //		    this.launchQuery(words, "W", "", true, "mcs", completions_count - parseInt(this.more_offset));
  		  }
  		  else if (completions_count > parseInt(this.max_completions_show)) {
  			  // NEW 09-04-08 (Markus): now we use an array for the query parameters instead of only the query type
  			  this.launchQuery(words, new Array(new Query("W", 1, this.max_completions_show)), true);
  //		    this.launchQuery(words, "W", "", true, "mcs", this.max_completions_show);
  		  }
  		}
  		break;

		case 46: // delete
  		if (this.query_types.indexOf('W') > -1 )
  		// NEW 23-04-07 (Markus): similiar behaviour like more_link functionality of the W box (completions)
  		// Increase the amount of shown entries in the W box by more_offset
  		{
  		  // NEW 09-04-08 (Markus): now we use an array for the query parameters instead of only the query type
  		  this.launchQuery(words, new Array(new Query("W", 1, parseInt(completions_count) + parseInt(this.more_offset))), true);
  //			this.launchQuery(words, "W", "", true, "mcs", parseInt(completions_count) + parseInt(this.more_offset));
  		}
  		break;
  
      // NEW 17Sep07 (Holger): Ctrl+Return goes to first hit
      case (13 | CTRL_KEYCODE):
        hit_body = document.getElementById('autocomplete_H_boxes_1_body').innerHTML;
        hrefs = hit_body.match(/href="(.*?)"/);
        if (hrefs.length >= 2) window.location.href = hrefs[1];
      break;

		default:
  		// NEW 04.05.07 (Markus): Ignore Backspace when query string is already empty
  		if (prev_words != undefined && prev_words == words && kk == 8)	{
  		}
  		else
  		// for keys a..z or 0..9 or $ or BACKSPACE or RETURN -> new completion list
  		// if (k == 8 || k == 13 || k == 16 || k >= 48 && k <= 57 || k >= 65 && k <= 90 || k == 32 || k == 190)
  		// why was there k = 16? That is SHIFT, right?
  
  		// NEW 22Jul06 (Holger): using onkeypress now in input field (was: onkeydown)
  		//
  		//   NOTE: with onkeypress, k = event.keyCode is 0 for anything typed, and the standard
  		//   codes for control characters like BS, TAB, cursor keys, etc.
  		//
  		//   NO! Does not work, because right after the onkeypress event, the input field is *not* up to date
  		//
  		// NEW 09Oct06 (Holger): kk != k means one of ALT, SHIFT, CTRL is/was pressed
                  // NEW 30May07 (Holger): prevented query launch for ˆ etc, Alt+Tab etc. no longer
                  // a problem because a query is now only launched when there is actually a change
  		if (/* k == kk && */ ( k == 8 || k == 13 || k == 32 || k >= 48 )) //if (k == 0 || k == 8)
  		{
  			// NEW 18Sep06 (Holger): log moved from above for reasons explained there
  			// NEW 16-04-08 (Markus): separate call of log_write to clear info box
  //			this.log_write("", this.log.levels.INFO, "clear");
  
  //			addNote("PROCESS INPUT KEY '" + String.fromCharCode(k) + "' (key code = " + k + ")");
  //			this.log_write("PROCESS INPUT KEY '" + String.fromCharCode(k) + "' (key code = " + k + ")", this.log.levels.DEBUG);
  			// Delayed call of getCompletions with current query string
  			// getCompletions will only proceed when after the delay, the
  			// then then current query string is the same as now
  			// this effects that no completions are computed for a keypress
  			// when another keypress follows within a time span less than
  			// the delay
        // CHANGED 27-02-08 (Markus)
  			this.first_hit = 1;
  			this.first_hit_shown = 1;
  			delay = this.delays[ Math.min(last_word.length, this.delays.length - 1) ];
  
  			// CHANGED Markus / 10-11-08: this fix the problem of multiple calls for empty query 
  			// when typing fast the backspace key until the input field is empty 
  			// (means for input "anna", type fast backspace four times)
  			if (k == 13) {
  //			if (k == 13 || words == "") {
  			  // User entered return
  				force = true;	// force the query even if the current query string is the same as the last one
  				delay = 0;
                                onclick = true;
  			}
  
  			// NEW 08-04-08 (Markus): use now a more object oriented approach to pass more complex objects than only string literals to the function
  			//	which should be triggered by setTimeout()
  		  this.query_task.init(words, this.getDefaultQueryParameters(onclick), force);
  		  notes.add("<b>Process user input key</b> '" + String.fromCharCode(k) + "' (key code = " + k + ")", 0);
  		  notes.add("Create a query task with default query parameters", 1);
  		  notes.add(objectArrayAsString(this.query_task.query_parameters), 2);
  		  notes.add("Delay: " + delay + "ms, force: " + force, 2);
  
  		  // Cite: BTW, general overal tip, stop passing string altogether, it's always preferable to pass a real function, rather than a string that'll need evaled.
  //		  info("Set timer for sending query '" + this.query_task.words + "' to " + delay + "ms");
  		  setTimeout(function(){ AC.query_task.perform(); }, delay);
  		  //			setTimeout("AC.query_task.perform()", delay);
  		  //			setTimeout('AC.launchQuery("' + words + '", "' + this.query_types + '", "", ' + force + ')', delay);
  
  			keyevent_counter++;
  		} // end if (k == 8 || k == 13 ....)
  		break; // end default case
	} // end switch (k)
	//}
	return false;
}


/**
 * LAUNCH QUERIES OF GIVEN TYPES (unless user is busy typing)
 *
 * @param string query_string
 * @param string query_parameters
 * @param boolean force is optional, default is false
 * @param string navigation_mode
 * @param string min_prefix_fallback allows launchQuery() to fall back to previous query string if the new one is too short
 */

AC_Class.prototype.launchQuery = function (query_string, query_parameters, force, navigation_mode, min_prefix_fallback)
{
//  info("Launch query: '" + query_string + "', '" + query_parameters.join(", ") + "', force: " + force);
	// Strip off trailing separators
  // NEW Markus / 14-04-09: moved from below to here
	query_string = query_string.replace(new RegExp('[' + this.block_separators + ']+$'), '');

  // Default for force is false
	if (typeof force == "undefined") {
		force = false;
	}
	if (typeof navigation_mode == "undefined") {
		navigation_mode = 'user';
	}
	if (typeof min_prefix_fallback == "undefined") {
		min_prefix_fallback = false;
	}

	// Time measurement
	this.result.times = new Object();
	// Array for the javascript times, initialized with the start time
	this.result.js_times = new Array(new Date().getTime().toString());

	// PROCEED ONLY IF USER IS NOT BUSY TYPING
	// NOTE / TODO: Hannah empfiehlt, this.separators durch this.block_separators zu ersetzen
	current_query_string = document.getElementById('autocomplete_query').value.replace(new RegExp('[' + this.block_separators + ']+$'), '');
  //	current_query_string = document.getElementById('autocomplete_query').value.replace(new RegExp('[' + this.separators + ']+$'), '');
	//words_after_delay = document.getElementById('autocomplete_query').value;
	if (current_query_string != query_string)
	{
		this.log_write("<small>[abort launch of \"" + query_string + "\": user is busy typing: '" + current_query_string + "']</small>", this.log.levels.INFO);
/*		this.log_write("<small>[abort launch of \"" + query_string + "\": user is busy typing]</small>", this.log.levels.INFO);*/
		return;
	}

	// NEW 17Sep06 (Holger): PROCEED ONLY IF QUERY DIFFERENT FROM LAST
	//
	// Note: if one types, say, "meh", processInput will be called when the
	// input field contains "m", "me", and "meh", but by the time the function
	// is executed, the input field will contain "meh" for all three calls, and
	// the same query will be launched three times and it will *not* be canceled
	// by the "busy-typing" mechanism from above!
	// NEW 16-04-07 (Markus): possibility to force a request even if the query string is unchanged (by user return for example)
	// Scenario: one of the request runs into timeout, user types return to repeat the request
	if (! force && query_string == this.last_query_string)
	{
		this.log_write("<small>[abort launch of \"" + query_string + "\": same query again]</small>", this.log.levels.INFO);
		return;
	}

	// The last word of the query string (= characters after the last seperator)
//	last_word = query_string.match(new RegExp('[^' + this.separators + ']*$')).pop();
	last_word = query_string.match(new RegExp('[^' + this.block_separators + ']*$')).pop();


	// Strip off trailing separators
	// NOTE / TODO: Hannah empfiehlt, this.separators durch this.block_separators zu ersetzen
  // NEW Markus / 14-04-09: moved from here to beginning of method
  //  query_string = query_string.replace(new RegExp('[' + this.block_separators + ']+$'), '');
  //	query_string = query_string.replace(new RegExp('[' + this.separators + ']+$'), '');

/*
  // TODO Markus / 10-01-09: is this still used?
	// NEW 04.05.07 (Markus): for an empty query clear all result boxes (maybe with exeption of the f boxes)
	if (query_string.length == 0)
	{
		// NEW 17-01-07 (Markus): if facettes should be shown even for empty queries
		//  we are not allowed to clear the F boxes
		if (this.show_facets_for_empty_query)
		{
			var qt = this.query_types.replace(new RegExp("F", ""));
			this.clearResults(qt);
			var e = document.getElementById("autocomplete_W_boxes_1");
			if (e != null) e.style.display = "none";
		}
		else {
			this.clearResults(this.query_types);
		}
	}
*/
  // NEW Markus / 15-04-09: if user inputs a second, third, etc word which is too short, we go back to the previous input (if min_prefix_fallback is true)
  if (min_prefix_fallback && last_word.length < this.min_prefix_size) {
    query_string = this.result.query_string;
    last_word = query_string.match(new RegExp('[^' + this.block_separators + ']*$')).pop();
		this.log_write("<small>[fallback to \"" + this.result.query_string + "\"]</small>", this.log.levels.INFO);
  }

	// DO NOTHING FOR SHORT PREFIXES EITHER (exception: ? or ! or only CAPITALS or EMPTY QUERY)
	if (query_string.length > 0 && query_string[query_string.length-1] != ']'
  	&& (query_string.length < this.min_query_size || last_word.length < this.min_prefix_size)
  	&& !query_string.match(new RegExp('['+this.capitals+']{2,}$')) // (browser incompatibility issue: the "new regExp" gives an error in Mozilla, but the whole thing works anyway)
  	&& !query_string.match(/[\?\!]/))
	{
		this.log_write("<small>[abort launch of \"" + query_string + "\": prefix too short]</small>", this.log.levels.INFO);
		// NEW 20.12.06 (Markus): don't return because we can not get translations from web server (in case of change of language)
		document.getElementById ("autocomplete_H_boxes_1_subtitle").innerHTML = this.getText("query_too_short", this.min_query_size);
		return;
	}

	this.last_query_string = query_string;
	this.query_string = query_string;
	
	notes.add("Launching query: " + query_string, 1);

	// Store time for performance measuring (time_E)
	this.result.time_E0 = new Date().getTime();

	// NEW 21.12.06 (Markus)
	this.query_id++;

  // 04-10-08 / Markus: Note: query_string is *not* UTF-8 encoded (in spite of page encoding = UTF-8) because is got from an input tag;
  //  So we have to encode it explicitly in ajax.php if collection is UTF-8 encoded
  query_string = "query=" + escape(query_string)
													+ "&name=" + escape(this.session_name)
													+ "&path=" + escape(this.index_path)
													+ "&page=" + escape(this.index_page)
													+ "&log=" + escape(this.error_log)
													// + "&" + this.varnames_abbreviations["language"] + "=" + escape(this.language)
													+ "&qid=" + this.query_id
													// + "&prt=" + escape(query_type_P == "P" ? 1 : 0);
													// Send the duration of the last user inactivity (before key stroke or click event) to the server
													//	and set the duration of inactivity to zero to dignal that sending is done (and the next inactivity can be recorded)
//													+ "&inactivity=" + this.history.inactivity
													+ "&navigation_mode=" + navigation_mode;

	this.history.inactivity = 0;

	// NEW 17-07-07 (Markus): use varnames_abbreviations for iteration instead of user_preferences
	for (var pref in this.varnames_abbreviations)
	{
	  // An optional function parameter overrides the value of this user preference
	  // Some parameters needs a special treatment (they are defined by the query_parameters above): qt, qi, mcs, fh, fhs
	  if (this.varnames_abbreviations[pref] != 'qt'
	  	&& this.varnames_abbreviations[pref] != 'qi'
	  	&& this.varnames_abbreviations[pref] != 'fh'
	  	&& this.varnames_abbreviations[pref] != 'fhs'
	  	&& this.varnames_abbreviations[pref] != 'mcs')
//	  if (this.varnames_abbreviations[pref] != 'mcs' && this.varnames_abbreviations[pref] != 'qi')
	  {
	    value = eval("this." + pref);
	    //this.log_write('pref: ' + pref + ' --> ' + value, this.log.levels.DEBUG)
	    // '!==' is important. If '!=' used instead all values with value zero are obmitted, too
	    if (value !== '') {
	      query_string += "&" + this.varnames_abbreviations[pref] + "=" + value;
	    }
	  }
	}
	// ATTENTION: in the next lines distinguishing between "this.query_types" and the method parameter "query_types" is very important!
	// This is the "basic" query string which the query types are appended to
	var basic_query_string = query_string;

	// Set the number of request sent for this query to zero
	this.request_count = 0;
//	info(": " + objectArrayAsString(query_parameters))

  // Now the single requests for separate calls
	for (var q in query_parameters)
	{
	  
		query_string = basic_query_string + "&qi=" + query_parameters[q].index
				+ "&fh=" + query_parameters[q].first_hit
				+ "&fhs=" + query_parameters[q].first_hit_shown
				+ "&mcs=" + query_parameters[q].count;

		// If the query belongs to the detail box increase the parameter mcs
		//		if (AC.detail_box.query_type == qti) {
		//      query_string = query_string.replace(new RegExp("&mcs=" + parseInt(this.max_completions_show)), "&mcs=" + parseInt(AC.max_completions_show_right));
		//		}

		notes.add("Launching request #" + this.request_id + ", query #" + this.query_id + ", type " + query_parameters[q].type, 2);
		notes.add(objectAsString(query_parameters[q]), 3);
//		this.log_write("Launching query #" + this.query_id + ", request #" + this.request_id + " (" + query_parameters[q].type + ") ...", this.log.levels.INFO);
//		this.log_write("Launching query #" + this.query_id + ", request #" + this.request_id + " (\"" + query_string + "\" with types \"" + query_parameters[q].type + "\") ...", this.log.levels.INFO);
		this.request(query_string, query_parameters[q].type);

		// Increase the number of request sent for this query
		this.request_count++;
	}
}


/*
	SEND QUERY TO SERVER via XMLHttpRequest
*/
AC_Class.prototype.request = function (query_string, query_type)
{
	// Start request
	// Get a free object (idle state) from the XMPHttpRequest pool to perform the request
	var container = this.getXMLHttpRequestContainer(query_type);
	var request = container.XMLHttpRequest;

	try
	{
		// Open url of PHP script which process this query and send header
		request.open( "POST", this.autocomplete_url + "ajax.php", true );
		
    // 04-10-08 / Markus: now query_string is explicitly UTF-8 encoded by us
    //  (because content of an input tag seems to be independant of the page encoding)
    //  Send this information now as part of the content type
//    request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
//		request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded; charset=" + this.encoding);
		request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

		// The callback function
		// NEW 14Jul06 (Holger): use global callback function (see below what it does)
		// NOTE: something like AC.requestReturned does not work (the "this" will not be right in that function)
		// NEW 14Jul06 (Holger): use dedicated callback function, e.g., AC_requestReturned
    // request.onreadystatechange = eval(this.name + "_requestReturned");
    // NEW Markus / 10-01-09: This way it works without a global callback function
		var t = this;
		request.onreadystatechange = function() { t.requestReturned(); };

		// Start animated gif to show progress
		container.showProgress();

		// Store time for performance measuring (time_D)
		this.result.time_D0 = new Date().getTime();
	}
	catch (e)
	{
		this.log_write ("Error while preparing request #" + this.request_id + " : " + e, this.log.levels.FATAL);
		notes.add("Error while preparing request #" + this.request_id, 0);
		notes.add(e, 1);
	}
	try
	{
		this.setTimestamp('before: send(#' + this.request_id + ')');
		// Send query
		request.send(query_string + "&rid=" + container.request_id + "&" + this.varnames_abbreviations["query_types"] + "=" + container.query_type);
		this.setTimestamp('after: send(#' + (this.request_id) + ')');
		// NEW 17Sep06 (Holger): just checking whether send takes significant time
		this.result.time_D2 = new Date().getTime();

		// Increase the request ID
		this.request_id++;
	}
	catch (e)
	{
		this.log_write ("Error while sending request #" + this.request_id + " : " + e, this.log.levels.FATAL);
		notes.add("Error while sending request #" + this.request_id, 0);
		notes.add(e, 1);
	}
}


/*
	The callback function for the XMLHttpRequest 'request'.
	Is called when the autocomplete request returns.
*/
AC_Class.prototype.requestReturned = function ()
{
	// Check all request containers whether they are running and have finished their request
	for (var i = 0; i < this.XMLHttpRequestContainers.length; i++)
	{
		// Test whether this XMLHttpRequest has a running query
		if (this.XMLHttpRequestContainers[i].request_id > -1)
		{
			var xmlHttpRequest = this.XMLHttpRequestContainers[i].XMLHttpRequest;

			try
			{
//				this.setTimestamp('[#' + i + ']after: readyState == ' + xmlHttpRequest.readyState);

				if (xmlHttpRequest.readyState == 4)
				{
//                    info("i=" + i + " --> " + this.XMLHttpRequestContainers[i] + " / id=" + this.XMLHttpRequestContainers[i].request_id);
					// NEW 30.04.07 (Markus): Belongs this request to the the current query of this type?
					// New is to destinguish between query types to prevent that termination of one query of type A cut off a query of type F, for example
					//
					// NEW 17-08-07 (Markus): comparaison with this.query_id_of_type is not necessary because the query_id is for all current request_id's the same
					if (this.XMLHttpRequestContainers[i].query_id == this.query_id)
//					if (this.XMLHttpRequestContainers[i].query_id == this.query_id_of_type[this.XMLHttpRequestContainers[i].query_type])
					{
						// Request returned, return code is ok
						if (xmlHttpRequest.status == 200)
						{
						  // Here he don't have the request_id
//	     				this.setTimestamp('after: request returned(#' + this.result.request_id + ")");
	     				this.setTimestamp('after: request returned');
  						error_msg = null;

  						// Set time D and size of response
							this.result.time_D = new Date().getTime() - this.result.time_D0;
							// NEW 17Sep06 (Holger): just checking whether send takes significant time
							this.result.time_D2 = new Date().getTime() - this.result.time_D2;
							this.response_size = xmlHttpRequest.responseText.length;

							// Retrieve the request id
							// NOTE: must be the first line of response text
							var lines = xmlHttpRequest.responseText.split("\n");

							// Skip empty lines at beginning
							for (var j = 0; j < lines.length; j++) {
							  if (lines[j].length > 2) break;  // at miniumum "x=y"
							}

							var pair = lines[j].split("=");
							var request_id = pair[0].replace(/ /, "");
							if (request_id == "request_id")
							{
								request_id = pair[1].replace(/\s/g, "").replace(/;/, "");
								if (request_id != this.XMLHttpRequestContainers[i].request_id)
								{
									// The request_id of the responseText don't match the one of the iteration's request object;
                                    //  means we are in the current query but not the correct request, so continue the iteration to look for the right request object
//                                    info('Looking for matching request container for request_id ' + request_id + ' ... '
//										+ this.XMLHttpRequestContainers[i].request_id + ' didn\'t match', this.log.levels.DEBUG);
									continue;
								}
							}
							else {
							  if (xmlHttpRequest.responseText == "") {
							    err_msg = '<b>Response from server is empty</b>. Probably a parse error in one of the included files of ajax.php occurred';
							  }
							  else if (lines.length == 1) {
							    err_msg = '<b>First line of response text does not contain the request id</b>: ' + lines[0];
							  }
							  else  err_msg = '<b>First line of response text does not contain the request id</b>: ' + lines[0] + lines[1];
                this.log_write(err_msg, this.log.levels.FATAL);
							  return;
							}

							this.setTimestamp('before: response text evaluated(#' + request_id + ")");

							// Stop (clear) the timer which should fade out the box belonging to this request
	  					clearTimeout (this.XMLHttpRequestContainers[i].timer);

							// Try to evaluate the returned text as javascript
							try
							{
								eval('with(this.result) { ' + xmlHttpRequest.responseText + '}');
							}
							catch (e)
							{
								document.getElementById('autocomplete_H_boxes_1_title').innerHTML = "COULD NOT EVAL RESPONSE TEXT : " + e + "  " + xmlHttpRequest.responseText;
								return;
							}

							// Update the history object of AC object
							// UNCOMMENTED 20-08-08 / Markus: leads to errors with the new extension of box assignments like F_boxes[1]={'title' : '...', 'body' : '...'}
//							this.history.elements = elements;
//							this.history.pos = pos;
//							this.log_write('history.elements: ' + this.history.elements + ' pos: ' + this.history.pos, this.log.levels.DEBUG);

							// Now refresh for every member of the new result the corresponding HTML element
							// Note: was before the method showResults which refreshed *all* HTML elements, which is no longer accurate
							// because of parallel requests
							for (var j = 1; j < lines.length; j++)
							{
								pair = lines[j].split("=");
								member = pair[0].replace(/\s/, "");
                                //								info("Line " + j + ": " + lines[j] + " ---> " + member + ": " + typeof(eval("this.result." + member)));
								if (typeof(eval("this.result." + member)) == "object")
								{
//								    info('Expression in line ' + j + ' is of type "object", now stepping through its members ...');
									// Two examples which demonstrates the both different kinds of evaluation of a line statement
									// A. F_boxes={'1' : {'title' : '...', 'body' : '...'}, '2' : {'title' : '...', 'body' : '...'} ...}
									// B. F_boxes[1]={'title' : '...', 'body' : '...'}
									// Statement A set the complete F_boxes (means overwrite them)
									// Statement B set the first element of F_boxes, other existing elements keep unchanged
									var tmp = eval("this.result." + member);
//									info('Consider member "' + tmp + '"');

                  for (x in tmp)
									{
//									  console.log('Consider member "' + x + '" --> ' + tmp[x]);
									  if (typeof(tmp[x]) == "object")
										{
											// This is a statement of type A
//										  console.log(x + ' --> ' + tmp[x]); console.log(tmp[x] + " is " + typeof(tmp[x]));
											for (xx in tmp[x])
											{
//												console.log('autocomplete_' + member + '_' + x + '_' + xx + " = " + unescape(tmp[x][xx]));
												if (document.getElementById('autocomplete_' + member + '_' + x + '_' + xx) != undefined)
												{
												  // NEW 17-11-07 (Markus)
													// Distinguish between A ("append") and R ("replace") mode if the element is the body of a box
													if (xx == 'body' && tmp[x]['mode'] != undefined && tmp[x]['mode'] == 'A')
													{
														// Ok, A (=append) mode
														// Compute the current height of the box to scroll to the top of the appended data
	                          var e = document.getElementById('autocomplete_' + member + '_' + x + '_' + xx);
														h = e.offsetHeight + getStyle(e, "paddingTop") + getStyle(e, "paddingBottom") + getStyle(e, "marginTop") + getStyle(e, "marginBottom");
														
//                            // Remove the "more" link from the end of the previous hits ...
//														var prev_hits = document.getElementById('autocomplete_' + member + '_' + x + '_' + xx).innerHTML;
//														// Wether upper or lower case letter for tags are used differs from browser to browser; therefore use br|BR, for example
//														prev_hits = prev_hits.replace(/(\[\s*<[aA].*id="?more_hits"?.*\<\/[aA]>\s*]\s*<(br|BR)(\/)?>)/, '');
//														// ... and append the new hits
//														document.getElementById('autocomplete_' + member + '_' + x + '_' + xx).innerHTML = prev_hits + unescape(tmp[x][xx]);
														// NEW 04-01-07 / 20-08-08 (Markus): use now this alternative for appending hits
														var more = document.getElementById("more_hits");
														if (more) {
															more.parentNode.removeChild(more);
														}
														var t = document.createElement("div");
														t.innerHTML = unescape(tmp[x][xx]);
														document.getElementById('autocomplete_' + member + '_' + x + '_' + xx).appendChild(t);
												  }
												  else {
//												    console.log("Set innerHTML of 'autocomplete_" +  + member + '_' + x + '_' + xx + "' to " + unescape(tmp[x][xx]));
												    document.getElementById('autocomplete_' + member + '_' + x + '_' + xx).innerHTML = unescape(tmp[x][xx]);
												  }
	                        // Now scroll to the top of the appended data (to show the new entries)
//													if (h) window.scrollTo(0, h);
//													window.scrollTo(0, myScrollHeight);
												}
										  }
											// Set the box to display=block (make it visible)
											if (document.getElementById('autocomplete_' + member + '_' + x) != undefined)
											{
											  // NEW 21-01-07 (Markus): if tmp[x] is an empty array make the corresponding box invisible
                        if (tmp[x].length == 0) {
//  											  console.log("Hide: " + 'autocomplete_' + member + '_' + x);
                          document.getElementById('autocomplete_' + member + '_' + x).style.display = "none";
                        }
  											else {
//  											  console.log("Make visible: " + 'autocomplete_' + member + '_' + x);
                          document.getElementById('autocomplete_' + member + '_' + x).style.display = "block";
  											}
											}
										}
										else
										{
											// This is a statement of type B
											// NEW 19-08-08 / Markus: zu hackig?
											// 'W_boxes[1]' change to 'W_boxes_1' to get the valid element 'autocomplete_W_boxes_1_body', for example
											member = member.replace(/\[/, "_").replace(/\]/, "");
//										  console.log('Check autocomplete_' + member + '_' + x);
											if (document.getElementById('autocomplete_' + member + '_' + x) != undefined)
											{
												// NEW 17-11-07 (Markus)
												// Distinguish between A ("append") and R ("replace") mode if the element is the body of a box
												if (x == 'body' && tmp['mode'] != undefined && tmp['mode'] == 'A')
												{
													// Ok, A (=append) mode
													// Compute the current height of the box to scroll to the top of the appended data
                          var e = document.getElementById('autocomplete_' + member + '_' + x);
													h = e.offsetHeight + getStyle(e, "paddingTop") + getStyle(e, "paddingBottom") + getStyle(e, "marginTop") + getStyle(e, "marginBottom");

//													var prev_hits = document.getElementById('autocomplete_' + member + '_' + x).innerHTML;
													// Remove the "more" link from the end of the previous hits
													// Wether upper or lower case letter for tags are used differs from browser to browser; therefore use br|BR, for example
//													prev_hits = prev_hits.replace(/(\[\s*<[aA].*id="?more_hits"?.*\<\/[aA]>\s*]\s*<(br|BR)(\/)?>)/, '');
//													prev_hits = prev_hits.replace(/(\[\s*<[aA].*id="?next_hits"?.*\<\/[aA]>\s*]\s*<(br|BR)(\/)?>)/, '');
													// ... and append the new hits
//													document.getElementById('autocomplete_' + member + '_' + x).innerHTML = prev_hits + unescape(tmp[x]);
													// NEW 04-01-07 / 20-08-08 (Markus): use now this alternative for appending hits
													var more = document.getElementById("more_hits");
													if (more) {
														more.parentNode.removeChild(more);
													}
													var t = document.createElement("div");
													t.innerHTML = unescape(tmp[x]);
													document.getElementById('autocomplete_' + member + '_' + x).appendChild(t);
	                        // Now scroll to the top of the appended data (to show the new entries)
//													if (h) window.scrollTo(0, h);
// 													window.scrollTo(0, myScrollHeight);
												}
												else {
//											    console.log("Set innerHTML of 'autocomplete_" + member + '_' + x + "' to " + unescape(tmp[x]));
												  document.getElementById('autocomplete_' + member + '_' + x).innerHTML = unescape(tmp[x]);
												}
												if (document.getElementById('autocomplete_' + member) != undefined)
												{
													// Set the box to display=block (make it visible)
//													console.log("Make visible: " + 'autocomplete_' + member);
									        document.getElementById('autocomplete_' + member).style.display = "block";
												}
											}
										}
									}  // END FOR
									
  							  // NEW 16-09-08 (Markus): if tmp is an empty array make the corresponding box invisible
                  if (tmp == "") {
										// 'W_boxes[1]' change to 'W_boxes_1' to get the valid element 'autocomplete_W_boxes_1_body', for example
										member = member.replace(/\[/, "_").replace(/\]/, "");
//                    info("Hide: " + 'autocomplete_' + member);
  									document.getElementById('autocomplete_' + member).style.display = "none";
                  }
									
								}
								else
								{
									if (document.getElementById('autocomplete_' + member) != undefined)
									{
										document.getElementById('autocomplete_' + member).innerHTML = unescape (eval("this.result." + member));
									}
								}
							}

							if (this.result.request_id == this.XMLHttpRequestContainers[i].request_id)
							{
								this.XMLHttpRequestContainers[i].hideProgress();
							}
							else
							{
                info("Warnung (TODO): ungleiche request_id's\n" + this.result.request_id + " <> " + this.XMLHttpRequestContainers[i].request_id + ", " + this.result.query_types + " i=" + i);
//                              document.getElementById('autocomplete_H_boxes_1_title').innerHTML = "Ungleiche request_id's\n" + this.result.request_id + " <> " + this.XMLHttpRequestContainers[i].request_id;
							}

							this.setTimestamp ('after: response text evaluated(#' + this.result.request_id + ")");

							// NEW 26.07.06 (Markus): an own try..catch for showing results
							try
							{
								notes.add("Request #" + this.result.request_id + " returned", 1);
								notes.add(this.XMLHttpRequestContainers[i].toString(), 2);
//								this.log_write2("Request #" + this.result.request_id + " returned", "", this.log.levels.INFO);

								// Set the used XMLHttpRequest object to state "idle"
								this.setXMLHttpRequestIdle (this.result.request_id);
							}
							catch (e)
							{
								this.log_write ("Error while showing results: " + e, this.log.levels.ERROR);
								notes.add("Error while showing results", 0);
								notes.add(e, 1);
							}

							// We don't need to round because getTime() delivers milliseconds (this.result.time_D = Math.round ((new Date().getTime() - this.result.time_D) * 100) / 100;)
							this.result.time_E = new Date().getTime() - this.result.time_E0;

							// Show time for request in notes area
							this.log_write ("Times A / B / C / D / E: " + this.result.time_A + " / " + this.result.time_B
							+ " / " + this.result.time_C
							+ " / " + this.result.time_D + " / " + this.result.time_E + " ms", this.log.levels.DEBUG);

							// NEW 07.12.06 (Markus): special case for timeout
							if (this.result.time_A <= 0) {
								tmp_A = "?"
							}
							else {
								if (this.result.time_A < 1) {
									tmp_A = "< 1";
								}
								else {
									tmp_A = Math.floor (this.result.time_A);
								}
							}

							// NEW 07.12.06 (Markus): special case for timeout
							if (this.result.time_B <= 0) {
								tmp_B = "?";
							}
							else {
								tmp_B = Math.floor (this.result.time_B - this.result.time_A);
							}

							tmp_C = Math.floor (this.result.time_C - this.result.time_B);
							tmp_D = Math.floor (this.result.time_D - this.result.time_C);
							// NEW 17Sep06 (Holger): just checking whether send takes significant time
							tmp_D2 = Math.floor (this.result.time_D2 - this.result.time_C);
							tmp_E = this.result.time_E - this.result.time_D;

							var time_diff = this.response_time - this.result.response_time;
              var download_rate = Math.floor(8*this.response_size/tmp_D);

							notes.add("Time & size", 2);
							notes.add("Size of received bytes & download rate", 3);
							notes.add("received from completion server: " + this.result.response_size + " bytes", 4);
							notes.add("received from web server: " + this.response_size + "</b> bytes" + " (<b>" + Math.floor (this.response_size / this.result.response_size * 100) / 100 + "</b> blowup)", 4);
							notes.add("download rate: " + download_rate + " kbits/sec (ignoring upload of query)", 4);

//							this.log_write("<b>Overview:</b><br>Size (server/sent): <b>" + this.result.response_size + "</b> / <b>" + this.response_size + "</b> bytes" + " (<b>" + Math.floor (this.response_size / this.result.response_size * 100) / 100 + "</b> blowup)", this.log.levels.INFO);
//							this.log_write("Download rate: <b>" + download_rate + " </b> kbits/sec (ignoring upload of query)", this.log.levels.INFO);

              // NEW 03Jan08 (Holger): show connection speed, if div with that id exists (experimental)
              var speed = document.getElementById("connection_speed");
              if (speed && download_rate > 0) speed.innerHTML = "[" + this.response_size + " bytes, " + download_rate + " kbit/s]";
              // NEW 06Jan08 (Holger): transmit time_D and time_E to the PHP
              // (Note: hits, completions, etc. are already shown at this point,
              // only the (optional) info box is missing)
              var client_measurement = document.getElementById("client_measurement");
              if (client_measurement)
              {
                var params =     "query="  + this.result.query_string
                              + "&types="  + this.result.query_types
                              + "&time_D=" + this.result.time_D
                              + "&time_E=" + this.result.time_E;
                try
                {
                  //var XMLHttpRequest = this.getXMLHttpRequestContainer("").XMLHttpRequest;
                  var XMLHttpRequest = this.createRequest();
                  XMLHttpRequest.open("POST", "client_measurement.php", true);
                  XMLHttpRequest.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );
                  XMLHttpRequest.onreadystatechange = function() { };
                  XMLHttpRequest.send(params);
                }
                catch(e)
                {
                }
                // iframe.location.href = "client_measurement.php?" + params;
              }

							tmp = "<table class='notes_table'>"
							+ "<tr><td>Computation completion server (CS):</td><td style='text-align:right'><b>" + tmp_A + "</b> ms</td></tr>"
							+ "<tr><td>Communication CS <-> web server:</td><td style='text-align:right'><b>" + tmp_B + "</b> ms</td></tr>"
							+ "<tr><td>Computation web server:</td><td style='text-align:right'><b>" + tmp_C + "</b> ms</td></tr>"
							+ "<tr><td>Communication web server <-> client:</td><td style='text-align:right'><b>" + tmp_D + "</b> ms</td></tr>"
							// NEW 17Sep06 (Holger): just checking whether send takes significant time
							+ "<tr><td>&nbsp;&nbsp;- download only:</td><td style='text-align:right'><b>" + tmp_D2 + "</b> ms</td></tr>"
							+ "<tr><td>Computation client:</td><td style='text-align:right'><b>" + tmp_E + "</b> ms</td></tr>"
							+ "<tr><td>Total time:</td><td style='text-align:right'><b>" + this.result.time_E + "</b> ms</td></tr>"
							+ "</table>";

							notes.add("Times overview", 3);
							notes.add(tmp, 4);

							// Show the times measured in result->times in the note area

							// Table with javascript times
							tmp = "<table class='notes_table'>";
							tmp += "<tr><td><b>Javascript:</b></td><td></td><td><b></b></td></tr>";

							last = 0;
							for (var i = 1; i < this.result.js_times.length; i++)
							{
								parts = this.result.js_times[i].split("|");
								span = Math.floor((parts[1] - last)*10)/10;
								if (span = Math.floor(span)) span_string = span + ".0"; else span_string = span;
								if (span > 10) span_string = '<font color=red>' + span_string + '</font>';
								if (parts[1] = Math.floor(parts[1])) timestamp = parts[1] + ".0"; else timestamp = parts[1];
								tmp += "<tr><td>" + parts[0] + "</td><td style='text-align: right'>" + timestamp + "</td><td style='text-align: right'><b>" + span_string + "</b>&nbsp;ms</td></tr>";
								last = parts[1];
							}

							//					for (property in this.result.js_times)
							//					{
							//						if (property == 'begin') continue;
							//						tmp += "<tr><td>" + property + "</td><td>" + this.result.js_times[property] + "</td><td><b>" + Math.floor((this.result.js_times[property] - last)*100)/100 + "</b> ms</td></tr>";
							//						last = this.result.js_times[property];
							//					}

							tmp += "<br>";
							tmp += "<tr><td><b>PHP:</b></td><td></td><td><b></b></td></tr>";

							// Table with PHP times
							var last = 0;
							for (var i = 1; i < this.result.times.length; i++)
							{
								var parts = this.result.times[i].split("|");
								// NEW 16.10.06 (Markus): Versuch, die Zeiten gruppenweise zu berechnen, erst mal gescheitert
								//						var segs = parts[0].split(":");
								//						var regExp = new RegExp ("[" + segs[1].substr(1) + "]");
								////						alert (this.result.times[1]+ "  " +regExp + "-->" + this.result.times[1].search(regExp))
								//						for (var j = 1; j < this.result.times.length; j++)
								//						{
								//							if (this.result.times[j].search(regExp) > 0) {
								//								break;
								//								alert(this.result.times[j]);
								//							}
								//						}

								span = Math.floor((parts[1] - last)*10)/10;
								if (span = Math.floor(span)) span_string = span + ".0"; else span_string = span;
								if (span > 100) span_string = '<font color=red>' + span_string + '</font>';
								else if (span > 10) span_string = '<font color=magenta>' + span_string + '</font>';
								if (parts[1] = Math.floor(parts[1])) timestamp = parts[1] + ".0"; else timestamp = parts[1];
								tmp += "<tr><td>" + parts[0] + "</td><td style='text-align: right'>" + timestamp + "</td><td style='text-align: right'><b>" + span_string + "</b>&nbsp;ms</td></tr>";
								last = parts[1];
							}

							notes.add("Time details (Javascript / PHP)", 3);
							notes.add(tmp, 4);
//							this.log_write(tmp + "</table>", this.log.levels.INFO);

              // NEW 03-12-07 (Markus): Resize the height of the hit box and detail box
							resizeHeight();
						}
						else // Daten wurden nicht ordnungsgem‰ﬂ oder unvollst‰ndig empfangen (z.B. wegen abort)
						{
							this.log_write ("Daten wurden nicht ordnungsgem‰ﬂ oder unvollst‰ndig empfangen (z.B. wegen abort): " + xmlHttpRequest.status, this.log.levels.FATAL);
							document.getElementById('autocomplete_H_boxes_1_title').innerHTML = "Daten wurden nicht ordnungsgem‰ﬂ oder unvollst‰ndig empfangen (z.B. wegen abort): " + xmlHttpRequest.responseText;
							// Set the used XMLHttpRequest object to state "idle"
							this.setXMLHttpRequestIdle (this.XMLHttpRequestContainers[i].request_id);
						}

						// Initialize completions.
						this.result.completions = new Array();
						regex = /href="javascript:AC.completion_link\('(.+?)'\)"/g;
            try {
						  completions_input  = document.getElementById('autocomplete_W_boxes_1_body').innerHTML;
						  completions_input += document.getElementById('autocomplete_F_boxes_1_body').innerHTML;
						  completions_input += document.getElementById('autocomplete_F_boxes_2_body').innerHTML;
						  completions_input += document.getElementById('autocomplete_F_boxes_3_body').innerHTML;
              // DEBUG(history bug): Bis hierhin geht's noch.
              // console.log("Hier noch!");
						  completions_input += document.getElementById('autocomplete_F_boxes_4_body').innerHTML;
              // DEBUG(history bug): Bis hierhin nicht mehr!
              // console.log("Hier nich!");
            } catch (e) {
              // console && console.log("WARNING: one of my boxes is not there (" + e + ")");
            }
						while (nextOcc = regex.exec(completions_input))
						{
							this.result.completions.push(nextOcc[1]);
						}


						this.result.completion_index = this.result.completion_start_index * 1;
						
						// The count of running request of the current query is now one less
						this.request_count--;

						// NEW 03-04-08 / 14-04-08 (Markus)
						if (this.request_count == 0)
						{
//						  info("All requests are done");
						  if (this.result.navigation_mode == "user")
						  {
						    // The current request is not a "history request" but a new user interaction and the request is the last one of this query
						    notes.add("<b>Browser history handling</b>", 0);
						    create_hash();
						  }

						  // NEW Markus / 04-07-2009: "auto more"-feature, automaticly load next hits until the page is full of hits
						  // (so the user gets a screen full of hits and the scrollbars are visible (and so the autoscroll feature can work)
						  if (AC.hits_autofill == 1)
						  {
						  // n is number of shown hits
						  var n = parseInt(this.result.H_boxes[1].fh) + parseInt(this.result.H_boxes[1].sent - 1);
						  if (n < this.result.H_boxes[1].total)
						  //            if (this.result.query_types == "H" && n < this.result.H_boxes[1].total)
						  {
						    //              info (n + ", " + this.result.H_boxes[1].total);
						    //              info (AC);
						    var content = document.getElementById("right");
						    if (content.offsetHeight + content.offsetTop <= getWindowHeight())
						    {
						      // Ask for more hits
						      var query = new Query("H", 1, this.hits_per_page_while_typing, AC.result.last_hit + 1, AC.result.first_hit);
						      this.box_navigation_mode = "A";
//						      info("more launch ...");
						      this.launchQuery(document.getElementById("autocomplete_query").value, new Array(query), true, "", true);
						      }
						    }
						  }
						}

						// NEW 31.01.07 (Markus): because showResults are no longer called, we have to call refreshNotes explicitly
						this.refreshNotes();
					}
					else
					{
						notes.add("Request #" + this.XMLHttpRequestContainers[i].request_id + " of past query #" + this.XMLHttpRequestContainers[i].query_id + " returned", 1);
//						this.log_write ("Request #" + this.XMLHttpRequestContainers[i].request_id + " of past query #" + this.XMLHttpRequestContainers[i].query_id + " returned");
						// Set the used XMLHttpRequest object to state "idle"
						this.setXMLHttpRequestIdle (this.XMLHttpRequestContainers[i].request_id);
					}
				}
			}
			catch (e)
			{
//				this.log_write ("Exception while receiving data (state = " + xmlHttpRequest.readyState + ") -- " + e, this.log.levels.FATAL);
        try
        {
          this.XMLHttpRequestContainers[i].hideProgress();
			  	this.setXMLHttpRequestIdle (this.XMLHttpRequestContainers[i].request_id);
        }
        catch(e) { }
			}
		}
	}
}


// COPY RESULT TO DIV'S
AC_Class.prototype.showResults = function()
{
	for (var member in this.result)
	{
		if (document.getElementById('autocomplete_' + member) != undefined)
		{
			document.getElementById('autocomplete_' + member).innerHTML = unescape (eval("this.result." + member));
		}
		else if (document.getElementById('autocomplete_result_' + member) != undefined)
		{
			document.getElementById('autocomplete_result_' + member).innerHTML = unescape (eval("this.result." + member));
		}
	}
	this.result.completion_index = this.result.completion_start_index * 1;
//	this.refreshNotes();
}


// Clear all box DIV's of type contained in qt
// Example: qt = "WC" --> clear W and C boxes
AC_Class.prototype.clearResults = function(qt)
{
	for (var j = 0; j < qt.length; j++)
	{
		nodes = document.getElementsByName ("box_" + qt.charAt(j));
		if (nodes != undefined) {
			for (var i = 0; i<nodes.length; i++)
			{
				nodes[i].innerHTML = "";
			}
		}
	}
}


//	Simple debug output
//
//	NEW 20-08-07 (Markus): Fatal errors are now written in the hit box title
//	NEW 16-04-08 (Markus): Setting the notes every time a log is written as whole is very time expansive. It's better to append a new DIV element.
AC_Class.prototype.log_write = function (text, level, clear)
{
  if (level == undefined)
  level = this.log.levels.ERROR;	// default

  if (level > this.log.level) {
    return;
  }

  if (level == this.log.levels.FATAL)
  {
    document.getElementById('autocomplete_H_boxes_1_title').innerHTML = text;
    document.getElementById('autocomplete_H_boxes_1_body').innerHTML = "";
    // If Firebug is installed and enabled write to its console
		//if (console != null) console.log(text);
  }

  if (document.getElementById("autocomplete_info_boxes_1_body") == null) return;

  if (clear != undefined)
  {
    document.getElementById("autocomplete_info_boxes_1_body").innerHTML = "";
  }

  var infoDiv = document.createElement("DIV");
  infoDiv.innerHTML = unescape(text);
  document.getElementById("autocomplete_info_boxes_1_body").appendChild(infoDiv);
}



/*
	Create a XMLHttpRequest.
*/
AC_Class.prototype.createRequest = function ()
{
	// Microsoft Internet Explorer uses ActiveXObject
	if (window.ActiveXObject)
	{
		try
		{
			this.xmlHttp = new ActiveXObject("Msxml12.XMLHTTP");
		}
		catch (e)
		{
			try
			{
				this.xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch (e) {
				this.log_write ("Could not create the ActiveXObject object", this.log.levels.FATAL);
			}
		}
	}
	// All other browsers use XMLHttpRequest
	else if (window.XMLHttpRequest)
	{
		try
		{
			this.xmlHttp = new XMLHttpRequest();
		}
		catch (e) {
			this.log_write ("Could not create the XMLHttpRequest object", this.log.levels.FATAL);
		}
	}
	// NEW 12.12.06
	return this.xmlHttp;
}


/*
	Check whether the AJAX request returned.
*/
AC_Class.prototype.callInProgress = function (xmlHttp)
{
	return (xmlHttp.readyState > 0 && xmlHttp.readyState < 4);
}


/*
	Send a notification that javascript seems to be enabled in user's browser.
*/
AC_Class.prototype.JSNotification = function ()
{
// Notes area don't exist when this log is called
//	this.log_write ("[Send a notification that javascript seems to be enabled in user's browser]", this.log.levels.INFO);
	if (! this.xmlHttp) {
		this.xmlHttp = this.createRequest();
	}
	// Start request
	if (this.xmlHttp)
	{
		if (this.callInProgress(this.xmlHttp) && (this.browser == "Netscape" || this.browser == "Opera"))
		{
			// TODO Was bringt diese Zeile?
			this.isAbort = true;
			this.xmlHttp = this.createRequest();
			this.log_write ("New request object created (because of Mozilla/Opera bug)", this.log.levels.DEBUG);
		}

		try
		{
			// Open url of PHP script which process this query and send header
                        // NEW 27Jun13 (baumgari): The request needs to be sent synchron, since
                        // elsewhise we might abort the request by recalling the
                        // javascript (which is done as long the javascript
                        // variable is false).
			this.xmlHttp.open( "POST", this.autocomplete_url + "/js_notification.php", false);
			this.xmlHttp.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );

			this.isAbort = false;

			// Send the query
			this.xmlHttp.send("session_name=" + escape (this.session_name));
		}
		catch (e)
		{
			this.log_write ("Error while sending a notification that javascript seems to be enabled: " + e, this.log.levels.ERROR);
		}
	}
}


/*
	Perform an unset of the server session to clear all data about the current search context
*/
AC_Class.prototype.unsetSession = function ()
{
	// NEW 02.08.06 (Markus): "clear" added
	this.log_write ("[Send a request to unset session]", this.log.levels.INFO, "clear");

	// Start request
	if (this.xmlHttp)
	{
		if (this.callInProgress(this.xmlHttp) && (this.browser == "Netscape" || this.browser == "Opera"))
		{
			this.log_write ("Abort previous request (status: " + this.xmlHttp.readyState + ")", this.log.levels.DEBUG);
			// TODO Was bringt diese Zeile?
			this.isAbort = true;
			this.createRequest();
			this.log_write ("New request object created (because of Mozilla/Opera bug)", this.log.levels.INFO);
		}

		try
		{
			// Open url of PHP script which process this query and send header
			this.xmlHttp.open( "POST", this.url + "/unset_session.php", true );
			this.xmlHttp.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );

			this.isAbort = false;

			// The callback function
			this.xmlHttp.onreadystatechange = this.sessionUnset;

			// Start animated gif to show progress
//			this.showProgress();

			// Send the query
			this.xmlHttp.send("session_name=" + escape (this.session_name));
		}
		catch (e)
		{
			this.log_write ("Error while unsetting session via XMLHttp: " + e, this.log.levels.ERROR);
		}
	}
}



/*
	Get the text pattern with identifier id and replace occurencies of %s by additional parameters of getText
	For example: pattern1 = "This is an %s", getText("pattern1", "example") --> "This is an example".
*/
AC_Class.prototype.getText = function (item)
{
//	var s = this.result.text[item];
	var s = this.text[item];

	if (s == undefined)
	{
		// To prevent recursive cals of get_text we use the following text without translation from $this->text array
		this.log_write("Text pattern for " + item + " is missing", this.log.levels.ERROR);
//		this.log_write (this.getText("error_text_missing"), this.log.levels.ERROR);
		return "";
	}
	else
	{
		// Note: getText is the name of this function, arguments the array of all calling argument
		for (var i = 1; i < this.getText.arguments.length; i++)
		{
			s = s.replace(/%s/, this.getText.arguments[i]);
		}
		return s;
	}
}

/*
	Do the following when the user clicks on a entry in the completions list
*/
AC_Class.prototype.completion_link = function (query, event)
{
  // NEW 11Sep12 (baumgari): In case ctrl is pressed, we want to negate the
  // clicked query (but not if the whole string consists of just one word or in
  // case there is a misplaced space. I assumed that a real word has at least 3
  // characters. To do that, we need to move the first word (the new query) to
  // the end, since completesearch is not able to negate a one-query-string (we
  // don't want that, since this would lead to very very large results).
  if (event.ctrlKey)
  {
    var firstSpace = query.search(' ');
    if (firstSpace > 3)
      query = query.substring(firstSpace + 1) + " -" + query.substring(0, firstSpace);
  }
	document.getElementById("autocomplete_query").value = query;
  // NEW 03-04-08 (Markus): Prevents the more links to start with first_hit different from 1 after having navigated through hits using page up/down
	this.first_hit = 1;
//	this.first_hit_shown = 1;

//  this.log_write("CLICKED COMPLETION LINK", this.log.levels.INFO, "clear");
  this.log_write("Interaction", "User clicked completion link", this.log.levels.INFO);

	// Save the timestamp of this click event
	this.history.time = new Date().getTime();

  // NEW 16-03-08 (Markus): set inactivity to 1000 sec to force a history entry
  this.history.inactivity = 1000000;
  // Wieso hat das mit getElementById("autocomplete_query") geklappt?
  this.launchQuery(query, this.getDefaultQueryParameters(true), true);
//	this.launchQuery(query, this.query_types, "");
//	this.launchQuery(document.getElementById("autocomplete_query").value, this.query_types, "");

	// TODO autocomplete_query --> query
	document.getElementById ("autocomplete_query").value = query;
	setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
	//document.getElementById ("autocomplete_query").focus();
}


/*
	Do the following when the user clicks on "more" link at the end of one of the result lists (for exacomple the completions list)
*/
AC_Class.prototype.more_link = function (query_type, query_index, value)
{
  // NEW 03-04-08 (Markus): Prevents the more links to start with first_hit different from 1 after having navigated through hits using page up/down
	// Note: Maybe no longer necessary because now first_hit is passed as query parameter
  //  this.first_hit = 1;

  notes.add("<b>User clicked more link</b> (" + query_type + query_index + ")", 0);
//  this.log_write("Interaction", "User clicked more link", this.log.levels.INFO);
//  this.log_write("CLICKED MORE COMPLETIONS LINK", this.log.levels.DEBUG, "clear");
	// 08-04-08 (Markus): query_type is now passed as an array of query_parameters
  this.launchQuery(document.getElementById("autocomplete_query").value, new Array(new Query(query_type, query_index, value)), true);
// 	this.launchQuery(document.getElementById("autocomplete_query").value, query_type, query_index, true, "mcs", value);

	setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
  //document.getElementById ("autocomplete_query").focus();
}


/*
	Do the following when the user clicks on one of the navigation links
*/
AC_Class.prototype.navigation_link = function (first_hit, box_navigation_mode, first_hit_shown)
{
  // Markus / 29-12-08
  if (typeof first_hit_shown == "undefined") {
  	first_hit_shown = first_hit;
  }
  // Markus / 29-12-08
  if (typeof box_navigation_mode == "undefined") {
  	this.box_navigation_mode = "R";
  }
  else {
  	this.box_navigation_mode = box_navigation_mode;
  }
  
  // Markus / 29-12-08: if this is an "append" hits request we replace the more link by an animated progress image
  if (this.box_navigation_mode == "A") 
  {
    var link = document.getElementById("more_hits_link");
    if (link) {
      link.style.display = "none";
    }
    var img = document.getElementById("progress_image_H_more");
    if (img) {
      img.style.display = "block";
      notes.add("<b>User clicked on more hits link</b>", 0);
    }
  }
  notes.add("<b>User clicked page up/down link</b>", 0);

	// NEW 10-04-08 (Markus)
	// Attention: the max_completions_show is here (in the case of a H query (hits)) the hits_per_page_while_typing parameter
	var query = new Query("H", 1, this.hits_per_page_while_typing, first_hit, first_hit_shown);
	
	// 08-04-08 (Markus): query_type is now passed as an array of query_parameters
	// NEW Markus / 15-04-09: the new fifth parameter (true) allows launchQuery() to fall back to previous query string 
	// if the new one is too short
  this.launchQuery(document.getElementById("autocomplete_query").value, new Array(query), true, "user", true);
//  this.launchQuery(document.getElementById("autocomplete_query").value, new Array(query), true);

  // If next line is used the browser window jump to the top to display the query input (because it gets the focus)
  // setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
}


/*
	Do the following when the user clicks on one of the language links
*/
AC_Class.prototype.language_link = function (language)
{
	this.language = language;

  notes.add("<b>User clicked language link</b>", 0);

	// 08-04-08 (Markus): query_type is now passed as an array of query_parameters
//	info("language: " + objectArrayAsString(this.getQueryParametersAsArray()))
  this.launchQuery(document.getElementById("autocomplete_query").value, this.getQueryParametersAsArray(), true);

  var time = new Date();
	var tmp = time.getTime() + this.cookie_expiration;
	time.setTime(tmp);

	if (language == 'de')
	{
		document.getElementById ("language1").href = "javascript:AC.language_link('en');";
		document.getElementById ("language1").innerHTML = 'English';
		document.getElementById ("language2").innerHTML = 'deutsch';
		}
	else if (language == 'en')
	{
		document.getElementById ("language1").href = "javascript:AC.language_link('de');";
		document.getElementById ("language1").innerHTML = 'deutsch';
		document.getElementById ("language2").innerHTML = 'English';
	}
	setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
}



/*
	Complete query by word from completion list as specified by completion_index.
*/
AC_Class.prototype.completeInput = function ()
{
	// Get completion
	if (this.result.completion_index < 0)
	{
		this.result.completion_index = 0;
	}
	if (this.result.completion_index >= this.result.completions.length)
	{
		this.result.completion_index = this.result.completions.length - 1;
	}

	// Because we got completions escaped from server (to transfer potential special characters correct)
	// the expression has to be unescaped
	var completion = unescape (this.result.completions[this.result.completion_index]);

	// SPECIAL: if completion is of type "PLthisE:aachen", remove the "PLthisE:" !!!
	if (completion.toUpperCase() != completion)
	{
		completion = completion.replace(/[A-Z]+:/, '')
	}

	// Replace last token in query by completion
	// autocomplete_query = document.getElementById('autocomplete_query').value;
	// document.getElementById('autocomplete_query').value = autocomplete_query.replace(new RegExp('[^'+this.separators+']+$'), completion);
	// NEW 01Dec11 (Ina): Clear (just replace by completion) autocomplete_query, since else every new
        // completion is going to be appended to the old one.
	document.getElementById('autocomplete_query').value = completion;
}


//! NEW 4Aug06 (Holger): PUT CURSOR AT END OF INPUT FIELD (e.g., after pressing home)
function positionCursorAtEnd()
{
  tmp = document.getElementById('autocomplete_query').value;
//  document.getElementById('autocomplete_query').value = "";
  document.getElementById('autocomplete_query').value = tmp;
}


AC_Class.prototype.history_back = function ()
{
//	for (I=0; I < this.history.size; I++) {
//		if (I == this.history.pos)
//			this.log_write(I + ': ' + this.history.elements[I] + '  <-- history.pos', this.log.levels.DEBUG);
//		else this.log_write(I + ': ' + this.history.elements[I]);
//	}
	if (this.history.pos > 0)
	{
		this.history.pos--;
//		this.log_write('histoy.pos--: ' + this.history.pos, this.log.levels.DEBUG);
    // NEW 01-08-07 (Markus): decoding is dependant of the collection encoding,
    //  so use a decode function which takes care of this instead of the utf8_decode
		words = this.charset_decode(unescape(this.history.elements[this.history.pos]));
//		words = this.utf8_decode(unescape(this.history.elements[this.history.pos]));
		document.getElementById('autocomplete_query').value = words;
		// When we move in history we have to reset first_hit and first_hit_shown to zero and first_hit_shown to 1
    // CHANGED 27-02-08 (Markus)
  	this.first_hit = 1;
  	this.first_hit_shown = 1;

	  // NEW 16-03-08 (Markus): set inactivity to 1000 sec to force a history entry
	  this.history.inactivity = 1000000;
		this.launchQuery(words, this.query_types, "");
	}
	else this.history.pos = 0;
	setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
    //document.getElementById('autocomplete_query').focus();
	  //positionCursorAtEnd();
}


AC_Class.prototype.history_forward = function ()
{
	if (this.history.pos < this.history.elements.length - 1)
	{
		this.history.pos++;
    // NEW 01-08-07 (Markus): decoding is dependant of the collection encoding,
    //  so use a decode function which takes care of this instead of the utf8_decode
		words = this.charset_decode(unescape(this.history.elements[this.history.pos]));
//		words = this.utf8_decode(unescape(this.history.elements[this.history.pos]));
		document.getElementById('autocomplete_query').value = words;
		// When we move in history we have to reset first_hit and first_hit_shown to zero and first_hit_shown to 1
    // CHANGED 27-02-08 (Markus)
  	this.first_hit = 1;
  	this.first_hit_shown = 1;

	  // NEW 16-03-08 (Markus): set inactivity to 1000 sec to force a history entry
	  this.history.inactivity = 1000000;
  	this.launchQuery(words, this.query_types, "");
	}
	setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
	  //document.getElementById('autocomplete_query').focus();
	  //positionCursorAtEnd();
}


AC_Class.prototype.setTimestamp = function (name)
{
	this.result.js_times[this.result.js_times.length] = name + "|" + (new Date().getTime() - this.result.js_times[0]);
}


AC_Class.prototype.refreshNotes = function ()
{
//	alert(notes.toString());
//	console.log(notes.toString());
//	console.log(notes.toDOM());
//	document.getElementById("autocomplete_info_boxes_1_body").innerHTML += notes.toDOM();

  var infobox = document.getElementById("autocomplete_info_boxes_1_body");
  if (infobox) {
    infobox.appendChild(notes.toDOM());
  }
  notes.clear();
//	if (this.log.level < this.log.levels.INFO)
////	if (this.notes == null || this.notes == "" || this.log_level.current < this.log.levels.WARNING)
////	if (this.notes == null || this.notes == "" || parseInt(this.log_level.current) < parseInt(this.log.levels.WARNING))
//	{
////		document.getElementById('autocomplete_notes_box').style.display = 'none';
//	}
//	else {
////		document.getElementById('autocomplete_notes_box').style.display = 'block';
//// UNCOMMENTED 15-04-08 (next line)
////  document.getElementById("autocomplete_info_boxes_1_body").innerHTML = this.notes + "<br>";
////		document.getElementById('autocomplete_info_boxes_1_body').style.display = 'block';
//	}
}


/*
AC_Class.prototype.updateHash = function(query_string, query_type, query_index, max_completion_show, first_hit)
{
  var hash = location.hash;
  if (hash.indexOf("query=") == -1) {
    hash = "query=" + query_string;
  }
  else hash = hash.replace(new RegExp("query=[^&]*"), "query=" + query_string);

  if (typeof(query_type) != "undefined" && query_type != "")
  {
    //		console.log("qt:" + query_type)
    var box = query_type + query_index;
    var v = (typeof(first_hit) == "undefined") ? max_completion_show : max_completion_show + "." + first_hit;
    if (hash.indexOf(box) >= 0) {
      // There is already a paramter for this box, so replace it
      hash = hash.replace(new RegExp(box + "\.[^:]*"), box + "." + v);
    }
    else {
      // First check whether there is an URL parameter for the parameters
      if (hash.indexOf("param=") == -1) {
        hash += "&param=" + box + "." + v;
      }
      else hash += ":" + box + "." + v;
    }
  }

  location.hash = hash;
  last_hash = hash;
}
*/


AC_Class.prototype.charset_decode = function(s)
{
  if (this.encoding == "utf-8") {
		return utf8_decode(s);
	}
	else return s;
}


// NEW 14Jul06 (Holger)
// Dedicated function for the AC request callback (just calls AC.requestReturned)
// NEW 24-10-07 (Markus): was member of the AC constructor (which leads to an error of non-defined AC object)
// TODO Error appear still (often after F5 / reload)
//AC_requestReturned = function()
//{
//  AC.requestReturned();
//}


function fade_out (elements, fi, keyevent)
{
  //info("fade_out: " + keyevent + " ?< " + keyevent_counter);
	if (keyevent < keyevent_counter) {
		return;
	}
  //info("fade_out: set color of " + elements);
	setColor (elements, fi, fi, fi);
}


function setColor (elements, r, g, b)
{
	var elements_array = elements.split(",");
	for (var i = 0; i < elements_array.length; i++)
	{
		nodes = document.getElementsByName(elements_array[i]);
		for (var j = 0; j < nodes.length; j++)
		{
			nodes[j].style.color = "rgb(" + r + "," + g + "," + b +")";
		}
	}
}


function addslashes(str)
{
	str=str.replace(/\'/g,'\\\'');
	str=str.replace(/\"/g,'\\"');
	str=str.replace(/\\/g,'\\\\');
	str=str.replace(/\0/g,'\\0');
	return str;
}


function stripslashes(str)
{
	str=str.replace(/\\'/g,'\'');
	str=str.replace(/\\"/g,'"');
	str=str.replace(/\\\\/g,'\\');
	str=str.replace(/\\0/g,'\0');
	return str;
}


// This event handler catches the CTRL-LEFT and -RIGHT key commands
// Prevents the standard navigation in input field with these both key command (we use these both for history navigation)
function preventCtrlLeftRight(event)
{
	// Firefox get the event object by parameter <event>, internet explorer by the window.event object
	if (!event) {
		event = window.event;
	}
	if (event.ctrlKey && (event.keyCode == 37 || event.keyCode == 39)) {
		return false;
	}
	else return true;
}


// Toggle the minimization the <i>-th box of type <type>
function toggleBox (type, i)
{
	var node = document.getElementById('autocomplete_' + type + '_boxes_' + i + '_body');
	if (node.style.display == "block" || node.style.display == '')
	{
		node.style.display = "none";
		document.getElementById('toggle_' + type + '_boxes_' + i).src = 'images/arrow_down2.gif';
	}
	else {
		node.style.display = "block";
		document.getElementById('toggle_' + type + '_boxes_' + i).src = 'images/arrow_up2.gif';
	}
}


function mouseDown (event)
{
	var mouseX;
	var posL = document.getElementById('left').offsetWidth;
	var posR = document.getElementById('detail').offsetLeft;

	// Firefox get the event object by parameter <event>, internet explorer by the window.event object
	if (!event) {
		event = window.event;
	}

	// Firefox get the mouse x-position by <pageX>, internet explorer by the <clientX>
	if (event.pageX) {
		mouseX = event.pageX;
	}
	else {
		mouseX = event.clientX;
	}

	if (mouseX)
	{
		posL1 = posL - 6;
		posL2 = posL + 6;
		// Check whether the mouse down happened in the resize area of the left box
		if (mouseX > posL1 && mouseX < posL2)
		{
			mouseTileL = true;
			// Disable the standard event handler for mouse down to prevent selecting of elements, for example
			// This one is for Firefox and Opera (for IE it doesn't work)
			return false;
		}
		else
		{
		  // Check whether the mouse down happened in the resize area of the right box
		  posR1 = posR - 6;
		  posR2 = posR + 6;
		  if (mouseX > posR1 && mouseX < posR2)
		  {
		    mouseTileR = true;
		    // Disable the standard event handler for mouse down to prevent selecting of elements, for example
		    // This one is for Firefox and Opera (for IE it doesn't work)
		    return false;
		  }
		}
	}
}


function mouseUp (event)
{
	mouseTileL = false;
	mouseTileR = false;
}


function autoScrollMoreHits (event)
{
//  info((getPageYOffset() + getWindowHeight()) / document.getElementById('page').scrollHeight);
  // A value of "0" means that autoscroll functionality is disabled
  if (AC.hits_autoscroll_threshold == 0) return;
  var page = document.body;
  // How pre-active the "more hits"-action is triggered is determined by hits_autoscroll_threshold
  if ((getPageYOffset() + getWindowHeight()) / page.scrollHeight >= AC.hits_autoscroll_threshold)
  {
    // If page is scrolled to bottom ask for more hits 
    //  (by evaluating the href of the more link located at page bottom)
    var p = document.getElementById("more_hits");
    if (p) {
      var t = p.getElementsByTagName("a");
      eval(unescape(t[0].href));
    }
  }
}


function mouseMove (event)
{
	var left = document.getElementById('left');
	var left_bg = document.getElementById('left_bg');
	var right = document.getElementById('right');
	var detail = document.getElementById('detail');
	var detail_bg = document.getElementById('detail_bg');
	var page = document.getElementById('page');
	var mouseX;
	var posL = left.offsetWidth;
	var posR = detail.offsetLeft;

	// Firefox get the event object by parameter <event>, internet explorer by the window.event object
	if (!event) {
		event = window.event;
	}

	// Firefox get the mouse x-position by <pageX>, internet explorer by <clientX>
	if (event.pageY) {
		mouseY = event.pageY;
	}
	else {
		mouseY = event.clientY;
	}
	
	// Firefox get the mouse x-position by <pageX>, internet explorer by <clientX>
	if (event.pageX) {
		mouseX = event.pageX;
	}
	else {
		mouseX = event.clientX;
	}

	if (mouseX)
	{
		posL1 = posL - 6;
		posL2 = posL + 6;
		posR1 = posR - 6;
		posR2 = posR + 6;
		if (mouseX > posL1 && mouseX < posL2)
		{
		  left.style.cursor = 'e-resize';
		  right.style.cursor = 'e-resize';
		  page.style.cursor = 'e-resize';
		}
		else
		{
		  if (mouseX > posR1 && mouseX < posR2)
		  {
		    right.style.cursor = 'e-resize';
		    page.style.cursor = 'e-resize';
		  }
		  else
		  {
		    left.style.cursor = 'default';
		    right.style.cursor = 'default';
		    detail.style.cursor = 'default';
		    page.style.cursor = 'default';
		  }
		}

    // Changing width of left box
    //
		if (mouseTileL && mouseX > 300 && mouseX < page.offsetWidth - detail.offsetWidth)
		{
		  // Difference between new and old value of width of left box
      var dw = left.offsetWidth - mouseX;

			// Note: style properties like left, width etc. needs a dimension in standard mode
      left.style.width = mouseX - (getStyle(left, 'paddingLeft') + getStyle(left, 'paddingRight')) + "px";
      left_bg.style.width = left.offsetWidth + "px";

      // Update the padding-left
      right.style.paddingLeft = (getStyle(right, 'paddingLeft') - dw) + "px";

      // Update the height of the hit box and detail box
      resizeHeight();
    }

    // Changing width of detail box
    //
		if (mouseTileR && mouseX > left.offsetWidth && page.offsetWidth - mouseX > 200)
		{
		  // Difference between new and old value of width of detail box
		  var dw = detail.offsetWidth - (page.offsetWidth - mouseX);

			// Note: style properties like left, width etc. needs a dimension in standard mode
		  detail.style.width = page.offsetWidth - (mouseX + getStyle(detail, 'paddingLeft') + getStyle(detail, 'paddingRight')) + 'px';
		  detail_bg.style.width = detail.offsetWidth + 'px';

      // Update the padding-right
//      if(getBrowser() != 'ie') {
        right.style.paddingRight = (getStyle(right, 'paddingRight') - dw) + "px";
//      }

      // Update the height of the hit box and detail box
      resizeHeight();
		}
	}

  // Disable the standard event handler for mouse down to prevent selecting of elements, for example
	return false;
}


/**
 * Move the <i>-th box of type <type> to the right and maximize it
 */
function minmaximize(type, i)
{
  var box = document.getElementById("autocomplete_" + type + "_boxes_" + i);
  var page = document.getElementById("page");
  var left = document.getElementById("left");
  var right = document.getElementById("right");
  var detail = document.getElementById("detail");
  var detail_bg = document.getElementById("detail_bg");

  if(box.parentNode == detail)
  {
    // We have to move the box back to the left

    // Set the count of entries to the standard value
  	// NEW 16-01-07 (Markus): no, don't do it
//    AC.more_link(type, i, AC.max_completions_show);

    detail.removeChild(box);

    if (AC.detail_box.next_sibling == null) {
      left.appendChild(box);
    }
    else left.insertBefore(box, AC.detail_box.next_sibling);

    AC.detail_box.query_type = null;

    document.getElementById("autocomplete_" + type + "_boxes_" + i + "_body").style.height = "7.2em";
    document.getElementById("move_" + type + "_boxes_" + i).src = 'images/box_max.gif';

    // Reduce the padding-right of the right box by the width of the detail box
    // (right box should fill the space is created by disappearing detail box)
    right.style.paddingRight = (getStyle(right, "paddingRight") - detail.offsetWidth) + "px";

    detail.style.display = "none";
    detail_bg.style.display = "none";
  }
  else
  {
    // We have to move the box to the right and to enlarge
    AC.detail_box.query_type = type;

    // Check if there is already another box at the right
    var child = detail.firstChild;
    if (child != undefined)
    {
      detail.removeChild(child);
      // Append it to its former place
      if (AC.detail_box.next_sibling == null) {
        left.appendChild(child);
      }
      else left.insertBefore(child, AC.detail_box.next_sibling);

      child.getElementsByTagName("div")[1].style.height = "7.4em";
      child.getElementsByTagName("img")[1].src = 'images/box_max.gif';
    }
    else {
      // Show detail box
      detail.style.display = "block";
      detail_bg.style.display = "block";

      // Set the width of the detail box same value as the width of the left one
      detail.style.width = left.offsetWidth - getStyle(left, 'paddingLeft') - getStyle(left, 'paddingRight') + "px";
        //detail.style.width = left.offsetWidth + "px";
      detail_bg.style.width = detail.offsetWidth + "px";

      // For non-internet explorer the padding-right must be set new
      // because for them the padding-right is related to the right border of the whole page (internet explorer is left border of the detail box)
//      if(getBrowser() != 'ie') {
        right.style.paddingRight = (detail.offsetWidth + getStyle(right, 'paddingRight')) + "px";
//      }
    }

    // Before move it to right, store its right DOM tree neighbor (that's the box below it)
    AC.detail_box.next_sibling = box.nextSibling;
    left.removeChild(box);
    detail.appendChild(box);

    // Compute the height h of the content area of the completion/info box which is moved to the right
    var title_height = box.getElementsByTagName("table")[0].offsetHeight;
    var h = detail.offsetHeight - title_height - 10;

//    var th = document.getElementById("autocomplete_" + type + "_boxes_" + i + "_body").getElementsByTagName("div")[0].offsetHeight;
//    AC.detail_box.max_completions_show = Math.floor((h-2) / (th+1))-1;

    box.style.height = "100%";
    document.getElementById("autocomplete_" + type + "_boxes_" + i + "_body").style.height = "100%";
//    box.style.height = (detail.offsetHeight - 30) + "px";
//    document.getElementById("autocomplete_" + type + "_boxes_" + i + "_body").style.height = h + "px";
    document.getElementById("move_" + type + "_boxes_" + i).src = 'images/box_min.gif';

    // Re-compute the completions for the box, if it's not the info box
    if (type != 'info') {
      AC.more_link(type, i, AC.max_completions_show_right);
    }
  }

  // Resize the height of the hit box and detail box
  resizeHeight();

  // Set focus to input field
	setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
    //document.getElementById('autocomplete_query').focus();
}


function resizeHeight()
{
	var detail_box = document.getElementById("detail");
	if (detail_box)
	{
		var hw = getWindowHeight();
		// Resize height of the hit box
		//  var h1 = document.getElementById("autocomplete_H_boxes_1_title").offsetHeight;
		//  var h2 = document.getElementById("autocomplete_H_boxes_1_footer").offsetHeight;
		//  document.getElementById("autocomplete_H_boxes_1_body").style.height = (hw - h1 - h2 - 56) + "px";

		// Set the height of the detail box to full page height
		document.getElementById("detail").style.height = (hw - 40) + "px";
	}
}


// Whenever javascript is enabled and the user try to send a query by using the return key
// this must happen by a javascript call (to prevent the URL parameter autocomplete_query to be set in URL).
// As long as possible we have to use the AJAX mechanism.
// If we wouln'd do so the history back mechanism in javascript mode woul'd be work because of parameter in URL
// which override the corresponding AC object values.
// So this function always returns false which means the form is not to be sent so long javascript is enabled
function always_return_false()
{
	return false;
}

/*
function pp(text)
{
  document.getElementById("autocomplete_info_boxes_1_body").innerHTML += text + "<br>";
}
*/

// NEW 31Dec07 (Holger)
// Set focus to the given input field and move cursor to end
function setFocusAndMoveCursorToEnd(input)
{
  input.focus();
  if (input.createTextRange)
  {
    var v = input.value;
    var r = input.createTextRange();
    r.moveStart('character', v.length);
    r.select();
  }
}


function showNote(topic, title, body)
{
  var e = document.getElementById("note");
  if (e) {
    e.innerHTML = "<a title=\"" + AC.text["note_hint"] + "\" href=\"" + AC.autocomplete_path + "note.php?session_name=" + AC.session_name + "&path=" + AC.index_path + "&page=" + AC.index_page + "&log=" + AC.error_log + "&note_topic=" + topic + "&note_title=" + title + "&note_body=" + body + "\"><img src=\"images/note.gif\" border=\"0\"></a>";
  }
}


function init()
{
    // Inform server that javascript is enabled
    AC.JSNotification();
    notes.add("Initialize Javascript ...", 0);

    // If index.php is called the first time and AC.javascript is false, all completion and more/less links are non-Javascript links.
    //	To prevent conflicts between these kind of links and the hash mechanismen for history navigation we reload index.php.
    //	NEW 10Jan13 (baumgari): In case of disabled cookies the page was
    //	reloaded nonstop, since the init function was called again and again.
    if (AC.javascript == false && AC.cookies == true)
    {
      notes.add("Reloading index.php ...", 1);
      location.href = "index.php" + location.hash;
    }

    // Check compatibility mode
    if (getCompatibilityMode() != 'standard' && AC.note.quirks_mode == 1) {
      showNote("quirks_mode", "quirks_mode_note_title", "quirks_mode_note_body");
    }

    // Set the cursor at the end of the input field
    // and remove the "?", "!" from the query string.
    setFocusAndMoveCursorToEnd(document.getElementById("autocomplete_query"));
    document.getElementById('autocomplete_query').value = document.getElementById('autocomplete_query').value.replace(/[\!\?]$/,'');

    // Show progress images if this option (member of class Settings) is set to true
    if (AC.show_progress_images)
    {
        var qt = AC.query_types;
        for (var j=0; j < qt.length; j++)
        {
            nodes = document.getElementsByName ("progress_image_" + qt.charAt(j));
            if (nodes != undefined) {
                for (var i=0; i<nodes.length; i++)
                {
                    nodes[i].style.display = "block";
                }
            }
        }
    }

  // The next if statement handle the case that the index.php is called from an external link with parameters set 
  //	(this can be a bookmark, for example).
  //	For IE the history frame is generated; the onFrameLoaded method calls the queries for the hash parameters.
  //	For Firefox we set the navigation_mode to "history" (to simulate a browser back or forward) and start the Firefox hash check timer; 
  //	so the queries are called by the check_hash method.
  if (location.hash != "")
  {
  	notes.add("Hash: " + location.hash, 1);
  	// Page is called with an URL hash
  	// NEW 18-04-08 (Markus): At beginning the content of the (hidden) iframe for IE must be set to the hash with which the page is called
  	// NEW Markus / 13-01-09
  	if (getBrowser().name == "ie" && getBrowser().version < 8)
  	{
  		notes.add("IE: write hash to history frame", 1)
  		var doc = document.getElementById("historyFrame").contentWindow.document;
  		var hash = location.hash;
  		if (hash.charAt(0) == "#") {
  			hash = hash.substring(1);
  		}

  		doc.open("javascript:'<html></html>'");
  		doc.write("<html><head><script type=\"text/javascript\">parent.onFrameLoaded('" + hash + "');<\/script></head><body></body></html>");
  		doc.close();
  	}

  	else
  	{
  		// Set the navigation_mode to "history" (to simulate a browser back or forward)
  		// TODO Hack: warum klappt das nicht bei Opera?
    	// NEW Markus / 11-01-09
//  		if (getBrowser().name == "firefox") {
  		  AC.result.navigation_mode = "history";
//  		}
//  		info("navigation_mode set to '" + AC.result.navigation_mode + "'");
  	}
  }
  
	// NEW Markus / 13-01-09
	if (getBrowser().name != "ie" || getBrowser().name == "ie" && getBrowser().version > 7)
  {
  	// Start the timer for non-IE browser to check url for a changed hash
  	setInterval("check_hash()", 200);
  	notes.add("Non-IE history: hash-check timer set to 200 ms", 1);
  }
}



/**
 *	Get defaults of all query parameters as array of Query objects
 *
 */
AC_Class.prototype.getDefaultQueryParameters = function(onclick)
{
  var result = new Array();

  for (var i = 0; i < this.query_types.length; i++)
	{
	  var type = this.query_types.charAt(i);
	  //info("type=" + type);
	  if (type == "H") {
            if (onclick)
	        result["H1"] = new Query("H", "1", this.hits_per_page_on_click, "");
            else
	        result["H1"] = new Query("H", "1", this.hits_per_page_while_typing, "");
	    //result.push(new Query(type, "", this.hits_per_page_while_typing, ""));
	  }
	  else {
	    if (type == "F" && this.facets_to_show) 
	    {
	      // NEW Markus / 10-01-09: this new "for .. in" loop works for arrays AND for objects (the version below did it only for arrays)
	      for (var j in this.facets_to_show) {
	        result["F" + (parseInt(j) + 1)] = new Query("F", (parseInt(j) + 1), this.max_completions_show, "");
	      }
//			for (var j = 1; j <= this.facets_to_show.length; j++) {
//			 result["F" + j] = new Query("F", j, this.max_completions_show, "");
//			}
	    }
	    else
	    {
	      result[type + "1"] = new Query(type, "1", this.max_completions_show, "");
	      //result.push(new Query(type, "", this.max_completions_show, ""));
	    }
	  }
	}
	//  for(o in result) console.log(result[o]);
	return result;
}


/**
 *	Get state of all query parameters as array of Query objects
 *
 */
AC_Class.prototype.getQueryParametersAsArray = function()
{
	var result = new Array();
	for (var i = 0; i < this.query_types.length; i++)
	{
		var type = this.query_types.charAt(i);
//		info("type: " + type);

		var boxes = eval("AC.result." + type + "_boxes");
		if (typeof(boxes) != "undefined")
		{
			var qi = 1;
			for (var box in boxes)
			{
//			  info("box: " + boxes[box]);
				// Markus / 27-11-08: "count" changed to "sent"
        var count = boxes[box]["sent"];
//				var count = boxes[box]["count"];
				if (typeof(count) != "undefined")
				{
					var fh = boxes[box]["fh"];
					result.push(new Query(type, qi, count, fh));
				}
				qi++;
			}
		}
	}
//	info ("result: " + result);
	return result;
}


/**
*	Get state of query parameters as string (used for the url hash)
*
*/
AC_Class.prototype.getQueryParametersAsString = function()
{
        var result = "";
	for (var i = 0; i < AC.query_types.length; i++)
	{
		var type = AC.query_types.charAt(i);
		var boxes = eval("AC.result." + type + "_boxes");
		if(typeof(boxes) != "undefined")
		{
			var qi = 1;
			for (var box in boxes)
			{
				var count = boxes[box]["sent"];
				if (typeof(count) != "undefined")
				{
					var fh = boxes[box]["fh"];
					result += type + box + "." + count + ((fh == 1 || fh == "") ? "" : "." + fh) + ":";
//					result += type + qi + "." + count + ((fh == 1 || fh == "") ? "" : "." + fh) + ":";
				}
				qi++;
			}
		}
	}

  if (result.length > 0) {
		// Remove the last ":"
    return result.slice(0, result.length - 1);
  }
  else {
    return "";
  }
}


/**
 *	Set the value of the query input to value
 *
 */
AC_Class.prototype.setQueryInput = function(value)
{
	document.getElementById("autocomplete_query").value = value;
}


/**
 *	Return the value of the query input
 *
 */
AC_Class.prototype.getQueryInput = function()
{
	return document.getElementById("autocomplete_query").value;
}


/**
 *	Toggle the visibility of sub notes in the infobox between visible and invisible
 *
 */
function toggleNote(note, more, event)
{
	if (note.style.display == "block")
	{
		note.style.display = "none";
		more.className = "li-plus"
	}
	else
	{
		note.style.display = "block";
		more.className = "li-minus"
	}
	// Prevent all super ordinated note from handling the click too
	if (event) {
		event.cancelBubble = true;
	}
	else window.event.cancelBubble = true;
}


function clearInfo()
{
  notes.clear();

  var infobox = document.getElementById("autocomplete_info_boxes_1_body");
  if (infobox) {
    infobox.innerHTML = "";
  }
}
