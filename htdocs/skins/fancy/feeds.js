/* feeds.js
 * JavaScript functions for the feed view.
 */
// XXX - Should block multiple updates from occurring in parallel.

document.addEventListener("DOMContentLoaded", init, false);

debug_window = null;

function init()
{
	window.addEventListener("keydown", handle_key, false);

	// Key bindings
	bind_key("d", toggle_details);
	bind_key("t", toggle_tools);
}

function debug(str)
{
return;
	if (debug_window == null)
	{
		debug_window = window.open("",
					"Debugging Window",
					"height=400,width=600,scrollbars,menubar");
	}
	var body = debug_window.document.childNodes[1].childNodes[1];
	body.innerHTML += str + "<br/>\n";
}

function clrdebug()
{
	if (debug_window == null)
		return;
	var body = debug_window.document.childNodes[1].childNodes[1];
	body.innerHTML = "";
}

/* createXMLHttpRequest
 * Create a new XMLHttpRequest object, hopefully in a
 * browser-independent manner.
 */
function createXMLHttpRequest()
{
	var request = false;

	/* Firefox, Safari, etc. */
	if (window.XMLHttpRequest)
	{
		if (typeof XMLHttpRequest != 'undefined')
		{
			try {
				request = new XMLHttpRequest();
			} catch (e) {
				request = false;
				debug("Error allocating new XMLHttpRequest\n");
			}
		}
	} else if (window.ActiveXObject)
	{
		/* IE */
		/* Create a new ActiveX XMLHTTP object */
		try {
			request = new ActiveXObject('Msxml2.XMLHTTP');
		} catch (e) {
			request = false;
			debug("Error allocating ActiveX XMLHTTP\n");
		}
	}
	return request;
}

/* update_feed
 * Update a feed, or all feeds.
 * This is intended to be called from inside an onclick="...", so
 * paradoxically we return 'false' if successful, and 'true' in case
 * of error.
 */
function update_feed(id)
{
clrdebug();
	// XXX - If updating one feed, ought to only clear that line
	clear_status();
debug("done clearing");

	var request = createXMLHttpRequest();
	if (!request)
		return true;

	var err;

	// reqobj: an object encapsulating everything we want to keep
	// track of during this operation
	var reqobj = {
		request: 	request,
		last_off:	0
		};

	request.open('GET',
		"update.php?id="+id+"&o=json",
		true);
	request.onreadystatechange = function(){ parse_response(reqobj) };
	request.send(null);

	return false;
}

function clear_status()
{
	var table = document.getElementById("feeds").childNodes[1];
debug("table == "+table);

	/* Iterate over each row, clearing status cell */
	for (var row = table.firstChild; row != table.lastChild; row = row.nextSibling)
	{
		if (row.firstChild == null)
			/* Skip #text nodes */
			// XXX - There must be a better way to do this
			continue;
debug("row == "+row.firstChild);
		row.firstChild.innerHTML = "";
		row.firstChild.style.backgroundColor = null;
	}
}

/* XXX - Bug: if there's an error in the backend script or something,
 * we can get into a state where a feed's indicator is spinning, but
 * there's never been a line to stop it.
 * When get to the end of the query's text (readyState 4), ought to
 * find these and turn them off. Perhaps use an error indicator.
 */
function parse_response(req)
{
	debug("parse_response readyState: " + req.request.readyState);
	switch (req.request.readyState)
	{
	    case 0:		// Uninitialized
	    case 1:		// Loading
	    case 2:		// Loaded
		break;
	    case 3:		// Got partial text
	    case 4:		// Got all text
		/* Get text from where we stopped last time to the end
		 * of what we've got now.
		 */
		var str = req.request.responseText.substr(req.last_off)

		/* Remember how much of the string we've gotten so far */
		req.last_off = req.request.responseText.length;

		/* Split the current input into lines */
		// XXX - This doesn't quite work: sometimes the buffer
		// contains part of an object. Perhaps ought to look
		// for /{.*}\n/g in the buffer.
		var lines = str.split("\n");
		for (var i = 0; i < lines.length; i++)
		{
			var line = lines[i];
if (line != "")
debug("line " + i + ": [" + lines[i] + "]");
			if (line[0] != '{')
				// There might be non-JSON lines in there.
				// Ignore them. For that matter, ignore
				// any JSON lines that aren't objects.
				continue;
			try {
				eval("l = " + line);
			} catch (e) {
				debug("Caught error " + e);
				continue;
			}

			if (l.feed_id == undefined)
			{
				debug("Error: no feed_id");
				continue;
			}

			var feed_row = document.getElementById("feed-" + l.feed_id);
			if (feed_row == undefined)
			{
				debug("Error: can't find row feed-" + l.feed_id);
				continue;
			}

			var status_cell = feed_row.firstChild;
			var title_cell = status_cell.nextSibling;

debug("feed id "+l.feed_id+", state "+l.state);
			// XXX - Paths to images / skin name shouldn't
			// be hardcoded. Perhaps add a class to the cell.
			switch (l.state)
			{
			    case 'start':
				status_cell.innerHTML = '<img src="' +
					skin_dir +
					'/Loading.gif"/>';
				break;
			    case "end":
				status_cell.innerHTML = '&nbsp;';
				status_cell.style.backgroundColor = null;
				title_cell.innerHTML = l.count_display;
				break;
			    case "error":
				// XXX - The 'alt=' and/or 'title='
				// means you can get error message by
				// hovering pointer over the error
				// icon, but it'd probably be better
				// to use a CSS-based tooltip.
				status_cell.innerHTML =
					'<img src="' +
					skin_dir + 
					'/Attention_niels_epting.png" title="' +
					l.error +
					'" alt="' +
					l.error +
					'"/>';
				break;
			    default:
				status_cell.innerHTML = l.state;
				break;
			}
		}
		break;
	    default:
		/* This should never happen */
		break;
	}
}

function toggle_details()
{
	var details = document.getElementsByClassName("feed-details");

	show_details = !show_details;

	if (show_details)
	{
		for (var i = 0, len = details.length; i < len; i++)
			remove_class(details[i], "hidden");
	} else {
		for (var i = 0, len = details.length; i < len; i++)
			add_class(details[i], "hidden");
	}
}

function toggle_tools()
{
	var tools = document.getElementsByClassName("feed-tools");

	show_tools = !show_tools;

	if (show_tools)
	{
		for (var i = 0, len = tools.length; i < len; i++)
			remove_class(tools[i], "hidden");
	} else {
		for (var i = 0, len = tools.length; i < len; i++)
			add_class(tools[i], "hidden");
	}
}

// XXX - Since these are duplicated in view.js, they should live in
// only one file.
/* ----------------------------------------
 * Functions for manipulating a DOM element's class list.
 *
 * All of these functions take a DOM element with an attribute of the
 * form class="foo bar baz", and add/remove/manipulate the class list.
 */

/* is_in_class
 * Returns true iff 'cls' is in the class list of 'elem'.
 */
function is_in_class(elem, cls)
{
	var class_str = elem.className;

	if (class_str == undefined)
		// No class set
		return false;

	return class_str.match(new RegExp('(^|\\s)'+cls+'($|\\s)'));
}

/* add_class
 * Make sure 'cls' is on the class list of 'elem', adding it if
 * necessary.
 */
function add_class(elem, cls)
{
	var class_str = elem.className;

	if (class_str == undefined)
		// No class set
		return false;

	var classes = class_str.split(" ");
			// Split class string into a list

	for (var i in classes)
	{
		if (classes[i] == cls)
			// Element already has this class
			return;
	}

	// Need to add the class
	elem.className = classes.concat([cls]).join(" ");
}

/* remove_class
 * Remove 'cls' from the class list of 'elem'.
 */
function remove_class(elem, cls)
{
	var class_str = elem.className;

	if (class_str == undefined)
		// No class set
		return false;

	var classes = class_str.split(" ");
			// Split class string into a list
	var new_classes = new Array();
			// The new class list

	for (var i in classes)
	{
		if (classes[i] == cls)
			// Skip over the class we're removing
			continue;
		new_classes.push(classes[i]);
			// Remember this other class
	}

	// The new class list is whatever we're left with.
	elem.className = new_classes.join(" ");
}

/* replace_class
 * Replaces 'old_class' with 'new_class' in the class list of 'elem'.
 * This is logically equivalent to
 *	remove_class(elem, old_class);
 *	add_class(elem, new_class);
 * but is more efficient, since it combines the two.
 */
function replace_class(elem, old_class, new_class)
{
	var class_str = elem.className;

	if (class_str == undefined)
		// No class set
		return false;

	var classes = class_str.split(" ");
			// Split class string into a list
	var new_classes = new Array();
	var seen_new_class = false;
			// Have we seen the new class already?

	for (var i in classes)
	{
		if (classes[i] == old_class)
			// Don't add old_class to new_classes
			continue;
		if (classes[i] == new_class)
			// Note that we've seen new_class on the way
			seen_new_class = true;
		new_classes.push(classes[i]);
	}

	// If new_class is already on the class list, don't add it a
	// second time.
	if (!seen_new_class)
		new_classes.push(new_class);

	// The new class list is whatever we're left with.
	elem.className = new_classes.join(" ");
}

/* toggle_class
 * If 'elem' has 'classA', replace that with 'classB', and vice-versa.
 * This is logically equivalent
 */
function toggle_class(elem, classA, classB)
{
	var class_str = elem.className;

	if (class_str == undefined)
		// No class set
		return false;

	var classes = class_str.split(" ");
			// Split class string into a list
	var new_classes = new Array();
	var toggled = false;
			

	for (var i in classes)
	{
		if (classes[i] == classA)
		{
			// We've found classA
			if (toggled)
				// We've already seen classA or classB.
				// Don't add this one a second time.
				continue;

			// We're toggling from classA to classB, so
			// put classB on the new list of classes.
			new_classes.push(classB);
			toggled = true;
				// Remember that we've done this
			continue;
		}
		if (classes[i] == classB)
		{
			// We've found classB
			if (toggled)
				// We've already seen classA or classB.
				// Don't add this one a second time.
				continue;

			// We're toggling from classB to classA, so
			// put classA on the new list of classes.
			new_classes.push(classA);
			toggled = true;
				// Remember that we've done this
			continue;
		}
		new_classes.push(classes[i]);
	}

	// The new class list is whatever we're left with.
	elem.className = new_classes.join(" ");
}
/* ---------------------------------------- */

// XXX - Key-related functions are duplicated in view.js, so they
var key_box = undefined;	/* Debugging key events */

// should be in a separate file.
/* keytab
 * This is the main table for mapping keystrokes to functions.
 * It's actually a 5-dimensional array, with the first four dimensions
 * being booleans, keyed on the various modifier flags: Ctrl, Shift,
 * Meta, and Alt. The fifth dimension is the keycode found in key
 * events.
 */
var keytab = [];
for (var ctrl = 0; ctrl <= 1; ctrl++)
{
	keytab[ctrl] = [];
	for (var shift = 0; shift <= 1; shift++)
	{
		keytab[ctrl][shift] = [];
		for (var meta = 0; meta <= 1; meta++)
		{
			keytab[ctrl][shift][meta] = [];
			for (var alt = 0; alt <= 1; alt++)
			{
				keytab[ctrl][shift][meta][alt] = [];
			}
		}
	}
}

/* bind_key
 * Similar to define-key in Emacs. 'key' is a human-readable string
 * defining a key combination, and 'func' is a function to call when
 * that key is pressed.
 *
 * 'key' can be a letter, with optional modifiers:
 *	x		The letter 'x'
 *	X		Shift-X
 *	S-x		Shift-X
 *	M-x		Meta-X
 *	C-x		Ctrl-X
 *	A-x		Alt-X
 * Modifiers may be combined:
 *	M-S-x		Meta-Shift-X
 *	A-C-M-S-x	Alt-Ctrl-Meta-Shift-X
 * Unfortunately, order matters.
 */
function bind_key(key, func)
{
	var matches;

	/* Extract the key definition */
	matches = /^(A-)?(C-)?(M-)?(S-)?(.)/.exec(key);
		// XXX - Error-checking

	var alt   = (matches[1] ? true : false);
	var ctrl  = (matches[2] ? true : false);
	var meta  = (matches[3] ? true : false);
	var shift = (matches[4] ? true : false);
	var ltr   = matches[5];

	if (ltr.toLowerCase() != ltr)
		// Special case: "S-x" and "X" are the same thing.
		shift = true;

	/* Bind the key to the function */
	keytab[ctrl+0][shift+0][meta+0][alt+0][ltr.toUpperCase().charCodeAt()] = func;
}

/* Handle keys */
function handle_key(evt)
{
	/* Display the key that was pressed */
	if (key_box == undefined)
		key_box = document.getElementById("thekey");
	if (key_box)
	{
		var msg = "Event: ";
		if (evt.ctrlKey)
			msg += "Ctrl-";
		if (evt.shiftKey)
			msg += "Shift-";
		if (evt.metaKey)
			msg += "Meta-";
		if (evt.altKey)
			msg += "Alt-";
		if (evt.keyCode >= 32 &&
		    evt.keyCode <= 126)
			msg += String.fromCharCode(evt.keyCode);
//		else
			msg += "&lt;"+evt.keyCode+"&gt;"
		key_box.innerHTML = msg;
	}

// evt: object KeyboardEvent
// originalTarget
// target
// currentTarget
// type (keyup)
// eventPhase (3)
// which (74 == ASCII J)
// ctrlKey (false)
// shiftKey (false)
// keyCode (74)
// metaKey (false)
// altKey (false)
// view (object Window)

	var func = keytab[evt.ctrlKey+0][evt.shiftKey+0][evt.metaKey+0][evt.altKey+0][evt.keyCode];
	if (func != undefined)
	{
//key_box.innerHTML = "Calling function ["+func+"]";
		func();
		return;
	}
return;

	if (!evt.ctrlKey &&
	    !evt.shiftKey &&
	    !evt.metaKey &&
	    !evt.altKey)
	{
		switch(evt.keyCode)
		{
		    case "R".charCodeAt():
			main_form.submit();
			return;
		    default:
			return;
		}
	} else if (!evt.ctrlKey &&
	     evt.shiftKey &&
	    !evt.metaKey &&
	    !evt.altKey)
	{
		switch (evt.keyCode)
		{
		    case "C".charCodeAt():	// S-C: collapse-all
			collapse_all();
			break;
		    case "E".charCodeAt():	// S-E: expand-all
			expand_all();
			break;
		}
		return;
	}
}
