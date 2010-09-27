/* wings.jsh
 */
#if DEBUG
#  include "js/debug.js"
#else
function debug() { }
function clrdebug() { }
#endif	// DEBUG
#include "js/xhr.js"
#include "js/Template.js"

var feed_entry_tmpl = new Template(
"<li><a class=\"feed-name\" onclick=\"show_feed(@id@)\">@id@: @title@</a></li>"
);

document.addEventListener("DOMContentLoaded", init, false);
			// Perform initialization once the DOM is loaded

var feed_box;		// Div listing available feeds
var feeds;		// List of known feeds
var pages;		// List of page divs
var current_page;	// Currently-visible page

function init()
{
	/* XXX - Console logging */

	pages = document.getElementById("page-list").
		getElementsByClassName("page");
			// Get the list of available pages

	feed_box = document.getElementById("feeds");

	flip_to_page("index-page");
			// Show the main page

	feeds = get_feeds();	// Get list of feeds from local storage
	display_feeds(feeds);
	fetch_feeds();

	/* XXX - Update the local store and see if anything needs to be
	 * deleted.
	 */
	/* XXX - Pre-fetch the latest articles from all feeds from the
	 * server.
	 */

	/* XXX - In Safari, can log to the console:
	 * console.log("log message");
	 * console.error("error message");
	 * console.warn("warn message");
	 * console.info("info message");
	 */
}

/* get_feeds
 * Get list of feeds from browser-side storage.
 */
function get_feeds()
{
	var feed_str = localStorage.getItem("feeds");

	if (feed_str == undefined)
		// No list of feeds
		return undefined;

	/* To get the feeds, eval the string */
	var feed_list;
	try {
		eval("feed_list = " + feed_str);
	} catch(e) {
		alert("Caught an error");
	}

	return feed_list;
}

function display_feeds(feed_list)
{

	/* Make a UL of feeds, and add it to feed_box. */
	var str = "<ol class=\"feed-list\">";

	if (feed_list == undefined)
	{
		// XXX - Ought to do something smart
		return;
	}
	for (var i = 0; i < feed_list.length; i++)
	{
		var f = feed_list[i];

		// Ignore the inactive feeds
		if (!f.active)
			continue;
		str += feed_entry_tmpl.expand(f);
	}
	str += "</ol>";
	feed_box.innerHTML = str;
}

/* fetch_feeds
 * Fetch latest list of feeds from the server, and save them to local
 * storage.
 */
function fetch_feeds()
{
	var request = createXMLHttpRequest();

	if (!request)
		return false;

	request.open('GET',
		     "feeds.php?o=jsonr",
		     true);
	request.onreadystatechange =
		function(){ fetch_feeds_callback(request) };
	request.send(null);
}

function fetch_feeds_callback(req)
{
	if (req.readyState != 4)
		return;

	localStorage.setItem("feeds", req.responseText);

	// XXX - Let the rest of the code know that the list of feeds
	// has been updated. This really ought to be done by an event
	// and a corresponding handler.
	get_feeds();
}

function get_feed_by_id(id, feed_list)
{
	for (var i = 0; i < feed_list.length; i++)
	{
		if (feed_list[i].id == id)
			return feed_list[i];
	}
	return null;
}

function show_feed(id)
{
	var feed = get_feed_by_id(id, feeds);

	// XXX - Fill in the feed page with TOC for the feed
	// XXX - Get articles from local cache

	flip_to_page('feed-page');

	// XXX - Get latest articles from the server
}

/* flip_to_page
 * Hide the currently visible page, and display the one named by 'page'.
 */
function flip_to_page(page)
{
	for (var i = 0; i < pages.length; i++)
	{
		var p = pages[i];
		if (p.id == page)
			p.style.display = "block";
		else
			p.style.display = "none";
	}
}
