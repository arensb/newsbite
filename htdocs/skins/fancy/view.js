/* Profiler class
 * Records when functions are entered/left, and can produce a report
 * of where time was spent.
 */
function Profiler()
{
	this.stamps = [];
	this.t0 = new Date().getTime();	// Record when started
}

// Recorded events are tuples of the form [type, msg, time],
// where
// type is a type of event:
//	0: passing a breakpoint
//	1: entering a function
//	2: exiting a function
// msg is a message; for entering and exiting functions, this is the
//	name of the function.
// time is the time at which the event was recorded, in milliseconds
//	since the epoch.

Profiler.prototype.record = function(str)
{
	this.stamps.push([0,
			  str,
			  new Date().getTime()
			  ]);
}

Profiler.prototype.enter = function(funcname)
{
	this.stamps.push([1,
			  funcname,
			  new Date().getTime()
			  ]);
}

Profiler.prototype.leave = function(funcname)
{
	this.stamps.push([2,
			  funcname,
			  new Date().getTime()
			  ]);
}

var spaces = "                                                                                                    ";
Profiler.prototype.report = function()
{
	var box = document.getElementById("profiler");
	if (box == undefined)
		return;

	var totals = {};
	var context = [];

	for (var i = 0; i < this.stamps.length; i++)
	{
		var event = this.stamps[i];
			// [0]: type
			// [1]: function/breakpoint name
			// [2]: timestamp
		var msg = "";

		msg += spaces.substr(0, context.length*4) + msg;
			// Indentation

		switch (event[0])
		{
		    case 1:
			msg += "Enter " + event[1];

			// Push a tuple of the form {funcname, time}
			// onto the context stack.
			context.push([event[1], event[2]]);
			break;
		    case 2:
			msg += "Exit " + event[1];
			// Hopefully the last item on the context stack
			// is from when this function was called
			if (context[context.length-1][0] != event[1])
			{
				box.innerHTML += "*** Expected " + event[1] + ", got " + context[context.length-1] + "\n";
			}
			var starttime = context[context.length-1][1];
			var totaltime = event[2] - starttime;
			if (totals[event[1]] == undefined)
				totals[event[1]] = 0;
			totals[event[1]] += totaltime;
			msg += " (" + totaltime + " ms)";
			context.pop();
			break;
		    case 0:
		    default:
			msg += event[1];

			var frame = context[context.length-1];
			var lasttime = frame[2];
			if (lasttime == undefined)
				lasttime = frame[1];
			frame[2] = event[2];
			var totaltime = event[2] - lasttime;
			msg += " (" + totaltime + " ms)";

			break;
		}

		box.innerHTML += (event[2]-this.t0)/1000 + ": " + msg + "\n";
	}
	box.innerHTML += "\n** Totals **\n";
	for (var i in totals)
	{
		box.innerHTML += i + ": " + totals[i] + "\n";
	}
}

// Profiler.register
// Replace the named function with a wrapper that records when the
// function was called, calls it, records when the function exited,
// and returns the original function's return value.
// XXX - For now, we assume that all functions are global
Profiler.prototype.register = function(funcname)
{
	var oldfunc = window[funcname];
	var prototyper = this;

	window[funcname] = function()
	{
		prototyper.enter(funcname);
		var retval = oldfunc.apply(this, arguments);
		prototyper.leave(funcname);
		return retval;
	}
}

Profiler.prototype.register_object = function(obj)
{
	var prototyper = this;
	var obj_name = obj.toString();
		// XXX - Ought to get the name of the class, but AFAIK
		// the only way to do that is to parse obj.constructor
		// and get the "function ClassName" part.

	for (var f in obj)
	{
		try {
			if (typeof(obj[f]) != "function")
				continue;
		} catch(e) {
			continue;
		}

		var old_method = obj[f];
		var method_name = obj_name + "." + f;

		obj[f] = function()
		{
			prototyper.enter(method_name);
			var retval = old_method.apply(this, arguments);
			prototyper.leave(method_name);
			return retval;
		}
	}
}

Profiler.prototype.register_class = function(theclass)
{
	var prototyper = this;
	var obj_name = theclass.toString();

	for (var f in theclass.prototype)
	{
		try {
			if (typeof(theclass.prototype[f]) != "function")
				continue;
		} catch (e) {
			continue;
		}

		var old_method = theclass.prototype[f];
		var method_name = obj_name + "." + f;

		theclass.prototype[f] = function()
		{
			prototyper.enter(method_name);
			var retval = old_method.apply(this, arguments);
			prototyper.leave(method_name);
			return retval;
		}
	}
}

// ----- End Profiler class ----------------------------------------
document.addEventListener("DOMContentLoaded", init, false);

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


//var p = new Profiler();

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

	/* Get the "which" attribute to see which pane is currently
	 * displayed, and toggle it.
	 * Ideally, CSS should take care of the rest, and in Firefox it
	 * does, but Safari is stupid and doesn't update its display.
	 * So we need to do this stuff manually.
	 */
	var cont_state = container.which_pane;
	if (cont_state == undefined)
		cont_state = container.getAttribute("which");
	if (cont_state == "summary")
	{
		cont_state = "content";
		container.which_pane = "content";
		replace_class(container, "show-summary", "show-content");
	} else {
		cont_state = "summary";
		container.which_pane = "summary";
		replace_class(container, "show-content", "show-summary");
	}

	ev.preventDefault();	// Stop processing the event
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
//		debug("Got all text. Len " + req.request.responseText.length +", \"" + req.request.responseText, "\"");
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
//debug("marking "+req.read[i]+" as unread");
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

	var classes = class_str.split(" ");
			// Split class string into a list

	for (var i in classes)
	{
		if (classes[i] == cls)
			// Yup. It's in 'cls'.
			return true;
	}
	return false;	// Nope. It's not in 'cls'
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

//// Mark all global functions for profiling.
//for (f in window)
//{
//	if (typeof(window[f]) == "function")
//	{
//		p.register(f);
//	}
//}

// XXX - register_class doesn't work properly, at least not for
// XMLHttpRequest: it contains functions that are security-restricted
// to avoid CSS exploits.
//p.register_class(XMLHttpRequest);
