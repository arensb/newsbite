/* CacheManager
 * This basically acts as a proxy between the database on the server,
 * and client.
 */
#ifndef _CacheManager_js_
#define _CacheManager_js_
/* XXX - When localStorage fills up, setItem() throws an exception:
 * "Exception... "Persistent storage maximum size reached"  code: "1014"
 * nsresult: "0x805303f6 (NS_ERROR_DOM_QUOTA_REQCHED)"  location:
 * #"http://.../foo.js Line: NN"
 *
 * Perhaps write wrappers around localStorage.setItem() and
 * localStorage.getItem(), which keep track of when each item was
 * added/modified/accessed. The setItem() wrapper can then catch quota
 * exceptions, and delete items as necessary.
 */
/* XXX - Add functions to mark items as read/unread.
 */
#include "types.js"		/* Feed and Item classes */
#include "xhr.js"		// For AJAX requests

/* CacheManager.js
 * Manage local storage.
 */
/* XXX - Need some kind of consistency check a la fsck.
 * Store a value indicating whether local storage is clean or not,
 * in case a non-atomic operation gets interrupted.
 * - Are there any items in nonexistent feeds?
 */

/* XXX - Interface:
 * - feeds() - Return the feeds structure currently in storage.
 *
 * - update_feeds(cb) - Send an AJAX request to get feed information,
 * and call cb(new_feeds) when it completes.
 *
 * items(feed_id, ptr, before, after) - Return some items currently in
 * memory.
 *	feed_id - feed ID, or null for all.
 *	ptr - item ID, or null for first available item, or -1 for
 *	      last available item.
 *	before - return this many items before 'ptr'
 *	after - return this many items after 'ptr'
 * Notionally, we have in localStorage a list of items, ordered from
 * most recent to least recent pub_time. get_items() allows us to get
 * a slice of that, centered on the item with ID 'ptr'. Returns 'ptr',
 * up to 'before' items before 'ptr', and up to 'after' items after
 * 'ptr'. If ptr is null, then 'before' is ignored; if ptr is -1, then
 * 'after' is ignored.
 *
 * - update_items(feed_id, ?, cb) - Send an AJAX request to get more items
 * from feed_id, then call cb() when it completes.
 */
/* XXX - Perhaps have callback functions called when feeds/items get
 * updated? For when things change in the background. Or would that be
 * a bad idea?
 */

function CacheManager()
{
	this.headers = [];	// Metadata for all cached items,
				// sorted by pub_date.
	this.itemindex = {};	// All cached items, indexed by ID.

	/* Scan localStorage for stuff saved since last time.
	 */
	// XXX - How does this affect execution speed?
	for (var i = 0, n = localStorage.length; i < n; i++)
	{
		var key = localStorage.key(i);
		var matches;

		// The '(...) != null' construct is only there to stop
		// Firefox from issuing a warning about whether I
		// meant to use == instead of =.
		if ((matches = key.match(/^item:(\d+)$/)) != null)
		{
			// XXX - Wrap this in a try{}.
			var item = new Item(JSON.parse(localStorage.getItem(key)));
			// Extract header info
			var header = {};
			header.id = item.id;
			header.feed_id = item.feed_id;
			header.pub_date = item.pub_date;

			// Store the header info.
			this.headers.push(header);
			this.itemindex[item.id] = item;
		}
		// XXX - Check for unrecognized entries?
	}

	// Sort headers by last_update, just like lib/database.inc.
	this.headers.sort(function(a, b) {
			if (a.last_update > b.last_update)
				return -1;
			else if (a.last_update < b.last_update)
				return 1;
			else
				return b.id - a.id;
		});
}

/* XXX - localStorage wrappers:
 * - length() ?
 * - key(i) ?
 * x getItem(key)
 * x setItem(key, value)
 * x removeItem(key)
 * - touch(key) ? - Just update atime/mtime/ctime
 */

/* getItem
 * Wrapper around localStorage.getItem(): retrieve and parse the
 * requested item. Update access time.
 */
CacheManager.prototype.getItem = function(key)
{
	var retval;
	var str = localStorage.getItem(key);

	if (str == null)
		return null;
	try {
		retval = JSON.parse(str);
	} catch (e) {
		// The item doesn't parse, or something. Delete it.
		this.removeItem(key);
			// XXX - Remove from the array with key
			// information. Or perhaps just use
			// this.removeKey()?
		return null;
	}

	// XXX - Update access time for key.

	return retval;
}

/* setItem
 * Wrapper around localStorage.setItem(): store the key=>value pair,
 * and update its mtime.
 */
CacheManager.prototype.setItem = function(key, value)
{
	var str = JSON.stringify(value);
		// XXX - Error-checking?

	try {
		localStorage.setItem(key, str);
	} catch (e) {
		// XXX - Do something intelligent: if we're out of
		// quota, free up some space by deleting old cruft.
	}
}

/* removeitem
 * Wrapper around localStorage.removeItem(): remove the item from both
 * localStorage and from the meta information list.
 */
CacheManager.prototype.removeItem = function(key)
{
	localStorage.removeItem(key);
	// XXX - Remove from meta
}

/* feeds
 * Retrieve stored list of feeds.
 */
CacheManager.prototype.feeds = function()
{
	var a = this.getItem('feeds');
			// Array of strings, rather than the array of
			// Feeds that we'll eventually return.
	// Convert elements to objects.
	var retval = new Array();
	for (var f in a)
		retval.push(new Feed(a[f]));
	return retval;
}

/* update_feeds
 * Send an AJAX request to update feed information. If 'counts' is true,
 * request counts of read/unread items (which is slow).
 * Once the request completes, call cb(items), where 'items' is the
 * new array of feeds.
 */
CacheManager.prototype.update_feeds = function(counts, cb)
{
	var ajax_args = {
		o:	"json",
		};
	if (counts)
		ajax_args['s'] = "true";

	var me = this;
	get_json_data("feeds.php",
		      ajax_args,
		      function(value) {
			      me._update_feeds_cb(value, cb);
		      },
		      true);
}

/* _update_feeds_cb
 * Callback for update_feeds. Parse the returned string into an array
 * of Feed objects, store it in localStorage, and call the user
 * callback.
 */
CacheManager.prototype._update_feeds_cb = function(value, user_cb)
{
	// XXX - Ought to update existing feed info, rather than just
	// replace what's there. In particular, if 'value' doesn't
	// contain the read/unread counts, ought to keep the old
	// value.
	var newfeeds = new Array();
	for (var i in value)
		newfeeds[i] = new Feed(value[i]);

	this.store_feeds(newfeeds);	// Save a copy of the feed info

	user_cb(newfeeds);
}

/* store_feeds
 * Save list of feeds to localStorage.
 */
CacheManager.prototype.store_feeds = function(feeds)
{
	this.setItem('feeds', feeds);
}

/* get_item
 * Retrieve an article (by id)
 */
CacheManager.prototype.get_item = function(id)
{
	var tmp = this.getItem("item:"+id);
	if (tmp == null)
		return null;
	return new Item(tmp);
}

/* getitems
 * Get some items from the given feed
 */
// XXX - Ought to be able to specify more details.
CacheManager.prototype.getitems = function(feed_id)
{
	var retval = new Array();
	for (var i = 0, l = this.headers.length; i < l; i++)
	{
		var head = this.headers[i];
		if (feed_id != "all" && head.feed_id != feed_id)
			continue;
		retval.push(this.get_item(head.id));
	}

	return retval;
}

/* update_items
 * Get items from the server.
 * feed_id - ID of the feed to update, or "all".
 * start - Start offset
 * cb - callback function to call when the update is complete.
 */
CacheManager.prototype.update_items = function(feed_id, start, cb)
{
	var ajax_args = {
		o:	"json",
		id:	feed_id,
		s:	start + 0,
		};

	var me = this;
	get_json_data("items.php",
		      ajax_args,
		      function(value) {
			      me._update_items_cb(value, cb);
		      },
		      true);
}

CacheManager.prototype._update_items_cb = function(value, user_cb)
{
	var newitems = new Array();
	for (var i in value.items)
	{
		var item = new Item(value.items[i]);
		newitems.push(item);
		this.store_item(item);
	}

	user_cb(newitems);
}

// XXX - Save an article
CacheManager.prototype.store_item = function(item)
{
	this.setItem('item:'+item.id, item);
}

#endif	// _CacheManager_js_
