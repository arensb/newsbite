/* feeds.jsh					-*- JavaScript -*-
 * JavaScript functions for the feed view.
 */
#include "js/guess-mobile.js"
// #include "js/defer.js"
#include "js/xhr.js"
#include "js/classes.js"
#include "js/keybindings.js"
#include "js/PatEvent.js"
#include "js/types.js"
#include "js/Template.js"
#include "js/CacheManager.js"
//#include "js/load_module.js"
// XXX - Should block multiple updates from occurring in parallel.
#include "js/status-msg.js"

var cache = new CacheManager();		// Cache manager for locally-stored data
var feeds;		// Master list of all feeds
var feed_title_tmpl = new Template(feed_title_tmpl_text);
var feed_tools_tmpl = new Template(feed_tools_tmpl_text);
// XXX - Should the show_* variables be put in localStorage?
var show_empty = true;
var show_inactive = true;
var show_stale = true;

// Interesting DOM nodes we want to keep track of
var feed_table = {};	// Pointers to interesting elements inside

document.addEventListener("DOMContentLoaded", init, false);

function init()
{
	feed_table = document.getElementById("feeds");

	window.addEventListener("keydown", handle_key, false);

	// Key bindings
	bind_key("d", toggle_details);
	bind_key("t", toggle_tools);
	bind_key("C-r", refresh_feed_list);	// C-r: refresh list of feeds.
	bind_key(".", refresh_feed_list);	// Same as Twitter

	init_feed_list();
}

/* update_feed
 * Update a feed, or all feeds.
 * This is intended to be called from inside an onclick="...", so
 * paradoxically we return 'false' if successful, and 'true' in case
 * of error.
 */
// XXX - Better to use PreventDefault (?) if successful, so we don't
// need the paradoxical 'return false'. But we'd need an event object
// to call it on, so we'd need to attach a "click" listener to the
// individual links. Perhaps with PatEvent.
// XXX - When receiving data, store in a local array.
// XXX - After receiving data (or perhaps after receiving an individual
// feed's data) store in local cache.
function update_feed(id)
{
	/* Inner helper functions */

	/* update_feed_handler
	 * Take a status line from 'update.php' and display the upshot.
	 */
	/* XXX - Bug: if the backend script times out, it stops giving
	 * updates, and we can get into a state where a feed's indicator is
	 * spinning, but there's never been a line to stop it. When get to the
	 * end of the query's text (readyState 4), ought to find these and
	 * turn them off. Perhaps use an error indicator.
	 *
	 * To do this, we need to keep track of each feed and its state, so we
	 * know which feeds we have and haven't set the indicator for.
	 */
	function update_feed_handler(line)
	{
		if (line.feed_id == undefined)
		{
			// no feed_id.
			// XXX - Tell user, perhaps?
			return;
		}

		var feed_row = document.getElementById("feed-" + line.feed_id);
		if (feed_row == undefined)
		{
			// Can't find row feed-$id
			// XXX - Do something intelligent
			return;
		}

		// XXX - Really ought to look these up better. Don't
		// hardcode positions.
		var status_cell = feed_row.firstChild;
		var count_cell = status_cell.nextSibling;
		var title_cell = count_cell.nextSibling;

		// XXX - Paths to images / skin name shouldn't be
		// hardcoded. Perhaps add a class to the cell.
		switch (line.state)
		{
		    case 'start':
			status_cell.innerHTML = '<img src="' +
				skin_dir +
				'/Loading.gif"/>';
			break;
		    case "end":
			status_cell.innerHTML = '&nbsp;';
			status_cell.style.backgroundColor = null;
			count_cell.innerHTML = line.counts.unread;
			break;
		    case "error":
			var title;
			if (line.feed_id in feeds)
				title = feeds[line.feed_id].title;
			else
				title = "feed "+line.feed_id;
			msg_add("Error in "+title+" ("+line.feed_id+"): "+line.error, 10000);
			// XXX - The 'alt=' and/or 'title=' means you
			// can get error message by hovering pointer
			// over the error icon, but it'd probably be
			// better to use a CSS-based tooltip.
			status_cell.innerHTML =
				'<img src="' +
				skin_dir +
				'/Attention_niels_epting.png" title="' +
				line.error +
				'" alt="' +
				line.error +
				'"/>';
			break;
		    default:
			status_cell.innerHTML = line.state;
			break;
		}
	}

	/* update_feed() main */

	// XXX - If updating one feed, ought to only clear that line
	clear_status();

	get_json_data("update.php",
		      { o:	"json",
			id:	id,
		      },
		      update_feed_handler,
		      function(status, msg) {
			      console.error("update_feed error: status "+
					    status +
					    ", msg " + msg);
		      },
		      false);

	return false;
}

function clear_status()
{
	var table = document.getElementById("feeds");
	var feed_tbody = table.getElementsByTagName("tbody")[0];

	/* Iterate over each row, clearing status cell */
	// XXX - Ought to use table.getElementsByType("tr") or something.
	for (var row = feed_tbody.firstChild; row != feed_tbody.lastChild; row = row.nextSibling)
	{
		if (row.firstChild == null)
			/* Skip #text nodes */
			// XXX - There must be a better way to do this
			continue;
		row.firstChild.innerHTML = "";
		row.firstChild.style.backgroundColor = null;
	}
}

function toggle_details()
{
	toggle_class(feed_table, "show-details", "hide-details");
}

function toggle_tools()
{
	toggle_class(feed_table, "show-tools", "hide-tools");
}

/* toggle_show_empty
 * Toggle between showing and hiding empty feeds: ones with no unread
 * articles.
 */
function toggle_show_empty()
{
	show_empty = !show_empty;
	redraw_feed_list();
}

/* toggle_show_inactive
 * Toggle between showing and hiding inactive feeds.
 */
function toggle_show_inactive()
{
	show_inactive = !show_inactive;
	redraw_feed_list();
}

/* toggle_show_stale
 * Toggle between showing and hiding stale feeds (ones with no recent
 * articles).
 */
function toggle_show_stale()
{
	show_stale = !show_stale;
	redraw_feed_list();
}

function init_feed_list()
{
	// Get the cached copy of feeds, from last time.
	feeds = cache.feeds();
	if (feeds != null)
		redraw_feed_list();

	// XXX - Ought to set a spinny icon.

	// Request a list of feeds
	cache.update_feeds(receive_feed_list);
}

/* refresh_feed_list
 * Get a new set of item counts and such from the server, and redraw
 * the list.
 */
function refresh_feed_list()
{
	// XXX - Ought to set a spinny icon.
	// Request a list of feeds
	cache.update_feeds(receive_feed_list);
}

function receive_feed_list(value)
{
	// XXX - Clear spinny icon.

	// Make sure value is a list
	// XXX - Shouldn't be necessary.
	if (!value instanceof Array)
		return;

	// Create an array of Feed objects from what we just got.
	/* XXX - Ought to update the existing list: we might store
	 * state or something, and don't want to lose that just
	 * because the feed count got updated.
	 */
	feeds = value;

	redraw_feed_list();
}

function redraw_feed_list()
{
	if (feeds == null)
		// Special case: no feeds (or something). Use an empty
		// array.
		feeds = new Array();

	/* Create a document fragment containing the list of feeds, as
	 * table rows.
	 */
	var thelist = document.createDocumentFragment();

	// XXX - Make this a separate function (a method on FeedList,
	// perhaps?) so that we can sort either by name, or by number
	// of unread articles (or anything else we might add).
	// XXX - The sorting functions should be able to handle
	// reverse sort order as well. Perhaps can just use
	// Array.reverse().
	var sorted_feeds = new Array();

	// Copy values from feeds (hash) into sorted_feeds (array)
	for (i in feeds)
		sorted_feeds.push(feeds[i]);
	// Now sort feeds by title
	sorted_feeds.sort(
		function(a, b)
		{
			var as = a.sortname();
			var bs = b.sortname();
			if (as < bs)
				return -1;
			else if (as > bs)
				return 1;
			return 0;
		});
	for (var i = 0, row = 0; i < sorted_feeds.length; i++)
	{
		var feed = sorted_feeds[i];

try {
		// Skip empty feeds if user wants.
		if (feed.num_unread == 0 && !show_empty)
			continue;
} catch (e) {
console.error("feed.num_unread error: %s", e);
console.log("feed:\n%o", feed);
try {
	console.trace();
} catch (e) { /* Ignore */ }
}

		// Skip inactive feeds if user wants.
		if (feed.active != 1 && !show_inactive)
			continue;

		// Skip stale feeds if user wants.
		if (feed.stale == 1 && !show_stale)
			continue;

		var line = document.createElement("tr");
		line.feed_id = feed.id;
		line.setAttribute("id", "feed-"+feed.id);
		if (row & 1)
			add_class(line, "odd-row");
		else
			add_class(line, "even-row");

		if (feed.active != 1)
			add_class(line, "inactive-feed");

		if (feed.stale == 1)
			add_class(line, "stale-feed");

		/* Status indicator */
		cell = document.createElement("td");
		add_class(cell, "icon-col");
		cell.innerHTML = "&nbsp;";
		line.appendChild(cell);
		line.status_cell = cell;

		/* Number of unread articles */
		cell = document.createElement("td");
		add_class(cell, "count-col");
		if (feed.num_unread == null)
		{
			// I think 'num_unread' doesn't get cached. So
			// if we're just starting out, and don't have
			// a value, don't display "undefined".
console.log("wha? "+feed.num_unread);
			cell.innerHTML = "?";
		} else
			cell.innerHTML = feed.num_unread;
		line.appendChild(cell);
		line.count_cell = cell;

		/* Title */
		var cell = document.createElement("td");
		add_class(cell, "title-col");
		feed.display_title = feed.displaytitle();
		// XXX - This is arguably bogus: we're using the Feed
		// object both in its capacity as an object, and also
		// as a hash of template values.
		// Having a separate hash with template values would
		// be cleaner.
		cell.innerHTML = feed_title_tmpl.expand(feed);
		line.appendChild(cell);
		line.title_cell = cell;

		/* Feed tools */
		cell = document.createElement("td");
		add_class(cell, "feed-tools");
		cell.innerHTML = feed_tools_tmpl.expand(feed);
		line.appendChild(cell);
		line.tools_cell = cell;

		thelist.appendChild(line);
		row++;
	}

	/* Delete the old contents of the feed div, and replace them with
	 * the new list.
	 */
	var feed_tbody = feed_table.getElementsByTagName("tbody")[0];
	while (feed_tbody.firstChild)
		feed_tbody.removeChild(feed_tbody.firstChild);
	feed_tbody.appendChild(thelist);

	// XXX - Put the list in local storage, so it can be seen offline.
}
