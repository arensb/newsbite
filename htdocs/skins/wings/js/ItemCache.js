#ifndef _ItemCache_js_
#define _ItemCache_js_
#include "debug.js"
/* ItemCache.js
 * Module for manipulating local storage.
 */
#include "xhr.js"
/* XXX - Things this module's API needs:
 * - Periodically refresh the cache from the server, either
 *	- as a worker thread (desktop browsers)
 *	- as part of the main thread (mobile Safari)
 * - Get/set set of known feeds
 * ? Get a feed by ID
 * - Get/set an item by ID
 * - Get the N most recent items in feed M/all.
 * - Get the N items in feed M/all that are older/younger than item J.
 * - Send an event when:
 *	- A new item in feed F appears.
 *	- Status of item J has been updated.
 *	- Status of feed F has changed.
 * - Maintenance:
 *	- Remove items from nonexistent feeds
 * - Expire old items from cache
 * - Add/remove listeners for interesting changes
 *   i.e., "let me know when feed F changes", "let me know if item J
 *   changes".
 * - Mark an item as read/unread.
 *   Need status to see whether the change went through.
 * - Synchronize time with server?
 * - Flush any pending changes that couldn't go through because the
 *   browser was offline (or the server was unreachable, or whatever).
 */
/* XXX - Set a prefix for keys, in case we need to have two storage
 * databases?
 */
/* XXX - Store transitory data in sessionStorage, and more permanent
 * data in localStorage?
 */
/* XXX - Naming convention for functions, to keep things straight.
 * get_X()	read X from local storage
 * fetch_X()	get updated X from server
 */
/* XXX - In normal (non-SQLite) localStorage, keys can only be strings.
 * How should data be subdivided?
 * - item, id 123	"item/123"
 */
/* XXX - See
 * http://hacks.mozilla.org/2009/06/localstorage/
 * for un/serializing using native JSON.
 * JSON.stringify(somevalue)
 * JSON.parse(json_string)
 *
 * Apparently defined everywhere, including mobile Safari.
 */
/* XXX - Ideally, should know how to use SQLite.
 * How to tell if this is available?
 * Apparently only in Mozilla:
 * See https://developer.mozilla.org/en/Storage
 */
/* XXX - Mozilla has globalStorage, which is apparently the same as
 * localStorage, except that http://foo.tld and https://foo.tld can both
 * access the same storage.
 */
/* XXX - Split this up into front end and back end. Back end
 * communicates with the server, and issues events (or calls
 * callbacks) when things happen.
 * Front-end is user API: allows user to get and set data.
 * Basically, front end is the user's interface to the data on the
 * server. Back end acts as man-in-the-middle, caching data as it
 * comes in.
 */
/* XXX - Possible API:
 *	get_foo(key, callback);
 * Called by the client, this specifies to get the foo-type item with
 * key 'key'. Return what's in localStorage right now, but also set up
 * a request to fetch the most recent version from the server, and
 * call the callback function when the data comes in.
 */
/* XXX - What happens when storage exceeds its quota (5Mb in many
 * cases)? Does setItem throw an exception?
 * From http://dev.w3.org/html5/webstorage/:
 *
 * "If it couldn't set the new value, the method must raise an
 * QUOTA_EXCEEDED_ERR exception. (Setting could fail if, e.g., the
 * user has disabled storage for the site, or if the quota has been
 * exceeded.)
 *
 * No obvious way to tell what the quota is.
 * XXX - Should there be a function to measure its size?
 *
 * Exception... "Persistent storage maximum size reached" code: "1014" nsresult: "0x805303f6 (NS_ERROR_DOM_QUOTA_REACHED)" location: "http://www.ooblick.com/newsbite/foo.html Line: 23"
 *
 * Filling up storage to measure its size takes forever. Perhaps a
 * better approach might be to try to measure it on the fly: store
 * quota_min, quota_max, which give a range of possible sizes for the
 * quota. If setItem(key, value) fails, we know that the quota is
 * smaller than tot_storage+len(key)+len(value)+k. If it succeeds, we
 * know that the quota is >= tot_storage+len(key)+len(value)+k. k is
 * some additional factor, and might involve the number of keys,
 * bookkeeping data, etc.
 *
 * For now, perhaps just have a loop: remove the oldest item from the
 * cache until setItem() succeeds.
 */
/* XXX - Store various items like which feed/item was the user last
 * looking at? Whether feed details etc. should be displayed?
 */
/* XXX - Flushing cache: need to flush cache when it gets too big,
 * i.e., when we want to store an N-byte object, and there are <N
 * bytes left.
 */
/* XXX - All of these things are just instances of "get an item with a
 * given key from localStorage, and cache it on the way".
 * Just write an object for this.
 */
/* XXX - Perhaps rename this to StorageManager. */

/* ItemCache()
 * A do-nothing constructor.
 */
function ItemCache()
{
	// XXX
}

/* ItemCache
 * Class methods.
 */
ItemCache = {
	feeds: undefined,	// Known feeds
	items: {},		// Item index
	foo: function()
	{
		alert("Hello, world!");
	},

	/* ItemCache.scan_cache
	 * See what's in the local storage and initialize any
	 * necessary variables. Useful when starting up with a
	 * nonempty cache.
	 */

	scan_cache: function()
	{
		for (var i = 0; i < localStorage.length; i++)
		{
			var key = localStorage.key(i);
			var value = localStorage[key];
			var matches;

			if (key == "feeds")
			{
				feeds = JSON.parse(value);
				continue;
			}
			if (matches = key.match(/^item\/(\d+)$/))
			{
				// "item/12345"
				// It's an article, whose ID is 12345.

				// XXX - Instead of fetching the entire
				// article from local storage, ought to parse
				// it and stash the important bits in an
				// index.

				items[parseInt(matches[1])] = value;

				continue;
			}
			// XXX - If we get this far, 
		}
	},

	get_feeds: function(callback)
	{
		var retval = localStorage.feeds;
			// Get the version currently in storage

		if (callback)
		{
			// Fetch an updated version; call the callback
			// function when it's ready
			get_json_data("feeds.php",
				      {"o": "json"},
				      function(value)
				      {
					      localStorage.feeds = value;
					      callback(value);
				      },
				      true);
		}

		return retval;
	},

	/* fetch_feeds
	 * Fetch latest list of feeds from the server, and save them
	 * to local storage.
	 */
	fetch_feeds: function()
	{
		// XXX - Use get_json_data(). That'll allow us to get rid
		// of fetch_feeds_callback().
		var request = createXMLHttpRequest();

		if (!request)
			return false;

		request.open('GET',
			     "feeds.php?o=json",
			     true);
		request.onreadystatechange =
			function(){ ItemCache.fetch_feeds_callback(request) };
		request.send(null);
	},

	fetch_feeds_callback: function(req)
	{
		if (req.readyState != 4)
			return;

		localStorage.setItem("feeds", req.responseText);
	},
};

#endif	// _ItemCache_js_
