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
//#include "js/ItemCache.js"
#include "js/CacheManager.js"

var feed_entry_tmpl = new Template(
"<li><a class=\"feed-name\" onclick=\"show_feed(@id@)\">@id@: @title@</a> (@num_unread@)</li>"
);
var item_entry_tmpl = new Template(
"<li><a class=\"article-title\" onclick=\"show_item(@id@)\">@id@: @title@</a></li>"
);

document.addEventListener("DOMContentLoaded", init, false);
			// Perform initialization once the DOM is loaded

var cm;			// Cache manager
var pages;		// List of page divs
var current_page;	// Currently-visible page
var current_feed_id;	// The ID of the feed we're currently reading
var current_item_id;	// The ID of the item currently displayed

var index_page = {
};

var feed_page = {
};

var article_page = {
}

function init()
{
	cm = new CacheManager;

	/* Get the list of available pages */
	pages = document.getElementById("page-list").
		getElementsByClassName("page");

	/* Initialize the index page */
	index_page.feed_box = document.getElementById("feeds");

	/* XXX - Initialize the feed page */
	/* XXX - Initialize the article page */
	var page = find_page("article-page");
	article_page.h1_box = page.getElementsByTagName("h1")[0];
	article_page.author_box = document.getElementById("author-name");
	article_page.pubdate_box = document.getElementById("article-pubdate");
	article_page.lastupdate_box = document.getElementById("article-lastupdate");
	article_page.summary_box = document.getElementById("article-summary");
	article_page.content_box = document.getElementById("article-content");

	// XXX - Perhaps each page should be an object with its own
	// initialization function and whatnot?

	flip_to_page("index-page");
			// Show the main page

	index_page.feed_box.innerHTML = "<p>Loading feeds&hellip;</p>";

	display_feeds(null);
	cm.get_feeds(display_feeds);

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

function display_feeds(feeds)
{
	/* Make a UL of feeds, and add it to index_page.feed_box. */
	var str = "<ol class=\"feed-list\">";

	if (feeds == undefined || feeds == null)
		feeds = cm.feeds;

	if (feeds.length == 0)
	{
		// XXX - Ought to do something smart.

		// XXX - There are several ways feeds could be the
		// empty list: 1) we're just starting up, with no
		// cached feeds, and waiting for data from the server.
		// 2) The user hasn't subscribed to any feeds.
		// Distinguish between these two cases and display an
		// appropriate message.
		index_page.feed_box.innerHTML = "<p>Waiting for feeds to show up</p>";
		return;
	}

	// Sort feeds by title
	// XXX - Ought to be smarter: "The Foo" and "A Foo" get sorted
	// under T and A, respectively, instead of under F.
	feeds.sort(function(a,b) {
			if (a.title == b.title) return 0;
			if (a.title <  b.title) return -1;
			return 1;
		});

	for (var i in feeds)
	{
		var f = feeds[i];

		// Ignore the inactive feeds
		if (!f.active)
			continue;
		str += feed_entry_tmpl.expand(f);
	}
	str += "</ol>";
	index_page.feed_box.innerHTML = str;
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

	feed = cm.feeds[id];
	// XXX - Is it safe to assume that 'feed' is set?

	current_feed_id = id;
	current_item_id = null;

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
	var art_list_box = document.getElementById("articles");

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
	show_items(null);
	cm.get_items(current_feed_id, show_items);

	// XXX - Get latest articles from the server
}

function show_items(items)
{
	// XXX - Find the first item in this feed
	var feed_items = cm.items;

	// XXX - Sort feed_items by newest-first.
	// XXX - Delete read items, if necessary.

	// XXX - Fill in its contents in the feed page.
	var thelist = '<ol class="item-list">';
	for (var i in feed_items)
	{
		var item = feed_items[i];

		// XXX - This next bit is a hack. Clean it up
		if (item.feed_id != current_feed_id)
			continue;
		if (item.summary == undefined)
		{
			item = JSON.parse(localStorage["item/"+item.id]);
		}

		// Make sure the item has a title that can be clicked
		if (item.title == null ||
		    item.title.match(/^\s*$/))
			item.title = "[no title]";
		thelist += item_entry_tmpl.expand(item);
	}
	thelist += "</ul>";

	// XXX - This should be cached in feed_page.
	var art_list_box = document.getElementById("articles");
	art_list_box.innerHTML = thelist;
}

function new_items_callback(feed_id, items)
{
//debug("New items have arrived in feed "+feed_id);
//for (var i in items)
//{
//debug("New item: "+items[i].title);
//}
}

function show_item(id)
{
//debug("Inside show_item("+id+")");
	var item = JSON.parse(localStorage["item/"+id]);
		// XXX - Probably shouldn't suck stuff out of localStorage
		// quite so directly.

	// XXX - Initialize the feed page stuff

	article_page.h1_box.innerHTML = item.title;
	article_page.author_box.innerHTML = item.author;
	article_page.pubdate_box.innerHTML = item.pub_date;
	article_page.lastupdate_box.innerHTML = item.last_update;
	if (item.summary == "")
		article_page.summary_box.innerHTML = "No summary";
	else
		article_page.summary_box.innerHTML = item.summary;
	if (item.content == "")
		article_page.content_box.innerHTML = "No content";
	else
		article_page.content_box.innerHTML = item.content;

	flip_to_page("article-page");
}

function foo()
{
	var items = cm.get_items(current_feed_id,
					   null);
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
