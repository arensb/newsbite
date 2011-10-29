/* feeds.jsh					-*- JavaScript -*-
 * JavaScript functions for the feed view.
 */
#define DEBUG 0
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
	// XXX - C-r: refresh list of feeds.

	init_feed_list();
}

/* XXX - Updating feeds is all kinds of broken.
 * The back-end PHP script shouldn't return any HTML on how to display
 * the feed. That should be done here.
 * The various values should be in identifiable <span>s, so that we can
 * just replace an existing value.
 * For that matter, there should probably be a JS list of feed_id => row
 * that we can use to quickly update all the values we want to.
 */

/* update_feed
 * Update a feed, or all feeds.
 * This is intended to be called from inside an onclick="...", so
 * paradoxically we return 'false' if successful, and 'true' in case
 * of error.
 */
// XXX - Better to use PreventDefault (?) if successful, so we don't
// need the paradoxical 'return false'
// XXX - Use get_json_data() from js/xhr.js
// XXX - When receiving data, store in a local array.
// XXX - After receiving data (or perhaps after receiving an individual
// feed's data) store in local cache.
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
	var table = document.getElementById("feeds")/*.childNodes[1]*/;
debug("table == "+table);

	/* Iterate over each row, clearing status cell */
	// XXX - Ought to use table.getElementsByType("tr") or something.
	for (var row = table.firstChild; row != table.lastChild; row = row.nextSibling)
	{
		if (row.firstChild == null)
			/* Skip #text nodes */
			// XXX - There must be a better way to do this
			continue;
//debug("row == "+row.firstChild);
		row.firstChild.innerHTML = "";
		row.firstChild.style.backgroundColor = null;
	}
}

/* XXX - Bug: if the backend script times out, it stops giving
 * updates, and we can get into a state where a feed's indicator is
 * spinning, but there's never been a line to stop it. When get to the
 * end of the query's text (readyState 4), ought to find these and
 * turn them off. Perhaps use an error indicator.
 *
 * To do this, we need to keep track of each feed and its state, so we
 * know which feeds we have and haven't set the indicator for.
 */
function parse_response(req)
{
	debug("parse_response readyState: " + req.request.readyState);
//debug("responseText ("+req.request.responseText.length+"): ["+req.request.responseText+"]");
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
		var str = req.request.responseText.substr(req.last_off);

		/* Remember how much of the string we've gotten so far */
		req.last_off = req.request.responseText.length;

		/* Split the current input into lines */
		var lines = str.split("\n");
		for (var i = 0; i < lines.length; i++)
		{
			var line = lines[i];
if (line != "")
debug("line " + i + "("+req.last_off+"): [" + lines[i] + "]");
			if (line[0] != '{')
				// There might be non-JSON lines in there.
				// Ignore them. For that matter, ignore
				// any JSON lines that aren't objects.
				continue;
			try {
				// Inside a try{} in case the server sent
				// bad JSON.
				l = JSON.parse(line);
			} catch (e) {
				debug("Caught error " + e);

				// If this isn't a complete line, put it
				// back for later. Yeah, this is a bit of
				// a hack.
				if (i == lines.length-1)
					req.last_off -= line.length;
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

			// XXX - Really ought to look these up better. Don't
			// hardcode positions.
			var status_cell = feed_row.firstChild;
			var count_cell = status_cell.nextSibling;
			var title_cell = count_cell.nextSibling;

//debug("feed id "+l.feed_id+", state "+l.state);
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
				count_cell.innerHTML = l.counts.unread;
				break;
			    case "error":
				msg_add("Error in "+l.title+" ("+l.feed_id+"): "+l.error, 10000);
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
	toggle_class(feed_table, "show-details", "hide-details");
}

function toggle_tools()
{
	toggle_class(feed_table, "show-tools", "hide-tools");
}

function init_feed_list()
{
	// Get the cached copy of feeds, from last time.
	feeds = cache.feeds();
	if (feeds != null)
		redraw_feed_list();

	// Request a list of feeds
	get_json_data("feeds.php",
		      { o: "json" },
		      receive_feed_list,
		      true);
}

function receive_feed_list(value)
{
	// Make sure value is a list
	if (!value instanceof Array)
		// XXX - Error-reporting?
		return;

	// Create an array of Feed objects from what we just got.
	/* XXX - Ought to update the existing list: we might store
	 * state or something, and don't want to lose that just
	 * because the feed count got updated.
	 */
	var newfeeds = new Array();
	for (var i in value)
		newfeeds.push(new Feed(value[i]))
	feeds = newfeeds;

	cache.store_feeds(feeds);
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
	var sorted_feeds = feeds.sort(
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
	for (var i = 0; i < sorted_feeds.length; i++)
	{
		var feed = sorted_feeds[i];

		// XXX - Skip inactive feeds if user wants
		// XXX - Skip empty feeds if user wants

		var line = document.createElement("tr");
		line.feed_id = feed.id;
		line.setAttribute("id", "feed-"+feed.id);
		if (i & 1)
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
