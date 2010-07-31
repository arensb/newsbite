document.addEventListener("DOMContentLoaded", init, false);

var main_form;		// Form containing all the items.

function init()
{
//	if (document.getElementsByClassName)
//;//		alert("have getElementsByClassName")
//	else
//;//		alert("don't have getElementsByClassName")
//		// XXX - Ought to implement one. But it exists in all
//		// the browsers I care about.
	// XXX - Firefox 2 (carrot) doesn't have getElementsByClassName

	addListenerByClass("collapse-bar", "click", toggle_pane, false);
	addListenerByClass("expand-bar", "click", toggle_pane, false);
	addListenerByClass("mark-check", "click", mark_item, false);

	// The main form, the one that holds all the items, their
	// checkboxes, the buttons at the top and bottom, etc.
	main_form = document.forms[0];

	window.addEventListener("keydown", handle_key, false);

	// Key bindings
	bind_key("C-r", function() { main_form.doit[0].click() });
			// XXX - Hack: just click on the button
	bind_key("S-c", collapse_all);
	bind_key("S-e", expand_all);
}

/* addListenerByClass
 * Adds an event listener to all elements with class 'className'.
 * The arguments 'event', 'handler', and 'capture' are passed on
 * to addEventListener().
 */
function addListenerByClass(className, event, handler, capture)
{
	var elements = document.getElementsByClassName(className);

	for (var i = 0; i < elements.length; i++)
	{
		try {
			elements[i].addEventListener(event, handler, capture);
		} catch (err) {
			// XXX - Is there anything to do?
		}
	}
}

//// Function.defer
//// From http://www.jslab.dk/library/Function.defer
//Function.prototype.defer =
//  function(n,o) {
//    // Get arguments as array
//    var a = [];
//    for(var i=2; i<arguments.length; i++)
//      a.push(arguments[i]);
//    var that = this;
//    window.setTimeout(function(){return that.apply(o,a);},n);
//  };

var debug_window = undefined;
var mark_read = {};		// Hash of item_id -> is_read? values
var mark_request = null;	// Data for marking items as read/unread

var last_time = null;
function debug(str)
{
/*return;*/
	if (debug_window == undefined)
		debug_window = document.getElementById("debug");
	if (debug_window == null)
		return;

	var t = new Date().getTime();
//	var delta = (last_time == null ? "" : t - last_time);
//	last_time = t;
//	debug_window.innerHTML += t + "(" + delta + "): " +
	debug_window.innerHTML += t + ": " +
		str + "<br/>\n";
}

function clrdebug()
{
	if (debug_window == null)
		return;
	debug_window.innerHTML = "";
}

// XXX - This function shouldn't be replicated. Consolidate.
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
//				debug("Error allocating new XMLHttpRequest\n");
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
//			debug("Error allocating ActiveX XMLHTTP\n");
		}
	}
	return request;
}

/* toggle-pane
 * Intended to be called from within
 * <div content-panes>
 *   <div {expand-bar|collapse-bar}>
 * The <div content-panes> is expected to have one <div item-summary>
 * and one <div item-content>.
 *
 * This function toggles the state of the <div content-panes>: if it
 * used to display the summary, it should now display the content, and
 * vice-versa.
 */
function toggle_pane(ev)
{
	var node = ev.currentTarget;
			// Not the element that was clicked on, but
			// the one that captured the event.
	var my_pane;		// Pane containing the calling element
	var sib_class;	 	// Class of sibling we're looking for

	var container = node.parentNode;

	/* Go up until we find the <div content-panes> that contains
	 * both the <div item-summary> and the <div item-content>.
	 */
	while (container && (!is_in_class(container, "content-panes")))
		container = container.parentNode;
	if (container == null)
		/* Something's wrong. Abort */
		return;

	set_pane(container);

	ev.preventDefault();	// Stop processing the event
}

function set_pane(container, state)
{
	/* Get the "which" attribute to see which pane is currently
	 * displayed, and toggle it.
	 * Ideally, CSS should take care of the rest, and in Firefox it
	 * does, but Safari is stupid and doesn't update its display.
	 * So we need to do this stuff manually.
	 */
	var cont_state;
	var new_state;
	if (state == undefined)
	{
		/* No new state given. Toggle the pane's state */
		cont_state = container.getAttribute("which");
		new_state = (cont_state == "summary" ? "content" : "summary");
	} else
		new_state = state;

	if (new_state == "summary")
	{
		replace_class(container, "show-content", "show-summary");
	} else {
		replace_class(container, "show-summary", "show-content");
	}
	container.setAttribute("which", new_state);
}

/* flush_queues
 * Send the contents of the queues to markitems.php
 */
function flush_queues()
{
clrdebug();
	// XXX - If another flush_queues() request is running, do
	// nothing; return.
	// XXX - There should never be two flush_queues()es running at
	// the same time.
//	if (mark_request == null)
//		return;

	// mark_request: an object encapsulating everything we want to keep
	// track of during this operation
	mark_request = {};
	mark_request.read = new Array();
	mark_request.unread = new Array();

	/* Assign each element of mark_read to either mark_request.read
	 * or mark_request.unread.
	 */
	for (i in mark_read)
	{
		if (mark_read[i])
			mark_request.read.push(i);
		else
			mark_request.unread.push(i);
		delete(mark_read[i]);
	}
//debug("Marking [" + mark_request.read + "] as read, and [" + mark_request.unread + "] as unread");

	/* Start a new request to send the queues (mark_request.read and
	 * mark_request.unread) to the server.
	 */
//debug("Flushing queues");
	// XXX - http://stackoverflow.com/questions/2680756/why-should-i-reuse-xmlhttprequest-objects
	// suggests reusing XMLHttpRequest objects: reduces number of
	// distinct objects, so fewer network connections to step on
	// each other's toes, less garbage collection, lower chance of
	// a memory leak, etc.
	// http://ajaxpatterns.org/XMLHttpRequest_Call says to make sure
	// the object has either completed, or you've called abort()
	// before reusing.
	var request = createXMLHttpRequest();
	if (!request)
	{
		// XXX - Better error-reporting
//		defaultStatus = "Error: can't create XMLHttpRequest";
//debug("Error: can't create XMLHttpRequest");
		return;
	}

	var err;

	mark_request.request = request;

	request.open('POST',
		'markitems.php?o=json',
		true);
	request.setRequestHeader('Content-Type',
		'application/x-www-form-urlencoded');
	request.onreadystatechange =
		function() { parse_flush_response(mark_request) };
	// Encode lists of items to mark as read/unread
	var req_data = "mark-read=" +
			encodeURIComponent(mark_request.read.join(",")) +
			"&mark-unread=" +
			encodeURIComponent(mark_request.unread.join(","));
//debug("req_data == [" + req_data + "]");
//p.record("before request.send");
	request.send(req_data);
//p.record("after request.send");
//debug("Sent request");

	return false;
}

function parse_flush_response(req)
{
//p.record("readyState " + req.request.readyState);
	var err = 0;
	var errmsg = undefined;
// XXX - If there's an error, take all the items in req and put them
// back in mark_read (bearing in mind that they may have been marked
// again by the user, so don't overwrite those).
//	debug("parse_response readyState: " + req.request.readyState);
	switch (req.request.readyState)
	{
	    case 0:		// Uninitialized
	    case 1:		// Loading
		return
	    case 2:		// Loaded
		/* Get request status */
//		debug("We get signal");

		/* Get HTTP status */
		try {
			err = req.request.status;
			errmsg = req.request.statusText;
//			debug("request status: [" + err + "]");
//			debug("request status text: [" + errmsg + "]");
		} catch (e) {
//			debug("Failed to get status: " + e);
			err = 1;
		}

		/* If the HTTP status isn't 200, abort the request and
		 * put the items back on the to-do list.
		 */
		if (err != 200)
		{
//			debug("Aborting");
			req.request.abort();
			req.aborted = true;

			/* Put the items to be marked back on mark_read */
			for (i in req.read)
			{
				var id = req.read[i];
//debug("Putting back " + id + " as read");
				if (mark_read[id] == undefined)
					mark_read[id] = true;
			}
			for (i in req.unread)
			{
				var id = req.unread[i];
//debug("Putting back " + id + " as unread");
				if (mark_read[id] == undefined)
					mark_read[id] = false;
			}
		}
		return;
	    case 3:		// Got partial text
//		debug("Got some text. Len " + req.request.responseText.length);
		return;
	    case 4:		// Got all text
//		debug("Got all text. Len " + req.request.responseText.length +", \"" + req.request.responseText+ "\"");
//		debug("Got all text. Len " + req.request.responseText.length);

		/* Check response text: if it's not a status message
		 * from our server saying that the messages were
		 * deleted, then consider it failed: an HTTP status
		 * code of 200 could have come from a proxy popping up
		 * a login box or something.
		 */
		// XXX - The following code is duplicated in feeds.js.
		// Ought to consolidate into a library or something.
		var lines = req.request.responseText.split("\n");
		var l = {};
		for (var i = 0; i < lines.length; i++)
		{
			var line = lines[i];

			if (line[0] != '{')
				// There might be non-JSON lines in there.
				// Ignore them. For that matter, ignore
				// any JSON lines that aren't objects.
				continue;
			try {
				eval("l = " + line);
				break;
			} catch (e) {
//				debug("Caught error " + e);
				continue;
			}
		}
		if (l.state != "ok")
		{
//			debug("Didn't get ok status from server.");
			/* Put the items to be marked back on mark_read */
			// XXX - This code is duplicated above. Consolidate
			// into a function.
			for (i in req.read)
			{
				var id = req.read[i];
//debug("Putting back " + id + " as read");
				if (mark_read[id] == undefined)
					mark_read[id] = true;
			}
			for (i in req.unread)
			{
				var id = req.unread[i];
//debug("Putting back " + id + " as unread");
				if (mark_read[id] == undefined)
					mark_read[id] = false;
			}
			return;
		}

		if (req.aborted)
			return;
		for (i in req.read)
		{
//debug("marking "+req.read[i]+" as read");
			var item = document.getElementById("item-"+req.read[i]);
			if (item == null)
				continue;
			item.setAttribute("deleted", "yes");
		}
		for (i in req.unread)
		{
//debug("marking "+req.unread[i]+" as unread");
			var item = document.getElementById("item-"+req.unread[i]);
			if (item == null)
				continue;
			item.setAttribute("deleted", "no");
		}
//defaultStatus = "Done";
//debug("Done");
		break;
	}
}

/* mark_item
 * Called when user changes the checkbox on an item, to toggle it from
 * read to unread or vice-versa.
 * 'elt' is the checkbox that was just checked by the user.
 */
function mark_item(ev)
{
	var elt = ev.target;

	/* Find the enclosing <div class="item"> by going up the parent
	 * chain.
	 */
	var item_div = elt.parentNode;
	while (item_div && !is_in_class(item_div, "item"))
		item_div = item_div.parentNode;
	if (item_div == null)
		/* Something's wrong. Abort */
		return;

	/* Iff the item is being marked as read, add the "item-read"
	 * class.
	 */
	if (elt.checked)
		add_class(item_div, "item-read");
	else
		remove_class(item_div, "item-read");

	/* There are several checkboxes for each item. Find them all,
	 * and make sure they're checked/unchecked as well.
	 */
	var buttons;
	buttons = item_div.getElementsByClassName("mark-check");

	for (var i = 0; i < buttons.length; i++)
	{
		buttons[i].checked = elt.checked;
	}

	/* Scroll so that the (collapsed) item is visible.
	 * Let's say the item is long; the user has read the item, and
	 * has clicked on the bottom checkbox to mark the item as
	 * read. By this time, the top of the item has scrolled past
	 * the top of the window. If we just collapse the item, the
	 * viewport will jump to a random position lower down on the
	 * window.
	 * To avoid this, we scroll the window so that the top of the
	 * item is at the top of the window, and the user can see the
	 * next item on the page.
	 * We don't want to do this if the top of the element is
	 * already visible, because unnecessary scrolling is jarring.
	 * We also don't want to do this when unchecking items,
	 * because collapsed items are already small enough that the
	 * whole thing is visible, so the problem described above
	 * doesn't occur.
	 */
	if (elt.checked &&
	    item_div.offsetTop < window.pageYOffset)
	{
		// Window has been scrolled so that top of item is no
		// longer visible.
		/* XXX - If the top is no longer visible, but the
		 * bottom is still visible, then we want to scroll so
		 * that the bottom is still in the same position.
		 */
		/* XXX - Likewise, if the user has zoomed the window,
		 * and the left edge is hidden, shouldn't scroll to
		 * the left edge. This means replacing the 0, below,
		 * with something else.
		 */
		window.scrollTo(0, item_div.offsetTop);
	}

	var item_id = elt.name.slice(4);
		// The name of the checkbox is either "cbt-12345" or
		// "cbb-12345". Get the item ID from that.
		// XXX - Ought to get it from a div attribute or
		// something. Perhaps add 'item-id="12345"' to the
		// item div.

	/* Add the item ID to the queue of items to mark as read/unread */
	/* XXX - As time goes on, item IDs will grow ever larger. So
	 * 'mark_read' will have an ever larger number of empty entries
	 * at the beginning.
	 */
	mark_read[item_id] = elt.checked;

	/* Flush the queue if necessary */
	flush_queues();

	/* Remove keyboard focus from the item. That way, can
	 * immediately hit space to scroll down, and it won't mark the
	 * item as read/unread again.
	 */
	elt.blur();
}

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
var key_box = undefined;	/* Debugging key events */

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
		func(evt);
		// XXX - Should this also evt.prevent_default()?
		return;
	}
}

/* XXX - collapse_all() and expand_all() don't play nice with
 * toggle_pane(): they force a collapse/expansion, but toggle_pane()
 * looks at the "which" attribute to decide which state to move the item
 * to.
 * collapse_all() and expand_all() should probably call toggle_pane()
 * with arguments saying to force a state, or soemthing.
 */
function collapse_all()
{
	var items = document.getElementsByClassName("content-panes");

//	alert("Found "+items.length+" items");
	for (var i = 0, len = items.length; i < len; i++)
	{
//		replace_class(items[i], "show-content", "show-summary");
		set_pane(items[i], "summary");
	}
}

function expand_all()
{
	var items = document.getElementsByClassName("content-panes");

//	alert("Found "+items.length+" items");
	for (var i = 0, len = items.length; i < len; i++)
	{
//		replace_class(items[i], "show-summary", "show-content");
		set_pane(items[i], "content");
	}
}

/* ---------------------------------------- */

/* load_module
 * Load the JS file specified by the url. When it's loaded, call the
 * callback function.
 */
function load_module(url, callback)
{
	var module = document.createElement("script");
	module.type = "text/javascript";
	module.src = url;
	module.onload = callback;
	document.getElementsByTagName("head")[0].appendChild(module);
}

//load_module("foo.js",
//	    function() {
//		    alert("Inside load_module callback");
//		    foo();
//	    });
