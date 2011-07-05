/* feeds.jsh					-*- JavaScript -*-
 * JavaScript functions for the feed view.
 */
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

document.addEventListener("DOMContentLoaded", init, false);

function init()
{
	window.addEventListener("keydown", handle_key, false);

	// Key bindings
	bind_key("d", toggle_details);
	bind_key("t", toggle_tools);
}

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
	var table = document.getElementById("feeds").childNodes[1];
debug("table == "+table);

	/* Iterate over each row, clearing status cell */
	for (var row = table.firstChild; row != table.lastChild; row = row.nextSibling)
	{
		if (row.firstChild == null)
			/* Skip #text nodes */
			// XXX - There must be a better way to do this
			continue;
debug("row == "+row.firstChild);
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
		var str = req.request.responseText.substr(req.last_off)

		/* Remember how much of the string we've gotten so far */
		req.last_off = req.request.responseText.length;

		/* Split the current input into lines */
		// XXX - This doesn't quite work: sometimes the buffer
		// contains part of an object. Perhaps ought to look
		// for /{.*}\n/g in the buffer.
		var lines = str.split("\n");
		for (var i = 0; i < lines.length; i++)
		{
			var line = lines[i];
if (line != "")
debug("line " + i + ": [" + lines[i] + "]");
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

			var status_cell = feed_row.firstChild;
			var title_cell = status_cell.nextSibling;

debug("feed id "+l.feed_id+", state "+l.state);
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
				title_cell.innerHTML = l.count_display;
				break;
			    case "error":
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
	var feed_list = document.getElementById("feeds");
	toggle_class(feed_list, "show-details", "hide-details");
}

function toggle_tools()
{
	var feed_list = document.getElementById("feeds");
	toggle_class(feed_list, "show-tools", "hide-tools");
}
