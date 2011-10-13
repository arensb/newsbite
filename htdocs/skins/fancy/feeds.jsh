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
//#include "js/load_module.js"
// XXX - Should block multiple updates from occurring in parallel.
#include "js/status-msg.js"

var feed_list;		// Table containing the list of feeds

document.addEventListener("DOMContentLoaded", init, false);

function init()
{
	feed_list = document.getElementById("feeds");

	window.addEventListener("keydown", handle_key, false);

	// Key bindings
	bind_key("d", toggle_details);
	bind_key("t", toggle_tools);

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
	toggle_class(feed_list, "show-details", "hide-details");
}

function toggle_tools()
{
	toggle_class(feed_list, "show-tools", "hide-tools");
}

function init_feed_list()
{
	feed_list = document.getElementById("feeds")

	// Request a list of feeds
	get_json_data("feeds.php",
		      { o: "json" },
		      receive_feed_list,
		      true);
}

function receive_feed_list(value)
{
	// XXX - Make sure value is a list

	/* XXX - Actually, this function should probably just
	 * - update the in-memory copy of the feed list
	 * - stash a copy in local storage
	 * - invoke a different function to redraw the list
	 */
	/* Create a document fragment containing the list of feeds, as
	 * table rows.
	 */
	var thelist = document.createDocumentFragment();

	/* XXX - This is bogus. Don't recreate the header line every
	 * time: the cells will have event handlers to sort by title
	 * or #unread. No need to recreate them each time.
	 */
	var header_line = document.createElement("tr");
	thelist.appendChild(header_line);

	// NB: We don't add the content of the row until later,
	// because Firefox is apparently too smart for its (or my)
	// good: evidently it thinks that at this point 'header_line'
	// is a free-floating node outside of a table, and therefore
	// mustn't contain any <td>s or <th>s, because those only go
	// inside tables. So it strips the <td>s and <th>s from
	// innerHTML.
	// So we have to wait until 'thelist' (containing
	// 'header_line') has been added to the table, below.

//header_line.innerHTML = '<td><i>hello world</i></td>';

	for (var i = 0; i < value.length; i++)
	{
		var feed = value[i];

		// XXX - Skip inactive feeds

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

		/* Number of unread articles */
		cell = document.createElement("td");
		add_class(cell, "count-col");
		cell.innerHTML = feed.num_unread;
		line.appendChild(cell);

		/* Title */
		var cell = document.createElement("td");
		add_class(cell, "title-col");
		var display_title;
		// Prefer nickname, if it's set
		if (feed.nickname == null || feed.nickname == "")
			display_title = feed.title;
		else
			display_title = feed.nickname;
		cell.innerHTML = '<a href="view.php?id='+feed.id+'">'+
			display_title +
			'</a>' +
			'&nbsp;<span class="feed-details">(' +
			'<a href="' + feed.url + '">site</a>' +
			'&nbsp;<a href="' + feed.feed_url + '">RSS</a>' +
			')</span>';
		line.appendChild(cell);

		/* Feed tools */
		cell = document.createElement("td");
		add_class(cell, "feed-tools");
		cell.innerHTML = '<a href="update.php?id='+feed.id+'" onclick="return update_feed('+feed.id+')">update</a>&nbsp;<a href="editfeed.php?id='+feed.id+'">edit</a>&nbsp;<a href="unsubscribe.php?id='+feed.id+'">unsub</a> <img src="skins/fancy/Attraction_transfer_icon.gif"/>';
		line.appendChild(cell);

		thelist.appendChild(line);
	}

	/* Delete the old contents of the feed div, and replace them with
	 * the new list.
	 */
	while (feed_list.firstChild)
		feed_list.removeChild(feed_list.firstChild);
	feed_list.appendChild(thelist);

	/* Add the header line that Firefox wouldn't allow us to add
	 * earlier.
	 */
	header_line.innerHTML = '<td>&nbsp;</td><th class="count-col">#</th><th class="title-col">Title</th><th class="feed-tools">Tools</th>';

	// XXX - Put the list in local storage, so it can be seen offline.
}
