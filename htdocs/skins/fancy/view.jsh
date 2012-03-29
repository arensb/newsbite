/*						-*- JavaScript -*- */
#include "js/debug.js"
#include "js/defer.js"
#include "js/xhr.js"
#include "js/classes.js"
#include "js/keybindings.js"
#include "js/PatEvent.js"
#include "js/types.js"
#include "js/CacheManager.js"
#include "js/Template.js"
/*#include "js/load_module.js"*/
#include "js/status-msg.js"

document.addEventListener("DOMContentLoaded", init, false);

// Set HTMLElement's toJSON method, so that JSON.stringify() of
// structures containing a DOM node doesn't fail with an error.
/* XXX - This seems like a hack: might be better to split up
 * onscreen.items[i] into { data: {...}, node: <HTMLElement> }.
 * If so, remove this hack.
 */
if (HTMLElement.prototype.toJSON == null)
{
	HTMLElement.prototype.toJSON = function() { return undefined; };
}

var main_form;		// Form containing all the items.
var mark_read = {};		// Hash of item_id -> is_read? values
var mark_request = null;	// Data for marking items as read/unread
var current_item = null;	// Current item, for keybindings and such
	// XXX - Replace 'current_item' with 'cur_item'.

var cache = new CacheManager();	// Cache manager for locally-stored data
var feeds;		// List of feeds

// XXX - Perhaps should just remember current item:
//	ID
//	xpos, ypos: position of top left corner of the item div
//	last_update or something?
var cur_item = {
	id:	null,
	xpos:	null,
	ypos:	null,
	};
var onscreen;		// List of displayed items
	// XXX - {
	// current item - ID of item that has focus
	// cur_xpos, cur_ypos - X, Y position of current item
	//	Should this be x, y offset relative to top left corner
	//	of current item?
	// items - list of displayed items:
	//	{
	//		copy of what's in the database
	//		whether collapsed or not
	//		whether marked as read or not
	//	}
	// }
	// Perhaps have
	//	onscreen.prepend_items(list)
	//	onscreen.append_items(list)

var itemlist;		// Div containing the items.
var item_tmpl = new Template(item_tmpl_text);
			// Defined in view.php

function init()
{
	itemlist = document.getElementById("itemlist");

	// The main form, the one that holds all the items, their
	// checkboxes, the buttons at the top and bottom, etc.
	main_form = document.forms[0];

	// Bind some events
	PatEvent.bind_event(itemlist, "click", ".collapse-bar",
			    toggle_pane, false);
	PatEvent.bind_event(itemlist, "click", ".expand-bar",
			    toggle_pane, false);
	PatEvent.bind_event(itemlist, "click", ".mark-check",
			    button_mark_item, false);

	window.addEventListener("keydown", handle_key, false);

	/* On desktop, keep track of current item, and add key bindings
	 * to navigate.
	 */
	if (mobile == "")
	{
		PatEvent.bind_event(itemlist, "_enter", ".item",
				    enter_item, false);
		PatEvent.bind_event(itemlist, "_exit", ".item",
				    exit_item, false);
		bind_key("d", key_mark_item);
		bind_key("c", toggle_collapse_item);
		// XXX - bind_key("k", move_up);
		// XXX - bind_key("j", move_down);

		// Key bindings
		bind_key("C-r", refresh);
		bind_key("S-c", collapse_all);
		bind_key("S-e", expand_all);
	}

	// Get feeds and items from cache.
	feeds = cache.feeds();

	// Fetch the list of what was on screen last time we started
	cur_item = cache.getItem("cur_item");
	// XXX - Initialize it if empty. Or initialize any missing
	// bits even if not empty.

	if (onscreen == null)
	{
		onscreen = {
			cur_item:	null,
			cur_xpos:	0,
			cur_ypos:	0,
		};
		onscreen.items = cache.getitems(feed.id, null, 0, 25);
	}

	// Draw what we've got so far, if anything

	// XXX - If feeds is null, can't draw anything.
	if (feeds != null &&
	    onscreen.items != null &&
	    onscreen.items.length > 0)
		redraw_itemlist();

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

	// XXX - Find entry in onscreen.items. Mark its state. Save to
	// cache.
}

/* flush_queues
 * Send the contents of the queues to markitems.php
 */
// XXX - Move most of the item-marking stuff to CacheManager.js
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
	for (var i in mark_read)
	{
		if (mark_read[i])
			mark_request.read.push(i);
		else
			mark_request.unread.push(i);
		delete(mark_read[i]);
	}

	get_json_data("markitems.php",
		      { o:	"json",
		        "mark-read":	mark_request.read.join(","),
		        "mark-unread":	mark_request.unread.join(","),
		      },
		      function(value) {
			      parse_flush_response(value, mark_request);
		      },
		      function(status, msg) {
			      parse_flush_error(status, msg, mark_request);
		      },
		      true);
}

function parse_flush_response(value, req)
{
	for (var i in req.read)
	{
		var item = document.getElementById("item-"+req.read[i]);
			// XXX - Should have a table of
			// currently-displayed items.
		if (item == null)
			continue;
		item.setAttribute("deleted", "yes");
			// XXX - What else needs to be done to mark an
			// item as read?

		// XXX - If there are more than 10 deleted items in
		// onscreen, delete the oldest ones
		// - delete from onscreen
		// - remove from localStorage
	}
	for (var i in req.unread)
	{
		var item = document.getElementById("item-"+req.unread[i]);
			// XXX - Should have a table of
			// currently-displayed items.
		if (item == null)
			continue;
		item.setAttribute("deleted", "no");
			// XXX - What else needs to be done to mark
			// the item as unread?
		// XXX - If there are now more than 25 unread articles
		// on screen, remove some. (Which ones?)
	}
}

function parse_flush_error(status, msg, mark_request)
{
	msg_add("Error marking items: "+status+": "+msg);

	/* Put the items to be marked back on mark_read */
	for (var i in mark_request.read)
	{
		var id = mark_request.read[i];
		if (mark_read[id] == undefined)
			mark_read[id] = true;

		/* Mark the in-cache copy as read */
		// XXX - A loop inside a loop seems inefficient. This
		// is probably slow. Then again, we're only iterating
		// over the short list of what's on screen.
		for (var j in onscreen.items)
		{
			var item = onscreen.items[j];
			if (item.id == id)
			{
				item.is_read = true;
				cache.store_item(item);
				// XXX - Delete it from cache?
			}
		}
	}
	for (var i in mark_request.unread)
	{
		var id = mark_request.unread[i];
		if (mark_read[id] == undefined)
			mark_read[id] = false;

		/* Mark the in-cache copy as unread */
		// XXX - A loop inside a loop seems inefficient. This
		// is probably slow. Then again, we're only iterating
		// over the short list of what's on screen.
		for (var j in onscreen.items)
		{
			var item = onscreen.items[j];
			if (item.id == id)
			{
				item.is_read = false;
				cache.store_item(item);
				// XXX - Re-fetch it from the server?
			}
		}
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

	var item_id = item_div.item_id;

	/* Add the item ID to the queue of items to mark as read/unread */
	mark_read[item_id] = item_div.is_read;

	/* Find the item's entry in onscreen.items, and mark it */
	for (var i = 0, l = onscreen.items.length; i < l; i++)
	{
		var item = onscreen.items[i];
		if (item.id != item_id)
			continue;
		item.is_read = item_div.is_read;
		cache.store_item(item);		// Mark in the cache as well.
	}

	/* XXX - If marking an item as read, bring up another item
	 * from cache.
	 * - Get last item in onscreen.items.
	 * - Get the item that comes after that.
	 * - Display it.
	 * - If there are now > 25 items on screen, delete the topmost one.
	 * - If the deleted item was marked read, purge it from cache.
	 */
	addnewnode:
	if (item_div.is_read)
	{
		var last_item = onscreen.items[onscreen.items.length-1];
				// Get last item in onscreen.items.
		var more_items = cache.getitems(feed.id, last_item, 0, 1);
				// Get the next one after that in cache
		if (more_items == null || more_items.length < 2)
			// If getitems() returned < 2 items, then
			// there aren't any more items in cache after
			// last_item.
			break addnewnode;	// Break out of if-block

		var new_item = more_items.pop();
				// The new item we're about to add.
		var new_node = item2node(new_item);

		new_node.item_id = new_item.id;
		new_item.node = new_node;

		itemlist.appendChild(new_node);
		onscreen.items.push(new_item);
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

	/* Flush the queue if necessary.
	 * Use defer() here so that the browser can redraw
	 * immediately, in case there's a delay creating the HTTP
	 * request.
	 */
	flush_queues.defer(0);
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

/* toggle_collapse_item
 * Toggle the current item between showing summary and showing
 * content. That is, expand or collapse the item as needed.
 */
function toggle_collapse_item()
{
	if (current_item == null)
		return;

	var panes = current_item.getElementsByClassName("content-panes");
	for (var i = 0, l = panes.length; i < l; i++)
	{
		var p = panes[i];
		if (p.getAttribute("collapsible") != "yes")
			continue;
		toggle_class(p, "show-summary", "show-content");
	}
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
/* XXX - Should this take arguments like the feed to update, and a
 * callback function?
 */
function init_feeds_items()
{
	/* XXX - If we have feeds but no items, then ought to skip
	 * directly to getting a list of items, and update the feeds
	 * later.
	 */
	cache.update_feeds(false, feed_callback);

	function feed_callback(value)
	{
		/* Start the next AJAX request going. It'll take
		 * forever, so we'll do other stuff while that's
		 * going.
		 */
		cache.update_items(feed.id, 0, item_callback);

		feeds = value;
	}

	function item_callback(value)
	{
		// We're running for the first time, so get the top
		// (latest) articles from the cache.
		onscreen.items = cache.getitems(feed.id, null, 0, 25);
		cache.setItem("onscreen", onscreen);

		// Redraw itemlist
		redraw_itemlist();
	}
}

/* item2node
 * Create a DOM node from 'item' by substituting variables in the item
 * template, and creating a DOM node from that.
 */
// XXX - Should this go in Item.prototype?
function item2node(item)
{
	var item_feed = feeds[item.feed_id];

	// XXX - Check to make sure that feeds[item.feed_id] exists,
	// that we haven't been given an item from a nonexistent feed?
	if (item_feed == null)
	{
		console.error("Undefined feed "+item.feed_id);
		return null;
	}

	var title = item.displaytitle();

	// Fill in values to plug into item template
	var item_values = {
		id:		item.id,
		url:		encodeURI(item.url),
		url_attr:	(mobile != "" ?
				 'target="_blank"'
				 : ""),
			// On mobile devices, open title link in a new
			// window.
		title:		title,
		feed_url:	encodeURI(item_feed.url),
		feed_title:	item_feed.displaytitle(),
		author:		item.author,
			// XXX - If author is empty, shouldn't display
			// the "by" before author name.
			// XXX - If ever use author URL, might want to
			// wrap author name in <a href="mailto:...>.
		pub_date:	item.pub_date,
		pretty_pub_date:item.pub_date,
			// XXX - Pretty-print the date.
		summary:	item.summary,
		content:	item.content,
		comment_url:	encodeURI(item.comment_url),
		comment_rss:	encodeURI(item.comment_rss),
		// Indicate whether collapsible (both content and
		// summary exist), and whether to display summary or
		// content.
		collapsible:	(item.summary != null &&
				 item.content != null ?
				 "yes" : "no"),
		which:		(item.content == null ?
				 "summary" : "content"),
	};

	// XXX - The following hack, to create a DOM node from HTML,
	// should probably be in Template.js.
	var item_node = item_tmpl.expand(item_values);
	var new_node = document.createElement("div");
	new_node.innerHTML = item_node;
	item_node = new_node.firstChild;

	return item_node;
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

	/* Draw the items in onscreen.items */
	// XXX - If there are items above or below, add arrows to scroll
	// up or down.
	// XXX - Scroll to current item, current position.

	for (var i in onscreen.items)
	{
		var item = onscreen.items[i];
		if (item == null)
			break;
		if (item.is_read)
			continue;

		var item_node = item2node(item);
		item_node.item_id = item.id;
		item.node = item_node;

		// Append to itemlist.
		new_itemlist.appendChild(item_node);

		/* XXX - Keep pointers to the items, and any other
		 * interesting information.
		 */
	}

	// XXX - There's probably a better way to do this. With CSS,
	// maybe.
	if (new_itemlist.childNodes.length == 0)
	{
		// No items to display
		var new_node = document.createElement("p");
		new_node.innerHTML = "There are no articles to display.";
		new_itemlist.appendChild(new_node);
	}

	// Delete existing children
	while (itemlist.firstChild)
		itemlist.removeChild(itemlist.firstChild);

	// Add the new list
	itemlist.appendChild(new_itemlist);
}

function refresh()
// XXX - Perhaps ought to take an argument saying whether the top or
// bottom button was pressed, and prepend or append articles to
// onscreen.items accordingly.
{
	// XXX - Find the articles marked as read, and purge them from
	// cache.

	// XXX - update onscreen
	if (feeds != null &&
	    onscreen.items != null &&
	    onscreen.items.length > 0)
	{
		onscreen.items = cache.getitems(feed.id, null, 0, 25);
//		cache.update_items(feed.id, 0,
//				   function(){}
//				  );
		redraw_itemlist();
	}
}
