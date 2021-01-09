/**
 *  Markus 26-06-2009: new functionality; Notes.add() has got a third parameter 'expanded'.
 *  If expanded is true the node is shown expanded, if not it is collapsed (as it was before).
 *  Default is collapsed (so it is displayes as before),
 */

Notes = function()
{
	this.all_notes = new Array();		// contains all notes of level 0
	this.current_notes = new Array();	// contains the first note of every level
}


Notes.prototype.add = function(text, level, expanded)
{
  if (! expanded) {
    expanded = false;
  }
  // TODO Check why AC is after a reload not defined!
  if (typeof(AC) !== "undefined" && AC.log_level > 3)
  {
    if (level == 0)
    {
      this.current_notes[0] = new Note();
      this.current_notes[0].text = text;
      this.current_notes[0].expanded = expanded;
      this.all_notes.push(this.current_notes[0]);
    }
    else {
      if (this.current_notes[level-1] == null) {
        // error
      }
      else {
        this.current_notes[level] = new Note();
        this.current_notes[level].text = text;
        this.current_notes[level].expanded = expanded;
        if (this.current_notes[level-1].subnotes == null) {
          this.current_notes[level-1].subnotes = new Array(this.current_notes[level]);
        }
        else {
          this.current_notes[level-1].subnotes.push(this.current_notes[level]);
        }
      }
    }
  }
}


Notes.prototype.clear = function()
{
	this.all_notes = new Array();		// contains all notes of level 0
	this.current_notes = new Array();	// contains the first note of every level
}


Notes.prototype.toString = function()
{
	var r = "<ul id='log'>";
	for (var i in this.all_notes) {
		r += this.all_notes[i].toString();
	}
	return r + "</ul>";
}


Note = function()
{
	this.text = "";
	this.subnotes = null;
	this.expanded = true;
}


Note.prototype.toString = function()
{
	r = "<li>" + this.text;
	if (this.subnotes != null)
	{
		r += "<ul>";
		for (var i in this.subnotes) {
			r += this.subnotes[i].toString();
		}
		r += "</ul>";
	}
	return r + "</li>";
}


Notes.prototype.toDOM = function()
{
//	alert("toDOM")
	var r = document.createElement("ul");
	r.id = 'log';
	for (var i in this.all_notes) {
		r.appendChild(this.all_notes[i].toDOM());
	}
	return r;
}


Note.prototype.toDOM = function()
{
	var r = document.createElement("li");
	r.innerHTML = this.text;
	if (this.subnotes != null)
	{
		var ul = document.createElement("ul");
		
		if (this.expanded) {
		  r.className = "li-minus";
		  ul.style.display = "block";
		} else {
		  r.className = "li-plus";
		  ul.style.display = "none";
		}
//		ul.style.display = this.expanded ? "block" : "none";
		
		for (var i in this.subnotes) {
			ul.appendChild(this.subnotes[i].toDOM());
		}
		r.appendChild(ul);
		r.onclick = function(event){toggleNote(ul, r, event);}
	}
	return r;
}


/*
var notes = document.getElementById("autocomplete_info_boxes_1_body");
var currentNote = null;
var currentNote = document.createElement("DIV");
var currentIndex = 0;
*/


function add(nodeTitle, visible)
{
	var note = document.createElement("DIV");
//	var title = document.createTextNode(nodeTitle);
	var title = document.createElement("DIV");
//	var content = document.createElement("DIV");

	note.className = (visible === false) ? "invisibleSubNote " : "visibleSubNote";
	note.id = ++currentIndex;
	title.id = ++currentIndex;
//	content.id = ++currentIndex;
	// It's very important to bound currentIndex to a local variable (here: index) before setting the onlick property
	var index = note.id;
//	var index = currentIndex;
	title.onclick = function() {noteToggle(index)};

	note.appendChild(title);
//	note.appendChild(content);

	title.innerHTML = nodeTitle;
//	title.appendChild(document.createTextNode(title));

	return note;
}


function addNote(title)
{
  var notes = document.getElementById("autocomplete_info_boxes_1_body");
  // It's very important to bound currentIndex to a local variable (here: index) before setting the onlick property
  var index = ++currentIndex;
  currentNote = document.createElement("DIV");
  currentNote.id = currentIndex;
  currentNote.onclick = function() {noteToggle(index)};
  currentNote.appendChild(document.createTextNode(title));
  notes.appendChild(currentNote);
}


function addSubNote(text)
{
	var subNote = document.createElement("DIV");
	subNote.innerHTML = text;
//	subNote.appendChild(document.createTextNode(text));
	subNote.className = "invisibleSubNote";
	currentNote.appendChild(subNote);
}

/*
function noteToggle(index)
{
//	alert(index)
	var note = document.getElementById(index);
//	alert(note.childNodes.length)
	for (var i = 1; i < note.childNodes.length; i++)
	{
		var t = note.childNodes[i];
		if (t.nodeType == 1)
		{
			t.className = (t.className == "visibleSubNote") ? "invisibleSubNote " : "visibleSubNote";
		}
	}
}
function toggleNote(note, more)
{
	if (note.style.display == "block")
	{
		more.innerHTML = "&nbsp;[+]";
		note.style.display = "none";
	}
	else
	{
		more.innerHTML = "&nbsp;[-]";
		note.style.display = "block";
	}
}
*/
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


