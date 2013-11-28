/*						-*- JavaScript -*- */
#include "js/guess-mobile.js"
/*#include "js/defer.js"*/
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
document.addEventListener("online",
	function(ev)
	{
		msg_add("Now we're online.");
	},
	false);
document.addEventListener("offline",
	function(ev)
	{
		msg_add("Now we're offline.");
	},
	false);

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

/* Hack for Date object on Android
 * Apparently the Android browser's Date object can't even parse its
 * own toJSON() output.
 *
 * First, we test to see if this is the case by creating a new Date(),
 * converting it to a JSON string, parsing that string, creating a
 * Date from that, and checking whether it's valid (if it is,
 * isNaN(thedate.getTime()) should be false).
 *
 * If the date is invalid, we redefine the toJSON method. The simple
 * approach would be to just use 'return this.valueOf()', but that
 * returns milliseconds since the epoch. In other code, we assume that
 * a numeric string should be interpreted as a Unix time_t, i.e.,
 * seconds since the epoch. So instead we construct a string
 * representation of the time.
 */
if (isNaN(new Date(JSON.parse(JSON.stringify(new Date()))).getTime()))
	Date.prototype.toJSON = function() {
		var year = this.getUTCFullYear();
		var mon  = this.getUTCMonth() + 1;
		var date = this.getUTCDate();
		var hour = this.getUTCHours();
		var min  = this.getUTCMinutes();
		var sec  = this.getUTCSeconds();
		// Return "yyyy-mm-dd HH:MM:SS GMT"
		return year + "-" +
			(mon  < 10 ? "0"+mon  : mon)  + "-" +
			(date < 10 ? "0"+date : date) + " " +
			(hour < 10 ? "0"+hour : hour) + ":" +
			(min  < 10 ? "0"+min  : min)  + ":" +
			(sec  < 10 ? "0"+sec  : sec) +
			" GMT";
	};

var mark_read = {};		// Hash of item_id -> is_read? values
var mark_request = null;	// Data for marking items as read/unread
var current_item = null;	// Current item, for keybindings and such

var cache = new CacheManager();	// Cache manager for locally-stored data
var feeds;		// List of feeds
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
var page_top_tmpl = new Template(page_top_tmpl_text);

var query_args = {};		// GET arguments passed in the URL
// Parse the GET arguments.
(function()
{
	// XXX - For debugging: error if a script passes in a query
	// string the wrong way.
	if (window.location.search.length > 0)
		console.error("Someone passed in a search string: %s",
			      window.location.search);

	var query = (window.location.hash.length > 0 ?
		     window.location.hash :
		     window.location.search)
		.substring(1);
		// If window.location.hash is nonempty, then we're
		// looking at a URL of the form "...#a=x&b=y", so use
		// the hash. Otherwise, use window.location.search,
		// because we were given "...?a=x&b=y".
		//
		// The ".substring(1)" chops off the leading "#" or "?".
	var vars = query.split("&");	// Split by variable assignment

	// Parse each variable assignment
	for (var i = 0; i < vars.length; i++)
	{
		var pair = vars[i].split("=");	// Split "var=value"
			// If 'vars[i]' is of the form "var" rather than
			// "var=value", then pair[1] should be the empty
			// string.

		if (pair.length != 2)
			// We only want "var=value" (for now).
			continue;

		query_args[pair[0]] =
			decodeURIComponent(pair[1].replace(/\+/g, " "));
			// First, we replace any "+"es in the value
			// with spaces. Then we decode the value as a
			// URI component.

		// XXX - We don't decode the variable name, so if it's
		// not ASCII, it won't work. Do we care?
	}
})()

// XXX - Should add "hashchange" event listener to window, to detect
// when window.location.hash changes.
// Firefox: event has
//	oldURL (what it used to be)
//	newURL (what it has changed to)
// Then again, that might be a bad idea: it's probably best to use
// some function that knows what's going on (and replaces the page,
// and things like that), than to just change the .hash and hope for
// the best. So don't use this to update the page.

var feed_id;		// Which feed are we looking at?
if ('id' in query_args)
{
	// XXX - Check to make sure this is either "all", or a
	// (string representation of a)n integer.

	feed_id = query_args['id'];
} else {
	console.warn("No feed_id given. Using \"all\"");
	feed_id = "all";
}

var mydomain = location.protocol + "//" +
	location.host;
	// Perhaps it'd be nice to add the subdirectory.

function init()
{
	itemlist = document.getElementById("itemlist");

	// Bind some events
	PatEvent.bind_event(itemlist, "click", ".collapse-bar",
			    toggle_pane, false);
	PatEvent.bind_event(itemlist, "click", ".expand-bar",
			    toggle_pane, false);
	PatEvent.bind_event(itemlist, "click", ".mark-check",
			    button_mark_item, false);

	window.addEventListener("keypress", handle_key, false);
		// XXX - This should probably go in js/keybindings.js

	window.addEventListener("orientationchange", reorient, false);

//	window.addEventListener("scroll", scroll_handler, false);
			// Scroll handler to detect when window has
			// moved.
//window.addEventListener("storage",
//	function(ev) {
//		console.log("Got a storage event: %s was %s, now %s",
//			    ev.key, ev.oldValue, ev.newValue);
//	},
//	false);

	/* On desktop, keep track of current item, and add key bindings
	 * to navigate.
	 */
	if (mobile == "")
	{
		PatEvent.bind_event(itemlist, "_enter", ".item",
				    enter_item, false);

		// Key bindings
		bind_key("C-r", slow_sync);
		bind_key("S-r", slow_sync);	// XXX - Only on Chrome
		bind_key(".", slow_sync);	// Same as Twitter
		bind_key("S-c", collapse_all);
		bind_key("S-e", expand_all);
	}

	/* Key bindings that I want on the Android */
	bind_key("d", key_mark_item);
	bind_key("c", toggle_collapse_item);
	bind_key("k", move_up);
	bind_key("j", move_down);
bind_key("l", recenter);

	// Get feeds and items from cache.
	feeds = cache.feeds();
if (feed_id == "all" ||
    feed_id in feeds)
{
console.log("I have this feed: ["+feed_id+"]");
}else
console.log("I don't have this feed: ["+feed_id+"]");
	set_feed_fields();	// XXX - Set the page title,
				// description, and so on.

	// Fetch the list of what was on screen last time we started
	// XXX - Initialize it if empty. Or initialize any missing
	// bits even if not empty.

	if (onscreen == null)
	{
		onscreen = {
			cur_item:	null,
			cur_xpos:	0,
			cur_ypos:	0,
		};
		onscreen.items = cache.getitems(feed_id, null, 0, 25);
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
		      { "mark-read":	mark_request.read.join(","),
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
		item.mtime = new Date();
		cache.store_item(item);
				// Mark in the cache as well.
	}

// XXX - Deferring the following block seems to improve performance
// (in that mark_item1() is no longer the function in which we spend
// most of our time. OTOH I think it breaks scrolling, particularly at
// the bottom of the page:
//
// If the reader marks the bottommost article as read, it gets
// collapsed to a thin sliver. The code after the defer()red block
// tries to scroll so that the top of this sliver is at the top of the
// window, and fails because the window contents aren't that tall
// (there's nothing at the bottom).
//
// Then the deferred code runs, and brings up a much taller new
// article, which could easily fill the page. If things are fast
// enough, the user shouldn't notice anything. But the article he just
// marked is at the bottom of the page, rather than at the top, where
// he expected.
//
// Note that this is probably only a problem at the bottom of the
// page. A better approach might be to monitor the viewport's
// movements (by capturing page scroll events or something), and
// updating the viewport so that there are always 10 items above and
// below what's currently visible.

//(function() {
	/* XXX - If marking an item as read, bring up another item
	 * from cache.
	 * - If there are now > 25 items on screen, delete the topmost one.
	 * - If the deleted item was marked read, purge it from cache.
	 */
	// XXX - Check whether item is in a known feed, and if not,
	// delete it from cache?
	addnewnode:
	if (item_div.is_read)
	{
		var last_item = onscreen.items[onscreen.items.length-1];
				// Get last item in onscreen.items.
		var more_items = cache.getitems(feed_id, last_item, 0, 1);
				// Get the next one after that in cache
		if (more_items == null || more_items.length < 2)
			// If getitems() returned < 2 items, then
			// there aren't any more items in cache after
			// last_item.
			break addnewnode;	// Break out of if-block

		var new_item = more_items.pop();
				// The new item we're about to add.
//console.log("new_item:\n%o", new_item);
		var new_node = item2node(new_item);

		if (new_node != null)
			// XXX - This is ugly code. new_node shouldn't
			// be null. If it is, it's probably because
			// it's a member of a nonexistent feed.
		{
			new_node.item_id = new_item.id;
			new_item.node = new_node;

			itemlist.appendChild(new_node);
			onscreen.items.push(new_item);
		}
	}
//}).defer(1, null);

	/* Scroll so that the (collapsed) item is visible.
	 * If the user has scrolled such that the top of the item is
	 * no longer visible, scroll so that the top of the
	 * now-collapsed article is at the top of the window.
	 */
	if (item_div.is_read &&
	    item_div.offsetTop < window.pageYOffset)
	{
		// XXX - This isn't quite right: item_div.offsetTop is
		// the offset from the top of item_div's container to
		// the to of item_div (or something like that). What
		// we really want is the offset from the top of the
		// page to the top of item_div.
		window.scrollTo(window.pageXOffset, item_div.offsetTop);
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

/* move_up
 * Make the previous item the current one.
 */
function move_up()
{
	if (onscreen.items.length == 0)
		return;

	if (current_item == null)
	{
		/* No currently-selected item */
		// XXX - Instead of selecting the bottommost item,
		// perhaps ought to find the bottommost one in the
		// visible window.
		current_item = onscreen.items[onscreen.items.length-1].node;
		add_class(current_item, "current-item");
	} else {
		remove_class(current_item, "current-item");

		/* Find the current item in onscreen.items, so we can
		 * get the next one.
		 */
		var i;
		for (i = 0; i < onscreen.items.length; i++)
		{
			if (onscreen.items[i].node == current_item)
				// Found it.
				break;
		}
		if (i == 0 || i >= onscreen.items.length)
		{
			/* Couldn't find current item, or we're at the
			 * last item.
			 */
			// XXX - Perhaps oughtto load the next item
			// from cache.
		} else {
			current_item = onscreen.items[--i].node;
		}
	}

	add_class(current_item, "current-item");

	// Scroll so that the new current item is at the top
	window.scrollTo(0, current_item.offsetTop);
}

/* move_down
 * Make the next item the current one.
 */
function move_down()
{
	if (onscreen.items.length == 0)
		return;

	if (current_item == null)
	{
		/* No currently-selected item */
		// XXX - Instead of selecting the topmost item,
		// perhaps ought to find the topmost one in the
		// visible window.
		current_item = onscreen.items[0].node;
		add_class(current_item, "current-item");
	} else {
		remove_class(current_item, "current-item");

		/* Find the current item in onscreen.items, so we can
		 * get the next one.
		 */
		var i;
		for (i = 0; i < onscreen.items.length; i++)
		{
			if (onscreen.items[i].node == current_item)
				// Found it.
				break;
		}
		if (i >= onscreen.items.length - 1)
		{
			/* Couldn't find current item, or we're at the
			 * last item.
			 */
			// XXX - Perhaps oughtto load the next item
			// from cache.
		} else {
			current_item = onscreen.items[++i].node;
		}
	}

	add_class(current_item, "current-item");

	// Scroll so that the new current item is at the top
	window.scrollTo(0, current_item.offsetTop);
}

/* enter_item
 * Called when mouse has entered an item. Highlight it and mark it as
 * current.
 */
function enter_item(ev)
{
	// Unmark the previous current item, if there is one.
	if (current_item != null &&
	    is_in_class(current_item, "item"))
		remove_class(current_item, "current-item");

	var elt = ev.currentTarget;
	if (!is_in_class(elt, "item"))
		return false;
	add_class(elt, "current-item");
	current_item = elt;
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
function init_feeds_items()
{
	/* Inner helper functions */
	function feed_callback(value)
	{
		/* Start the next AJAX request going. It'll take
		 * forever, so we'll do other stuff while that's
		 * going.
		 */
msg_add("starting slow_sync 1");
		cache.slow_sync(feed_id,
				item_callback,
				function(status, msg) {
					msg_add("slow_sync 1 failed ("+status+"):"+msg);
				});

		feeds = value;

		// XXX - Now that we have the most current value for
		// feeds, update the page: title, description, etc.
	}

	function item_callback(value)
	{
msg_add("slow_sync 1 done");

		// We're running for the first time, so get the top
		// (latest) articles from the cache.
		onscreen.items = cache.getitems(feed_id, null, 0, 25);
		cache.setItem("onscreen", onscreen);

		// Redraw itemlist
		redraw_itemlist();
	}

	/* init_feeds_items() main */
msg_add("calling update_feeds");
	cache.update_feeds(feed_callback);
}

/* item2node
 * Create a DOM node from 'item' by substituting variables in the item
 * template, and creating a DOM node from that.
 */
// XXX - Should go in Item.prototype. Then again, this is very
// skin-specific, so maybe not.
function item2node(item)
{
	var item_feed = feeds[item.feed_id];

	// Check to make sure that feeds[item.feed_id] exists; that we
	// haven't been given an item from a nonexistent feed
	if (item_feed == null)
	{
		console.error("Undefined feed "+item.feed_id);
		// XXX - Ought to delete this item from cache.
		// cache.purge_item(item.id).defer(something)
		// Need to specify 'cache' as the object it applies to.
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
		pretty_pub_date:item.pub_date.toDateString() + ", " +
				item.pub_date.toTimeString(),
			// Pretty-print the date.
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

	// Mark articles > 1 day old
	var yesterday = new Date();
	yesterday.setDate(yesterday.getDate()-1);
	if (item.pub_date < yesterday)
		add_class(item_node, "old1d");

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
		if (item_node == null)
			// XXX - This should probably never happen. If
			// it does, it's likely because the item is in
			// a nonexistent or unknown feed, and should
			// therefore be removed from the cache.
			continue;
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

/* set_feed_fields
 * Set various fields in the page to whichever feed we're displaying at the
 * moment.
 */
function set_feed_fields()
{
	var feed;

	if (feed_id == "all")
	{
		feed = {
			id:		"all",
			title:		"All feeds",
			url:		null,
			subtitle:	"",
			description:	"",
		};
	} else if (!feed_id in feeds)
	{
		console.error("Can't find feed_id %d in feeds.", feed_id);
		return;
	} else
		feed = feeds[feed_id];
	// XXX - On a newly-subscribed feed, this will be null. Ought
	// to refresh the feed list, and try again. But also should
	// make sure not to go into an infinite loop (i.e., someone
	// could go to "view.php#id=9999", where 9999 is a nonexistent
	// feed ID; set_feed_fields() would see that it doesn't have a
	// feed 9999, request a new feed list from the server, call
	// set_feed_fields() again, and the cycle would continue. We
	// want to avoid that.)
	if (feed == null)
	{
		alert("Feed information not cached. Please refresh index");
		return;
	}

	// Set the page title.
	try {
		var title_field = document.getElementsByTagName("title")[0];
		title_field.innerHTML = "Newsbite: "+feed.title;
	} catch (e) {
		// Android browser throws
		// NO_MODIFICATION_ALLOWED_ERR: DOM Exception 7
		// XXX - Ought to check that this is in fact the error
		// we got, and re-throw if it isn't.
	}

	// Expand the page-top template, which has a bunch of stuff
	// like the feed title, subtitle, description, as well as URLs
	// that have the feed ID as an argument.
	var page_top = document.getElementById("page-top");
	page_top.innerHTML = page_top_tmpl.expand(feed);
}

function reorient(ev)
{
	var orientation = "up";	// Direction of top of device
	switch (window.orientation)
	{
	    case 0:		// Portrait, right-side up
		orientation = "up";
		break;
	    case 90:		// Landscape, top facing left
		orientation = "left";
		break;
	    case -90:		// Landscape, top facing right
		orientation = "right";
		break;
	    case 180:		// Portrait, upside-down
		orientation = "down";
		break;
	    default:
		// This should never happen
		orientation = "up";
		break;
	}

	document.body.setAttribute("orientation", orientation);
}

function slow_sync()
{
msg_add("starting slow_sync 2");
	cache.slow_sync(feed_id,
			function() {
				// Once it's done, redraw as necessary
				msg_add("Returned from slow_sync");
				onscreen.items = cache.getitems(feed_id, null, 0, 25);
				cache.setItem("onscreen", onscreen);
				redraw_itemlist()
			},
			function(status, msg) {
				msg_add("slow_sync 2 failed ("+status+"):"+msg);
			});
}

/* scroll_handler
 * Gets called when the user scrolls the window (event "scroll").
 *
 * XXX - A few problems/idiosyncracies to keep in mind: on the iPad,
 * if you flick the page down, this event gets invoked only when the
 * page has stopped moving, which can take a second or two.
 *
 * On Android, this works, but it looks as though maybe not every
 * single scroll fires off an event.
 *
 * In Firefox and Safari, a single flick of the mouse wheel can
 * trigger multiple "scroll" events. So presumably the Right Way to
 * handle this is to have scroll_handler schedule a DTRT function some
 * time (1sec?) in the future. scroll_handler should check whether
 * this function has already been scheduled, and not do anything.
 * Perhaps measure how quickly events get generated, and set the delay
 * short enough that the page feels responsive, but long enough that
 * we're likely to run the update once the user has stopped scrolling.
 */
function scroll_handler(ev)
{
//	msg_add("scroll("+ev+")");
// From https://developer.mozilla.org/en/DOM/window.onscroll
//	alert("scroll event detected! "+window.pageXOffset+" "+window.pageYOffset);
//	note: you can use window.innerWidth and window.innerHeight to
//	access the width and height of the viewing area
}

/* recenter
 * Figure out what the user it looking at (i.e., where the top of the
 * window is, wrt to the list of articles), and ensure that there are
 * 10 unread articles above and below the current one.
 */
function recenter()
{
	var items = document.querySelectorAll("#itemlist article");
//console.log("recenter: items: %d: %o", items.length, items);
	var topmost_item_div = null;

	/* Find the topmost visible item: the first item whose bottom
	 * is not above the top of the window/viewport.
	 */
	// XXX - This could use some improvement: this might catch an
	// item where only the bottommost row of pixels is visible,
	// which seems intuitively bogus. So for one thing, it'd be
	// better to ignore the bottom border and check whether the
	// content pane is visible.
	//
	// Then we might have a case where only the bottommost pixel
	// of the last row of letters is visible. So instead of
	// comparing to the top of the window, we might want to draw
	// an imaginary line a centimeter or two down the window, and
	// compare to that.
	for (var i = 0, l = items.length; i < l; i++)
	{
		var item = items[i];
//console.log("Examining item %o", item);
		var box = item.getBoundingClientRect();
				// Reminder: 'box' coordinates are
				// relative to the viewport.
//console.log("box top: %d, height, %d, both: %d", box.top, box.height, box.top+box.height);

		// See whether this item's bottom edge is above the
		// top of the window.
		if (box.bottom < 0)
			continue;

		topmost_item_div = item;
		break;
	}

//console.log("topmost_item_div:");
//console.log(topmost_item_div);
	// XXX - Debugging/tracing:
	if (topmost_item_div != null)
	{
		topmost_item_div.style.backgroundColor = "red";
	}

	// Get ID of current item.
	var item_id;
	if (topmost_item_div == null)
		// No items on page
		item_id = null;
	else
		item_id = topmost_item_div.item_id;
console.log("Topmost item %o, ID: %d", topmost_item_div, item_id);

	// XXX - Get the corresponding item in onscreen
	var topmost_item = null;
	for (var i = 0, l = onscreen.items.length; i < l; i++)
	{
		var item = onscreen.items[i];

		if (item.id == item_id)
		{
			topmost_item = item;
			break;
		}
	}

	if (topmost_item == null)
	{
		console.error("onscreen.items doesn't have an item with ID %d", item_id);
		return;
	}

	// Get the surrounding items
console.log("Calling cache.getitems(%s, %s, ...)", feed_id, item_id);
	var newitems = cache.getitems(feed_id, topmost_item, 2, 2);
			// XXX - 2 should be 10, or something, in production.
//console.log("newitems:");
//console.log(newitems);

	// XXX - Find topmost_item in newitems.
	// XXX - What if the topmost item is marked read?

	// XXX - Remove all but 10 read items above topmost_item.
	// XXX - Make the parts above topmost_item match newitesm
	// XXX - Remove all but 10 read items below topmost_item.
	// XXX - Make the parts below topmost_item match newitesm
}
