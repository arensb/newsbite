/* CacheManager
 * This basically acts as a proxy between the database on the server,
 * and client.
 */
#ifndef _CacheManager_js_
#define _CacheManager_js_
/* XXX - Add functions to mark items as read/unread.
 */
#include "types.js"		// Feed and Item classes
#include "xhr.js"		// For AJAX requests
#include "defer.js"		// Put off long initialization until later

/* CacheManager.js
 * Manage local storage.
 */
/* XXX - Need some kind of consistency check a la fsck.
 * Store a value indicating whether local storage is clean or not,
 * in case a non-atomic operation gets interrupted.
 * - Are there any items in nonexistent feeds?
 */

/* XXX - Interface:
 * x feeds() - Return the feeds structure currently in storage.
 *
 * - update_feeds(cb) - Send an AJAX request to get feed information,
 * and call cb(new_feeds) when it completes.
 *
 * items(feed_id, ptr, before, after) - Return some items currently in
 * memory.
 *	feed_id - feed ID, or null for all.
 *	ptr - item ID, or null for first available item, or -1 for
 *	      last available item.
 *	      Should be a struct describing the current item: ID, date,
 *	      etc. so we can find the "current" item if that particular
 *	      one has been deleted.
 *	before - return this many items before 'ptr'
 *	after - return this many items after 'ptr'
 * Notionally, we have in localStorage a list of items, ordered from
 * most recent to least recent pub_time. get_items() allows us to get
 * a slice of that, centered on the item with ID 'ptr'. Returns 'ptr',
 * up to 'before' items before 'ptr', and up to 'after' items after
 * 'ptr'. If ptr is null, then 'before' is ignored; if ptr is -1, then
 * 'after' is ignored.
 */
/* XXX - Perhaps have callback functions called when feeds/items get
 * updated? For when things change in the background. Or would that be
 * a bad idea?
 */

function CacheManager()
{
	this.headers = [];	// Metadata for all cached articles,
				// sorted by pub_date.
	this.itemindex = {};	// Metadata for all cached articles,
				// indexed by ID.
	this._ls_index = {};	// Metainformation about the data in
				// localStorage.
			// XXX - Keep track of size?
	this.last_sync = undefined;
// XXX - Is this.last_sync used? - Yes. Ought to remember it
				// Time of last update fetched through
				// "updates.php".

	/* Scan localStorage for stuff saved since last time.
	 */
	// XXX - How does this affect execution speed?
	// XXX - Should _ls_index be saved across sessions?
	var todelete = [];	// Array of read articles to delete
	for (var i = 0, n = localStorage.length; i < n; i++)
	{
		var key = localStorage.key(i);
		var matches;

		if (key == "feeds" ||
		    key == "onscreen")
		{
			this._ls_index[key] = {
				"time":	new Date(),
			};
		} else if ((matches = key.match(/^item:(\d+)$/)) != null)
			// The '(...) != null' construct is only there
			// to stop Firefox from issuing a warning
			// about whether I meant to use == instead of
			// =.
		{
			// XXX - Wrap this in a try{}.
			var item = new Item(JSON.parse(localStorage.getItem(key)));

			/* Delete items marked as read */
			// XXX - Is this too low-level? At this stage,
			// is CacheManager just a low-level interface
			// to localStorage? Of course, if so, we've
			// kinda blown it by knowing about the Item
			// class.

			// XXX - It may be premature to delete read
			// items here: the initialization happens in
			// every window/tab, so deleting read items in
			// one tab can affect other tabs, which may
			// not be what we want. Then again, this whole
			// thing isn't all that tab-friendly.
			if (item.is_read)
			{
				todelete.push(key);
				continue;
			}

			this._ls_index[key] = {
				"time":	new Date(),
			};

			// Extract header info
			var header = {};
			header.id = item.id;
			header.feed_id = item.feed_id;
			header.pub_date = item.pub_date;
			header.mtime = item.mtime;
			header.last_update = item.last_update;
				// XXX - Which one do I want? pub_date, or
				// last_update? Gah! So confused!
			header.is_read = item.is_read;

			// Store the header info.
			this.headers.push(header);
			this.itemindex[item.id] = header;
		}

		/* Ignore unrecognized entries: they probably belong
		 * to some other app on the same machine+port.
		 */
	}

	/* Delete the articles marked read.
	 * Deleting takes a long time in Firefox, so we stick this
	 * part in a separate function that runs deferred. Hopefully
	 * this will make the app more responsive.
	 */
	_purge_stuff.defer(1, null, todelete);

	return this;
}

/* _purge_stuff
 * Takes a list of keys, and deletes entries with those keys from
 * localStorage, 100 at a time (because this takes a long time in
 * Firefox).
 */
// XXX - This probably ought to be a CacheManager method rather than a
// standalone top-level function.
function _purge_stuff(todelete)
{
	if (!(todelete instanceof Array))
		return;

	var i = 0;	// Number of items deleted in this run
	var key;
	while ((key = todelete.shift()) != null)
	{
		localStorage.removeItem(key);
		if (++i > 25)
		{
			// We've done enough for now. Give someone
			// else a chance to run.
			_purge_stuff.defer(1, null, todelete);
			return;
		}
	}
}

/* localStorage wrappers */

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
		console.error("Error parsing JSON: "+e);
		console.trace();
		this.removeItem(key);
		return null;
	}

	// Update access time for key.
	this._ls_index[key] = {
		"time":	new Date(),
		};

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
		// If we're out of quota, free up some space by
		// deleting old cruft.
		// XXX - Check to make sure the error is actually
		// "over quota".
		// According to
		// http://www.w3.org/TR/webstorage/#the-localstorage-attribute
		// this should be QuotaExceededError.
		console.log("Error in setItem: "+e);
		console.trace();
		this._ls_purge(key.length+str.length);
	}

	// Update timestamp on the item.
	this._ls_index[key] = {
		"time":	new Date(),
		};
}

/* removeitem
 * Wrapper around localStorage.removeItem(): remove the item from both
 * localStorage and from the meta information list.
 */
CacheManager.prototype.removeItem = function(key)
{
	localStorage.removeItem(key);

	// Remove from timestamp data.
	delete this._ls_index[key];
}

/* _ls_purge
 * Delete old cruft from localStorage. 'size' says how many bytes to
 * free up.
 */
// XXX - This function can delete things that other functions are
// using.
CacheManager.prototype._ls_purge = function(size)
{
msg_add("Garbage collection");
	var tmp = new Array;

	/* Make an array from _ls_index data */
	for (var i in this._ls_index)
	{
		tmp.push({
			"key":	i,
			"time":	this._ls_index[i].time,
			});
	}

	/* Sort the array by time */
	// XXX - Entries marked as read should be deleted before
	// unread ones. But this layer doesn't know about any of that.
	// Perhaps need to allow higher layers to indicate that this
	// is a "delete preferentially" type of entry.
	tmp.sort(function(a, b) {
		return a.time.getTime() - b.time.getTime()
		});

	/* Purge the oldest entries */
	/* XXX - This is rather stupid and inefficient: we're just
	 * taking the oldest 50% of the entries and purging that. It'd
	 * be smarter to free up 'size' bytes, perhaps plus some
	 * additional space for elbow room.
	 */
	for (var i = 0, len = Math.floor(tmp.length/2); i < len; i++)
	{
		// Purge the i'th element.
		this.removeItem(tmp[i].key);
	}
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
	var retval = {};
	for (var f in a)
		retval[f] = new Feed(a[f]);
	return retval;
}

/* update_feeds
 * Send an AJAX request to update feed information.
 * Once the request completes, call cb(items), where 'items' is the
 * new array of feeds.
 */
CacheManager.prototype.update_feeds = function(cb)
{
	/* Inner helper functions */
	function update_feeds_callback(value)
	{
//msg_add("get_json_data(feeds.php) returned.");
		// XXX - Ought to update existing feed info, rather
		// than just replace what's there. [[In particular, if
		// 'value' doesn't contain the read/unread counts,
		// ought to keep the old value.]] Is the [[]] part
		// still applicable?
		var newfeeds = {};
		for (var i in value)
		{
			newfeeds[i] = new Feed(value[i]);
		}

		/* XXX - If the user has unsubscribed from some feed, we may
		 * have cached items from no-longer-existing feeds. Ought to
		 * go through the cache and delete them.
		 * Best not to do it in this function, though, since the
		 * browser user is waiting for stuff to happen. Rather, ought
		 * to wait a bit and do maintenance while other stuff is going
		 * on.
		 */

		self.store_feeds(newfeeds);	// Save a copy of the feed info
				// XXX - This can be deferred. But if so,
				// we ought to do the cleanup described
				// above.

		if (cb != null)
			cb(newfeeds);
	}

	/* update_feeds() main */
	var self = this;	// Remember 'this' to pass to callback
				// function.
//msg_add("get_json_data(feeds.php)");
	get_json_data("feeds.php",
		      { },
		      update_feeds_callback,
		      function(status, msg) {	// Error handler
			      msg_add("feeds.php JSON failed: "+status+": "+msg);
		      },
		      true);
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
 *
 * feed_id: the ID of the feed for which to get items, or "all".
 * cur: an Item object, the reference item for 'before' and 'after'; or:
 *	null: get the first available items.
 *	-1: get the last available items.
 * before: integer. The number of items to return that come before 'cur',
 *	i.e., whose publication/modification time is more recent than
 *	'cur'.
 * after: integer. Like 'before', but items coming after 'cur', i.e. whose
 *	publication time is older than 'cur'.
 */
// XXX - Ought to be able to specify more details.
CacheManager.prototype.getitems = function(feed_id, cur, before, after)
{
	var retval = new Array();

	/* Sort headers by last_update, just like lib/database.inc. */
	var hdrs = [];
	for (var i = 0, l = this.headers.length; i < l; i++)
	{
		var h = this.headers[i];
		if (h.is_read)
			continue;
		if (feed_id != "all" && h.feed_id != feed_id)
			continue;
		hdrs.push(h);
	}

	// The sort function gets called a lot. Don't pessimize it.
	hdrs.sort(function(a, b) {
			if (a.last_update > b.last_update)
				return -1;
			else if (a.last_update < b.last_update)
				return 1;
			else
				return b.id - a.id;
		});
	var hlen = hdrs.length;

	if (hlen == 0)
		// No items cached
		return null;

	/* Find the entry corresponding to 'ptr', or the place where
	 * it would go if it existed.
	 */
	var ptr = null;	// Index of item corresponding to 'cur'.

	// Check for special values: null for first available, -1 for
	// last available item.
	if (cur == null)
		ptr = 0;
	else if (cur == -1)
		ptr = hlen-1;
	else {
		for (var i = 0; i < hlen; i++)
		{
			var item = hdrs[i];

			// Found the exact item we're looking for
			if (cur.id != null && item.id == cur.id)
			{
				ptr = i;
				break;
			}

			// Couldn't find the exact one. We'll settle for the
			// one after that (chronologically).
			if (item.last_update < cur.last_update)
			{
				if (i == 0)
					ptr = 0;
				else
					ptr = i - 1;
				break;
			}
		}
	}
	if (ptr == null)
		// Couldn't find anything. This might happen if 'cur'
		// refers to an article from last week, but all we
		// have in cache is articles from today. Give them the
		// oldest available article.
		ptr = hlen-1;

	var first = ptr - before;
	var last = ptr + after;

	if (first < 0)
		first = 0;
	if (last >= hlen)
		last = hlen-1 ;


	for (var i = first; i <= last; i++)
	{
		var head = hdrs[i];
		var el = this.get_item(head.id);
		if (el == null)
		{
			console.error("Can't get_item(%s)", head.id);
			continue;
		}
		retval.push(this.get_item(head.id));
	}

	return retval;
}

/* store_item
 * Save an article to cache.
 */
CacheManager.prototype.store_item = function(item)
{
	this.setItem('item:'+item.id, item);
			// Store in localStorage

	// Add to this.headers, this.itemindex
	var header = this.itemindex[item.id];
	if (header == null)
	{
		header = {};
		this.headers.push(header);
		this.itemindex[item.id] = header;
	}
	header.id = item.id;
	header.feed_id = item.feed_id;
	header.pub_date = item.pub_date;
	header.last_update = item.last_update;
	header.is_read = item.is_read;
	header.mtime = item.mtime;
}

/* purge_item
 * Delete an article from cache, by item_id.
 */
CacheManager.prototype.purge_item = function(item_id)
{
	this.removeItem("item:"+item_id);

	for (var i in this.headers)
	{
		var h = this.headers[i];
		if (h.id == item_id)
			this.headers.splice(i,1);
	}
	delete this.itemindex[item_id];
}

/* slow_sync
 * Send:
 *	{
 *	id: feed ID
 *	ihave: hash of items
 *	}
 * ihave[id] -> {is_read, mtime}
 * where is_read is a boolean saying whether the item has been marked
 * read, and mtime is a timestamp of when the item was updated.
 *
 * The server takes this list of items and decides what to do for each
 * one: normally, it just sets the "is_read" status to what's in the
 * call above. If the item was updated after 'mtime', then 'is_read'
 * is ignored, and the item stays as-is.
 *
 * Receive:
 *	[
 *	  {
 *		id: 12345,
 *		action: delete
 *	  },
 *	  {
 *		id: 23456,
 *		is_read: <bool>,
 *		mtime: <timestamp>,
 *	  },
 *	  {
 *		id: 34567,
 *		title: "...",
 *		summary: "...",
 *		...
 *	  },
 *	... ]
 *
 * The server returns a list of items that the client needs to update:
 * If "action" is set to "delete", the article no longer exists in the
 * database and should be deleted.
 *	If just "is_read" is set (and is true), then the article
 * has been marked read. It can be removed from cache.
 *	If it's a whole new item, then it's an item that has arrived
 * on the server, that the client doesn't have yet. It should be added
 * to local storage.
 */
CacheManager.prototype.slow_sync = function(feed_id, user_cb, user_err_cb)
{
	/* Inner helper functions */

	// get_json_data callback when things go well
	function slow_sync_cb(value)
	{
msg_add("sync.php returned ok");
		// XXX - Sanity checking for value: make sure it's an
		// array, of length > 0.

		for (var i in value)
		{
			var entry = value[i];

			if ('action' in entry &&
			    entry['action'] == "delete")
					// Use subscript notation
					// because 'action' might not
					// exist.
			{
				// This item doesn't exist in the
				// database. Remove it from cache.
				me.purge_item(entry.id);
				continue;
			}

			if (entry.is_read)
			{
				// This item is read. Remove from cache.
				me.purge_item(entry.id);
				continue;
			}

			// This is a new item. Add it to cache.
			try {
			var item = new Item(entry);
			me.store_item(item);
			continue;
			} catch(e) {
				console.error("Can't add item: %o", e);
				console.trace();
			}

			// XXX - What's left?
console.log("What should I do with this?:\n%o", entry);
		}

		// Call callback function
		if (typeof(user_cb) == "function")
			user_cb();
	}

	// get_json_data callback when there's an error
	function slow_sync_error(status, msg)
	{
		alert("slow_sync_error "+status+"\n"+msg);
		if (typeof(user_err_cb) == "function")
			user_err_cb(status, msg);
	}

	/* slow_sync() main */

	var me = this;		// 'this' for child functions.

	/* Get list of items in cache */
	var tosend = {};
	for (var id in this.itemindex)
	{
		/* Compose list of {id, mtime, is_read} entries to send */
		var header = this.itemindex[id];
		tosend[header.id] = {
			     is_read:	header.is_read,
			     mtime:	header.mtime,
			};
	}

	/* Send to server */
msg_add("get_json_data(sync.php)");
	get_json_data("sync.php",
		      {id: feed_id,
		       ihave: JSON.stringify(tosend),
		      },
		      slow_sync_cb,
		      slow_sync_error,
		      true);
	// Response will be collected by slow_sync_cb
}

#endif	// _CacheManager_js_
