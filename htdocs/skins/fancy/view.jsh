/*						-*- JavaScript -*- */
#include "js/debug.js"
// #include "js/defer.js"
#include "js/xhr.js"
#include "js/classes.js"
#include "js/keybindings.js"
#include "js/PatEvent.js"
#include "js/types.js"
#include "js/CacheManager.js"
#include "js/Template.js"
/*#include "js/load_module.js"*/
#include "js/status-msg.js"
/* XXX - Should have a separate module for handling updates from the
 * server: have functions to request fresh copy of feed list, or
 * getting new items.
 *
 * When those things come in, perhaps send an event.
 */
/* XXX - Delete unused functions */

document.addEventListener("DOMContentLoaded", init, false);

var main_form;		// Form containing all the items.
var mark_read = {};		// Hash of item_id -> is_read? values
var mark_request = null;	// Data for marking items as read/unread
var current_item = null;	// Current item, for keybindings and such

var cache = new CacheManager();	// Cache manager for locally-stored data
var feeds;		// List of feeds
var allitems;		// In-memory list of all known items.
			// XXX - This shouldn't exist. Most known
			// items should be in cache.
var disp_items;		// List of displayed items

var itemlist;		// Div containing the items.
var item_tmpl = new Template(item_tmpl_text);
			// Defined in view.php

function init()
{
	itemlist = document.getElementById("itemlist");

	// XXX - The root node for bind_event shouldn't be document,
	// but rather whichever div will contain the articles.
	PatEvent.bind_event(document, "click", ".collapse-bar",
			    toggle_pane, false);
	PatEvent.bind_event(document, "click", ".expand-bar",
			    toggle_pane, false);
	PatEvent.bind_event(document, "click", ".mark-check",
			    button_mark_item, false);

	// The main form, the one that holds all the items, their
	// checkboxes, the buttons at the top and bottom, etc.
	main_form = document.forms[0];

	window.addEventListener("keydown", handle_key, false);

	// Key bindings
	bind_key("C-r", function() { main_form.doit[0].click() });
			// XXX - Hack: just click on the button
	bind_key("S-c", collapse_all);
	bind_key("S-e", expand_all);

	/* On desktop, keep track of current item, and add key bindings
	 * to navigate.
	 */
	if (mobile == "")
	{
		PatEvent.bind_event(document, "_enter", ".item",
				    enter_item, false);
		PatEvent.bind_event(document, "_exit", ".item",
				    exit_item, false);
		bind_key("d", key_mark_item);
		// XXX - bind_key("k", move_up);
		// XXX - bind_key("j", move_down);
	}

	// Get feeds and items from cache.
	feeds = cache.feeds();
//console.debug("initially feeds == "+feeds);
	allitems = cache.getitems();
//console.debug("initially allitems == "+allitems);

	// Draw what we've got so far, if anything
	if (feeds != null && allitems != null)
		redraw_itemlist();
else console.debug("not calling redraw_itemlist yet");

	// Get fresh feed and item information. When that arrives,
	// it'll update the feed list.
	init_feeds_items();
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

	/* Start a new request to send the queues (mark_request.read and
	 * mark_request.unread) to the server.
	 */
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
		return true;
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
				if (mark_read[id] == undefined)
					mark_read[id] = true;
			}
			for (i in req.unread)
			{
				var id = req.unread[i];
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

/* mark_item1
 * Marks an item as read or unread. 'ev' is an event (or at least
 * ev.target has to point to a DOM element inside the item that's
 * being marked).
 * This function is the common code for all the ways an item can be
 * marked (keyboard, click on a checkbox, etc.)
 */
function mark_item1(ev)
{
	var elt = ev.target;

	/* Find the enclosing <div class="item"> by going up the parent
	 * chain.
	 */
	var item_div = elt;
	while (item_div && !is_in_class(item_div, "item"))
		item_div = item_div.parentNode;
	if (item_div == null)
		/* Something's wrong. Abort */
		return;

	var is_read = item_div.is_read;
		// If item_div.is_read isn't set, it defaults to false

	// Flip the is_read bit, and add/remove the "item-read" class.
	if (item_div.is_read)
	{
		item_div.is_read = false;
		remove_class(item_div, "item-read");
	} else {
		item_div.is_read = true;
		add_class(item_div, "item-read");
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
	if (item_div.is_read &&
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

	/* There are several checkboxes for each item. Find them all,
	 * and make sure they're checked/unchecked as well.
	 */
	var buttons;
	buttons = item_div.getElementsByClassName("mark-check");

	for (var i = 0; i < buttons.length; i++)
	{
		buttons[i].checked = item_div.is_read;
	}

	var item_id = item_div.id.slice(5);
		// The 'id' attribute is of the form "item-12345". Get
		// the item ID from that.

	/* Add the item ID to the queue of items to mark as read/unread */
	/* XXX - As time goes on, item IDs will grow ever larger. So
	 * 'mark_read' will have an ever larger number of empty entries
	 * at the beginning.
	 */
	mark_read[item_id] = item_div.is_read;

	/* Flush the queue if necessary */
	flush_queues();
}

/* button_mark_item
 * Called when user clicks a button to mark an item. Does the
 * button-specific stuff, then defers the rest to mark_item1().
 */
function button_mark_item(ev)
{
	var elt = ev.target;

	/* Do the common stuff */
	mark_item1(ev);

	/* Find the enclosing <div class="item"> by going up the parent
	 * chain.
	 */
	var item_div = elt.parentNode;
	while (item_div && !is_in_class(item_div, "item"))
		item_div = item_div.parentNode;
	if (item_div == null)
		/* Something's wrong. Abort */
		return;

	/* Remove keyboard focus from the item. That way, can
	 * immediately hit space to scroll down, and it won't mark the
	 * item as read/unread again.
	 */
	elt.blur();
}

/* key_mark_item
 * Mark the current item as read. Invoked by key press.
 */
function key_mark_item()
{
	if (current_item == null)
		return;

	mark_item1({target: current_item});
	// XXX - Advance to next item, and make it current.
}

/* collapse_all
 * Collapse all collapsible items, displaying only the summary.
 */
function collapse_all()
{
	var items = document.getElementsByClassName("content-panes");

	for (var i = 0, len = items.length; i < len; i++)
	{
		if (items[i].getAttribute("collapsible") != "yes")
			continue;
		set_pane(items[i], "summary");
	}
}

/* expand_all
 * Expand all collapsible items, displaying the content rather than the
 * summary.
 */
function expand_all()
{
	var items = document.getElementsByClassName("content-panes");

	for (var i = 0, len = items.length; i < len; i++)
	{
		if (items[i].getAttribute("collapsible") != "yes")
			continue;
		set_pane(items[i], "content");
	}
}

/* enter_item
 * Called when mouse has entered an item. Highlight it and mark it as
 * current.
 */
function enter_item(ev)
{
	var elt = ev.currentTarget;
	if (!is_in_class(elt, "item"))
		return false;
	add_class(elt, "current-item");
	current_item = elt;
	return true;
}

/* exit_item
 * Called when mouse has left an item. Unhighlight it.
 */
function exit_item(ev)
{
	var elt = ev.currentTarget;
	if (!is_in_class(elt, "item"))
		return false;
	remove_class(elt, "current-item");
	current_item = null;
	return true;
}

/* init_feeds_items
 * Get both feeds and items from the server.
 *
 * The problem is that we want to display things to the user quickly,
 * but we can't display items before we have feeds, because we want to
 * display things like the feed name, which aren't in the item.
 *
 * It also doesn't make sense to launch two AJAX requests at once,
 * since they'll just step on each other (and we can't reuse the XHR
 * object, which is apparently efficient). On top of which, trying to
 * coordinate multiple execution threads this way is a PITA.
 *
 * So in an attempt to keep things manageable, this function launches
 * an XHR request for the feeds. The callback for that function
 * launches an XHR request for items. The callback for that one
 * updates the displayed list.
 */
/* XXX - Should this be renamed to update_feeds_items(), and take
 * arguments like the feed to update, and a callback function?
 */
function init_feeds_items()
{
console.debug("Inside init_feeds_items");
	get_json_data("feeds.php",
		      { o:	"json",
			id:	"all",
		      },
		      feed_callback,
		      true);

	function feed_callback(value)
	{
console.debug("Got feeds: "+value)
		/* Start the next AJAX request going. It'll take
		 * forever, so we'll do other stuff while that's
		 * going.
		 */
		get_json_data("items.php",
			      { o:	"json",
			        id:	feed.id,
			      },
			      item_callback,
			      true);

		// Create an array of Feed objects from what we just got.
		/* XXX - Ought to update the existing list: we might
		 * store state or something, and don't want to lose
		 * that just because the feed count got updated.
		 */
		var newfeeds = new Array();
		for (var i in value)
			newfeeds[i] = new Feed(value[i]);
		feeds = newfeeds;

		cache.store_feeds(feeds);
	}

	function item_callback(value)
	{
		console.debug("Got items: "+value);

		feed = value.feed;	// Update current feed description

		/* Convert the items received into Item objects */
		for (i in value.items)
			value.items[i] = new Item(value.items[i]);

		// XXX - Cache the new items in local storage
		// XXX - Update in-memory list of items we know about?
		allitems = value.items;

		// Redraw itemlist
		redraw_itemlist();
	}
}

/* redraw_itemlist
 * We've received either a feed list or an item list. Update the
 * displayed list to reflect any necessary changes.
 */
function redraw_itemlist()
{
	var new_itemlist = document.createDocumentFragment();
		// XXX - Probably shouldn't create this until we know
		// we need to.
	// XXX - For now, just assume that we should display every
	// known item.
	for (var i in allitems)
	{
		var item = allitems[i];
		var item_feed = feeds[item.feed_id];
			// XXX - Check to make sure that
			// feeds[item.feed_id] exists, that we haven't
			// been given an item from a nonexistent feed?
		var title = item.displaytitle();

		// XXX - Fill these in.
		// XXX - Indicate whether to show content or summary.
		// XXX - Indicate whether collapsible.
		var item_values = {
			id:		item.id,
			url:		encodeURI(item.url),
			url_attr:	"",
				// XXX - On iPhone/iPad, open title link
				// in a new window.
			title:		title,
			feed_url:	encodeURI(item_feed.url),
				// XXX - Get this from feeds.
			feed_title:	item_feed.displaytitle(),
			author:		item.author,
				// XXX - If author is empty, shouldn't
				// display the "by" before author
				// name.
				// XXX - If ever use author URL, might
				// want to wrap author name in <a
				// href="mailto:...>.
			pub_date:	item.pub_date,
			pretty_pub_date:item.pub_date,
				// XXX - Pretty-print the date.
			summary:	item.summary,
			content:	item.content,
			comment_url:	encodeURI(item.content_url),
			comment_rss:	encodeURI(item.content_rss),
			// Indicate whether collapsible (both content
			// and summary exist), and whether to display
			// summary or content.
			collapsible:	(item.summary != null &&
					 item.content != null ?
					 "yes" : "no"),
			which:		(item.content == null ?
					 "summary" : "content"),
		};

		// XXX - The following hack, to create a DOM node from
		// HTML, should probably be in Template.js.
		var item_node = item_tmpl.expand(item_values);
		var new_node = document.createElement("div");
		new_node.innerHTML = item_node;
		item_node = new_node.firstChild;

		// Append to itemlist.
		new_itemlist.appendChild(item_node);

		/* XXX - Keep pointers to the items, and any other
		 * interesting information.
		 */
	}

	// Delete existing children
	while (itemlist.firstChild)
		itemlist.removeChild(itemlist.firstChild);
	itemlist.appendChild(new_itemlist);
}
