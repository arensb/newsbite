/* wings.jsh			-*- JavaScript -*-
 */
/* XXX - Need a way to link directly to a specific feed, or a
 * specific, article, so that tabbed browsing will continue to work.
 * Presumably can accept "fid=1234" argument to display a specific
 * feed, and "id=12345" to display a specific article.
 *
 * Should these be full copies of the entire app, or just
 * stripped-down versions with only the information requested? And if
 * the former, how can it be made fast? For one thing, can start by
 * initializing the page the user wants. The other pages can be
 * initialized in the background, or after the user has something to
 * read.
 */
#define DEBUG	1
#if DEBUG
#  include "js/debug.js"
#else
function debug() { }
function clrdebug() { }
#endif	// DEBUG
#include "js/Template.js"
#include "js/ItemCache.js"
#include "js/CacheManager.js"

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
	/* Get the list of available pages */
	pages = document.getElementById("page-list").
		getElementsByClassName("page");

	/* Initialize the index page */
	feed_box = document.getElementById("feeds");

	/* XXX - Initialize the feed page */
	/* XXX - Initialize the article page */

	flip_to_page("index-page");
			// Show the main page

	ItemCache.scan_cache();

	feed_box.innerHTML = "<p>Loading feeds&hellip;</p>";

	feeds = CacheManager.get_feeds(
		function(value) {
			feeds = value;
			display_feeds();
		});
	display_feeds();

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

function display_feeds()
{
	/* Make a UL of feeds, and add it to feed_box. */
	var str = "<ol class=\"feed-list\">";

	if (feeds == undefined || feeds == null)
	{
		// XXX - Ought to do something smart
		feed_box.innerHTML = "<p>Waiting for feeds to show up</p>";
		return;
	}
	for (var i = 0; i < feeds.length; i++)
	{
		var f = feeds[i];

		// Ignore the inactive feeds
		if (!f.active)
			continue;
		str += feed_entry_tmpl.expand(f);
	}
	str += "</ol>";
	feed_box.innerHTML = str;
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
	var feed;

	for (var i = 0; i < feeds.length; i++)
	{
		if (feeds[i].id == id)
		{
			feed = feeds[i];
			break;
		}
	}
	// XXX - Fill in the feed page with TOC for the feed

	// XXX - This business of finding the various h1s and divs
	// that will be filled in with per-feed information should
	// happen just once, in init().
	// Create a hash named 'feed_page' or something, with all the
	// information we'll need, including references to the DOM
	// elements that need to be updated.
	var feed_page = find_page("feed-page");
	var h1_box = feed_page.getElementsByTagName("h1")[0];
	var subtitle_box = feed_page.getElementsByClassName("feed-subtitle")[0];
	var desc_box = feed_page.getElementsByClassName("feed-description")[0];

	if (feed.nickname == null || feed.nickname == "")
		h1_box.innerHTML = feed.title;
	else
		h1_box.innerHTML = feed.nickname;

	// XXX - Rather than messing with .style.display, use class
	// "hidden". Copy fancy skin's classes.js to do this.
	if (feed.subtitle == null || feed.subtitle == "")
		subtitle_box.style.display = "none";
	else {
		// XXX - Sanitize subtitle
		subtitle_box.innerHTML = feed.subtitle;
		subtitle_box.style.display = "block";
	}

	if (feed.description == null || feed.description == "")
		desc_box.style.display = "none";
	else {
		// XXX - Sanitize description
		desc_box.innerHTML = feed.description;
		desc_box.style.display = "block";
	}

	flip_to_page('feed-page');

	// XXX - Get articles from local cache

	// XXX - Get latest articles from the server
}

/* flip_to_page
 * Hide the currently visible page, and display the one named by 'page'.
 */
/* XXX - It'd be cool if these could slide back and forth in webkit.
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

/* find_page
 * Look through 'pages' (which should have been initialized by now)
 * and find the element whose id is 'name'.
 * Returns a reference to that page (a DOM element), or null if the
 * page couldn't be found.
 */
function find_page(name)
{
	for (var i = 0; i < pages.length; i++)
	{
		if (pages[i].id == name)
		{
			return pages[i];
		}
	}

	return null;
}
