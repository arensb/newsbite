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
function toggle_pane(node)
{
	var my_pane;		// Pane containing the calling element
	var sib_class;	 	// Class of sibling we're looking for

	var container = node.parentNode;

	/* Go up until we find the <div content-panes> that contains
	 * both the <div item-summary> and the <div item-content>.
	 */
	while (container && (container.className != "content-panes"))
		container = container.parentNode;
	if (container == null)
		/* Something's wrong. Abort */
		return;

	/* Set the "which" attribute on the pane container. CSS does
	 * the rest: there are different rules for displaying expanded
	 * and collapsed articles.
	 */

	cont_state = container.getAttribute("which");
	if (cont_state == "summary")
		container.setAttribute("which", "content");
	else
		container.setAttribute("which", "summary");
}

/* flush_queues
 * Send the contents of the queues to markitems.php
 */
function flush_queues()
{
clrdebug();
	// XXX - If another flush_queues() request is running, do
	// nothing; return.
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
debug("Marking [" + mark_request.read + "] as read, and [" + mark_request.unread + "] as unread");

	/* Start a new request to send the queues (mark_request.read and
	 * mark_request.unread) to the server.
	 */
debug("Flushing queues");
	var request = createXMLHttpRequest();
	if (!request)
	{
		// XXX - Better error-reporting
//		defaultStatus = "Error: can't create XMLHttpRequest";
debug("Error: can't create XMLHttpRequest");
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
debug("req_data == [" + req_data + "]");
	request.send(req_data);
debug("Sent request");

	return false;
}

function parse_flush_response(req)
{
	var err = 0;
	var errmsg = undefined;
// XXX - If there's an error, take all the items in req and put them
// back in mark_read (bearing in mind that they may have been marked
// again by the user, so don't overwrite those).
	debug("parse_response readyState: " + req.request.readyState);
	switch (req.request.readyState)
	{
	    case 0:		// Uninitialized
	    case 1:		// Loading
		return
	    case 2:		// Loaded
		/* Get request status */
		debug("We get signal");

		/* Get HTTP status */
		try {
			err = req.request.status;
			errmsg = req.request.statusText;
			debug("request status: [" + err + "]");
			debug("request status text: [" + errmsg + "]");
		} catch (e) {
			debug("Failed to get status: " + e);
			err = 1;
		}

		/* If the HTTP status isn't 200, abort the request and
		 * put the items back on the to-do list.
		 */
		if (err != 200)
		{
			debug("Aborting");
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
		debug("Got some text. Len " + req.request.responseText.length);
		return;
	    case 4:		// Got all text
//		debug("Got all text. Len " + req.request.responseText.length +", \"" + req.request.responseText, "\"");
		debug("Got all text. Len " + req.request.responseText.length);

		/* Check response text: if it's not a status message
		 * from our server saying that the messages were
		 * deleted, then consider it failed: an HTTP status
		 * code of 200 could have come from a proxy popping up
		 * a login box or something.
		 */
		// XXX - The following code is duplicated in feeds.js.
		// Ought to consolidate into a library or something.
		var lines = req.request.responseText.split("\n");
		var l;
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
			} catch (e) {
				debug("Caught error " + e);
				continue;
			}
		}
		if (l.status != "ok")
		{
			debug("Didn't get ok status from server.");
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
debug("Done");
		break;
	}
}

/* mark_item
 * Called when user changes the checkbox on an item, to toggle it from
 * read to unread or vice-versa.
 */
function mark_item(elt)
{
	/* Find the enclosing <div class="item"> by going up the parent
	 * chain.
	 */
	var item_div = elt.parentNode;
	while (item_div && (item_div.className != "item"))
		item_div = item_div.parentNode;
	if (item_div == null)
		/* Something's wrong. Abort */
		return;

	/* Set the "state" attribute to either "read" or "unread" so
	 * that the CSS rules can change the appearance appropriately.
	 */
	item_div.setAttribute("state", (elt.checked ? "read" : "unread"));

	/* There are two checkboxes for each item. Find the matching
	 * one, and make sure it's checked/unchecked as well.
	 * The two checkboxes are named "cbt-12345" and "cbb-12345",
	 * so given the name of the box the user clicked on, we can
	 * easily construct the name of the other one. And since
	 * they're both fields in the same form, we can save a lot of
	 * searching.
	 */
	var other_cb_name;
	if (elt.name[2] == "t")
		other_cb_name = "cbb-" + elt.name.slice(4);
	else
		other_cb_name = "cbt-" + elt.name.slice(4);
	elt.form[other_cb_name].checked = elt.checked;

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
		window.scrollTo(0, item_div.offsetTop);
	}

	var item_id = elt.name.slice(4);
		// The name of the checkbox is either "cbt-12345" or
		// "cbb-12345". Get the item ID from that.

	/* Add the item ID to the queue of items to mark as read/unread */
	mark_read[item_id] = elt.checked;

	/* Flush the queue if necessary */
	flush_queues();
}
