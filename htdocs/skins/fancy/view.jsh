#if DEBUG
#  include "js/debug.js"
#else
function debug() { }
function clrdebug() { }
#endif	// DEBUG
// #include "js/defer.js"
#include "js/xhr.js"
#include "js/classes.js"
#include "js/keybindings.js"
#include "js/load_module.js"

document.addEventListener("DOMContentLoaded", init, false);

var main_form;		// Form containing all the items.
var mark_read = {};		// Hash of item_id -> is_read? values
var mark_request = null;	// Data for marking items as read/unread

function init()
{
	// XXX - Firefox 2 (carrot) doesn't have
	// document.getElementsByClassName()

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

	/* Find the "item" container: if we collapse from the bottom
	 * bar, we might wind up looking at the middle of a completely
	 * unrelated article, which is surprising and annoying.
	 * So if the top of the item is above the window border when
	 * we collapse, scroll so that the top of the item is at the
	 * top of the browser window.
	 */
	var item_div = container.parentNode;
	while (item_div && !is_in_class(item_div, "item"))
		item_div = item_div.parentNode;
	if (item_div == null)
		return;
	if (item_div.offsetTop < window.pageYOffset)
		window.scrollTo(0, item_div.offsetTop);
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
	request.send(req_data);

	return false;
}

function parse_flush_response(req)
{
	var err = 0;
	var errmsg = undefined;

	// XXX - If there's an error, take all the items in req and
	// put them back in mark_read (bearing in mind that they may
	// have been marked again by the user, so don't overwrite
	// those).
	//	debug("parse_response readyState: " + req.request.readyState);

	switch (req.request.readyState)
	{
	    case 0:		// Uninitialized
	    case 1:		// Loading
		return
	    case 2:		// Loaded
		/* Get request status */

		/* Get HTTP status */
		try {
			err = req.request.status;
			errmsg = req.request.statusText;
		} catch (e) {
			err = 1;
		}

		/* If the HTTP status isn't 200, abort the request and
		 * put the items back on the to-do list.
		 */
		if (err != 200)
		{
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
		return;
	    case 4:		// Got all text

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
				continue;
			}
		}
		if (l.state != "ok")
		{
			/* Put the items to be marked back on mark_read */
			// XXX - This code is duplicated above. Consolidate
			// into a function.
			for (i in req.read)
			{
				var id = req.read[i];
				if (mark_read[id] == undefined)
					mark_read[id] = true;
			}
			for (i in req.unread)
			{
				var id = req.unread[i];
				if (mark_read[id] == undefined)
					mark_read[id] = false;
			}
			return;
		}

		if (req.aborted)
			return;
		for (i in req.read)
		{
			var item = document.getElementById("item-"+req.read[i]);
			if (item == null)
				continue;
			item.setAttribute("deleted", "yes");
		}
		for (i in req.unread)
		{
			var item = document.getElementById("item-"+req.unread[i]);
			if (item == null)
				continue;
			item.setAttribute("deleted", "no");
		}
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

function collapse_all()
{
	var items = document.getElementsByClassName("content-panes");

	for (var i = 0, len = items.length; i < len; i++)
	{
		set_pane(items[i], "summary");
	}
}

function expand_all()
{
	var items = document.getElementsByClassName("content-panes");

	for (var i = 0, len = items.length; i < len; i++)
	{
		set_pane(items[i], "content");
	}
}
