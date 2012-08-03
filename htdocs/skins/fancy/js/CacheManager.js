/* CacheManager
 * This basically acts as a proxy between the database on the server,
 * and client.
 */
#ifndef _CacheManager_js_
#define _CacheManager_js_
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
	this.headers = [];	// Metadata for all cached articles,
				// sorted by pub_date.
	this.itemindex = {};	// Metadata for all cached articles,
				// indexed by ID.
	this._ls_index = {};	// Metainformation about the data in
				// localStorage.
			// XXX - Keep track of size?
	this.last_sync = undefined;
// XXX - Is this.last_sync used? - Yes. Ought to remember it the same
// way as last_whatsread.
				// Time of last update fetched through
				// "updates.php".
	this.last_whatsread = new Date(localStorage.getItem("last_whatsread"));
				// Time of last update fetched through
				// "whatsread.php".
	if (isNaN(this.last_whatsread))
		this.last_whatsread = new Date();

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
		    key == "onscreen" ||
		    key == "last_whatsread")	// Last 'whatsread.php' time.
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

	/* Delete the articles marked read */
	for (var key in todelete)
		localStorage.removeItem(todelete[key]);

	return this;
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
		      function(status, msg) {	// Error handler
			      msg_add("JSON failed: "+status+": "+msg);
		      },
		      true);
}

/* _update_feeds_cb
 * Callback for update_feeds. Parse the returned string into an array
 * of Feed objects, store it in localStorage, and call the user
 * callback.
 */
// XXX - Move this inside update_feeds()? Will 'this' be set correctly?
CacheManager.prototype._update_feeds_cb = function(value, user_cb)
{
	// XXX - Ought to update existing feed info, rather than just
	// replace what's there. In particular, if 'value' doesn't
	// contain the read/unread counts, ought to keep the old
	// value.
	var newfeeds = {};
	for (var i in value)
		newfeeds[i] = new Feed(value[i]);

	this.store_feeds(newfeeds);	// Save a copy of the feed info

	/* XXX - If the user has unsubscribed from some feed, we may
	 * have cached items from no-longer-existing feeds. Ought to
	 * go through the cache and delete them.
	 * Best not to do it in this function, though, since the
	 * browser user is waiting for stuff to happen. Rather, ought
	 * to wait a bit and do maintenance while other stuff is going
	 * on.
	 */

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
// XXX - Document the arguments
// XXX - Ought to be able to specify more details.
CacheManager.prototype.getitems = function(feed_id, cur, before, after)
{
//console.debug("CacheManager.getitems("+feed_id+", "+cur+", "+before+", "+after+")");
//if (cur != null)
//console.debug("cur.last_update: "+cur.last_update);
	var retval = new Array();

	// XXX - Get the unread articles from whichever feed we're
	// reading.
	// Sort them. Find the spot, then return the surrounding
	// articles.

	/* Sort headers by last_update, just like lib/database.inc. */
	var hdrs = [];
//msg_add("this.headers.length: "+this.headers.length);
	for (var i = 0, l = this.headers.length; i < l; i++)
	{
		var h = this.headers[i];
		if (h.is_read)
			continue;
		if (feed_id != "all" && h.feed_id != feed_id)
			continue;
		hdrs.push(h);
	}

	/* XXX - Why does this function often complain of
	 * "reference to undefined property a.last_update" in FF 13?
	 * The obvious answer is that 'a' has id, feed_id, pub_date, is_read,
	 * and mtime, but not last_update. But why?
	 if (a['last_update'] == undefined)
	 console.debug(a);
	 if (b['last_update'] == undefined)
	 console.debug(b);
	*/
	hdrs.sort(function(a, b) {
			// XXX - Does this function do what I want?
			// Want to sort by last_update, from newest to
			// youngest; use id (larger id goes first) as
			// a tiebreaker.
			if (a.last_update > b.last_update)
				return -1;
			else if (a.last_update < b.last_update)
				return 1;
			else
				return b.id - a.id;
		});
	var hlen = hdrs.length;
//msg_add("hlen: "+hlen);

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
		      function(status, msg) {	// Error handler
			      msg_add("JSON failed: "+status+": "+msg);
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
	// XXX - Test this.
	this.removeItem("item:"+item_id);

	for (var i in this.headers)
	{
		var h = this.headers[i];
		if (h.id == item_id)
			this.headers.splice(i,1);
	}
	delete this.itemindex[item_id];
}

/* get_updates
 * Find the oldest mtime in the cache, get updates since that time, and
 * apply them to the cache
 */
CacheManager.prototype.get_updates = function(feed_id, cb)
{
	// Get the latest mtime we have
//	var latest_mtime = this.last_sync;
	var latest_mtime;

	// Need to have a separate latest_mtime for each feed ID.
	if (this.getItem("latest_mtime:"+feed_id) != null)
		latest_mtime = new Date(this.getItem("latest_mtime:"+feed_id));
//console.debug("first latest_mtime: "+latest_mtime);

	// If we already have a latest_mtime, use it as is, so we get
	// all the updates that happened since then.

	// Otherwise, assume that the page just loaded, so get the
	// most recent mtime of all the articles.
	if (latest_mtime == undefined)
	{
		latest_mtime = new Date(0);

		for (var i = 0, n = this.headers.length; i < n; i++)
		{
			var hdr = this.headers[i];
			var mtime;

			if (hdr.mtime != null)
				mtime = hdr.mtime;
			else
				mtime = hdr.last_update;
			if (mtime > latest_mtime)
				latest_mtime = mtime;
		}
	}

	// Place AJAX call to
	//	updates.php?
	//		id=feed_id
	//		o=json
	//		t=<newest-mtime>
	var me = this;	// Trick so that we can call _get_updates_cb
			// as a method, not a regular function.
	get_json_data("updates.php",
		      { o:	"json",
			id:	feed_id,
			t:	Math.floor(latest_mtime.valueOf()/1000),
		      },
		      function(value) {
			      me._get_updates_cb(feed_id, value, cb);
		      },
		      null,
		      true);
}

/* _get_updates_cb
 * Callback function for get_updates(): receive a bunch of updated
 * posts, and do something smart with them.
 */
// XXX - Move this inside get_updates()?
CacheManager.prototype._get_updates_cb = function(feed_id, value, user_cb)
{
var num_read = 0;
var num_new = 0;
console.debug("inside _get_updates_cb: feed_id == "+feed_id);

	if (value == null ||
	    value.updates == null ||
	    value.updates.length == 0)
	{
		// No updates.
console.debug("no updates");
	} else {
		var updates = value['updates'];
		var num_updates = value['num_updates'];
msg_add("num_updates: "+num_updates);
		var latest_mtime = new Date(0);
			// Remember the most recent update, so we don't
			// request things over and over: otherwise,
			// get_updates() can return the same read articles
			// over and over.

		/* Process updated items from updates.php */
		for (var i = 0, n = updates.length; i < n; i++)
		{
			var item = new Item(updates[i]);

			if (item.is_read)
			{
				// Got an item marked read on the
				// server. Remove it from cache.

				/* XXX - Not so fast: ought to check
				 * the mtime on the server and the
				 * mtime in our cache, and keep the
				 * most recent one.
				 */
num_read++;
				this.purge_item(item.id);
			} else {
				// Got an unread item. Update it in
				// cache.
num_new++;
				this.store_item(item);
			}

			if (item.mtime > latest_mtime)
				// Remember the most recent update
				latest_mtime = item.mtime;
		}

		this.last_sync = latest_mtime;	// Remember for next time.
console.debug("saving latest_mtime: "+this.last_sync.valueOf()+": "+this.last_sync);
		this.setItem("latest_mtime:"+feed_id, this.last_sync.valueOf());
msg_add(updates.length+" updates @ "+this.last_sync+": "+num_read+"/"+num_new+", "+num_updates+" tot");
	}

	/* Call user callback, if requested */
	// XXX - What arguments should it take?
	if (user_cb != null)
		user_cb();
}

/* get_marked
 * Find the oldest mtime in the cache, get items that have been marked
 * as read since then, and purge those from the cache.
 */
CacheManager.prototype.get_marked = function(feed_id, cb)
{
	// Get the latest mtime we have
//	var last_whatsread = new Date(localStorage.getItem("last_whatsread"));
	var last_whatsread;
				// Time of last update fetched through
				// "whatsread.php".
	if (this.getItem("last_whatsread") != null)
		last_whatsread = new Date(this.getItem("last_whatsread"));
console.debug("first last_whatsread: "+last_whatsread);

	// If we already have a last_whatsread, use it as is, so we get
	// all the updates that happened since then.

	// Otherwise, assume that the page just loaded, so get the
	// most recent mtime of all the articles.

	// XXX - I don't think this approach works. We're likely to
	// overlook a bunch of stuff.
	if (isNaN(last_whatsread))
	{
		last_whatsread = new Date();
console.debug("init last_whatsread: "+last_whatsread)

		for (var i = 0, n = this.headers.length; i < n; i++)
		{
			var hdr = this.headers[i];
			var mtime;

			if (hdr.mtime != null)
				mtime = hdr.mtime;
			else
				mtime = hdr.last_update;
//console.debug("mtime: "+mtime);
			if (mtime < last_whatsread)
{
				last_whatsread = mtime;
console.debug("new last_whatsread: "+last_whatsread)
}
		}
	}

	// Place AJAX call to
	//	whatsread.php?
	//		o=json
	//		t=<newest-mtime>
	var me = this;	// Trick so that we can call _get_marked_cb
			// as a method, not a regular function.
//msg_add("last_whatsread: "+Math.floor(last_whatsread.valueOf()/1000));
//msg_add("last_whatsread: "+last_whatsread.valueOf());
msg_add("last_whatsread: "+last_whatsread);
	get_json_data("whatsread.php",
		      { o:	"json",
			t:	Math.floor(last_whatsread.valueOf()/1000),
		      },
		      function(value) {
			      me._get_marked_cb(value, cb);
		      },
		      function(status, msg) {
			      msg_add("Error getting marked items: "+
				      status+": "+msg);
		      },
		      true);
}

/* _get_marked_cb
 * Callback function for get_marked(): receive a bunch of records for
 * items that have been marked as read, and purge them from cache.
 */
// XXX - Move this inside get_marked()?
CacheManager.prototype._get_marked_cb = function(value, user_cb)
{
var num_read = 0;

console.debug("In _get_marked_cb:");
//console.debug(value);
	if (value == null ||
	    value.updates == null ||
	    value.updates.length == 0)
	{
		// No updates.
console.debug("no updates");
	} else {
		var updates = value['updates'];
		var num_updates = value['num_updates'];
console.debug("num_updates: "+num_updates);
		var latest_mtime = new Date(0);

		/* Process updated items from updates.php */
		for (var i = 0, n = updates.length; i < n; i++)
		{
			var item = new Item(updates[i]);

			// Got an item marked read on the server.
			// Remove it from cache.

			/* XXX - Not so fast: ought to check the mtime
			 * on the server and the mtime in our cache,
			 * and keep the most recent one.
			 */
num_read++;
			this.purge_item(item.id);

			if (item.mtime > latest_mtime)
				// Remember the most recent update, so
				// we don't request things over and
				// over: otherwise, get_marked() can
				// return the same read articles over
				// and over.
				latest_mtime = item.mtime;
		}

		this.last_whatsread = latest_mtime;
console.debug("saving latest_mtime: "+this.last_whatsread.valueOf()+": "+this.last_whatsread);
		this.setItem("last_whatsread", this.last_whatsread.valueOf());
				// Stash for next time.
msg_add(value.length+" read @ "+this.last_whatsread+": "+num_read+", "+num_updates+" tot");
	}

	/* Call user callback, if requested */
	// XXX - What arguments should it take?
	if (user_cb != null)
		user_cb();
}

CacheManager.prototype.slow_sync = function(feed_id, user_cb, user_err_cb)
{
	var me = this;		// 'this' for child functions.

	// get_json_data callback when things go well
	function slow_sync_cb(value)
	{
//console.debug("In slow_sync_cb, me == "+me);
		// XXX
//console.debug("slow_sync_cb: got");
//console.debug(value);
		// XXX - Sanity checking for value: make sure it's an
		// array, of length > 0.

		// XXX - Delete nonexistent and read items
		//	Get inspiration from _get_marked_cb()
		for (var i in value)
		{
			var entry = value[i];

//console.debug(entry);
			if ('action' in entry &&
			    entry['action'] == "delete")
					// Use subscript notation
					// because 'action' might not
					// exist.
			{
				// This item doesn't exist in the
				// database. Remove it from cache.
console.debug("Deleted item "+entry.id);
				me.purge_item(entry.id);
				continue;
			}

			if (entry.is_read)
			{
				// This item is read. Remove from cache.
console.debug("Marked read item "+entry.id);
				me.purge_item(entry.id);
				continue;
			}

			// This is a new item. Add it to cache.
			try {
			var item = new Item(entry);
			me.store_item(item);
console.debug("Added new item "+item.id+": "+item.title);
//console.debug(localStorage["item:"+item.id]);
			continue;
			} catch(e) {
				console.error("Can't add item: "+e);
			}

			// XXX - What's left?
console.debug("What should I do with this?:");
console.debug(entry);
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

	/* Get list of items in cache */
	var tosend = {};
	for (var id in this.itemindex)
	{
		/* Compose list of {id, mtime, is_read} entries to send */
		var header = this.itemindex[id];
//console.debug("id "+header.id+", is_read "+header.is_read+", mtime "+header.mtime);
		tosend[header.id] = {
			     is_read:	header.is_read,
			     mtime:	header.mtime,
			};
	}

	/* Send to server */
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
