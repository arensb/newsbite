/* CacheManager
 * This basically acts as a proxy between the database on the server,
 * and client.
 */
#ifndef _CacheManager_js_
#define _CacheManager_js_
/* XXX - Perhaps organize things as follows:
 * localStorage:
 * - feeds: basically a mirror of the 'feeds' table.
 * - item:NNN: the item with id NNN.
 */
/* XXX - When localStorage fills up, setItem() throws an exception:
 * "Exception... "Persistent storage maximum size reached"  code: "1014"
 * nsresult: "0x805303f6 (NS_ERROR_DOM_QUOTA_REQCHED)"  location:
#"http://.../foo.js Line: NN"
 */
#include "types.js"		/* Feed and Item classes */
#include "xhr.js"		// For AJAX requests

/* CacheManager.js
 * Manage local storage.
 */
/* XXX - Need some kind of consistency check a la fsck.
 * Store a value indicating whether local storage is clean or not,
 * in case a non-atomic operation gets interrupted.
 * - Does each ihead: entry have an ibody: and vice-versa?
 * - Are there any items in nonexistent feeds?
 */

/* XXX - Interface:
 * feeds() - Return the feeds structure currently in storage.
 *
 * update_feeds(cb) - Send an AJAX request to get feed information,
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
 * update_items(feed_id, ?, cb) - Send an AJAX request to get more items
 * from feed_id, then call cb() when it completes.
 */
/* XXX - Perhaps have callback functions called when feeds/items get
 * updated?
 */


function CacheManager()
{
	/* XXX - Load known objects? Or would that affect execution
	 * speed? Or perhaps just stick a flag in the prototype saying
	 * that this object hasn't been initialized properly yet?
	 */
	this.items = {};	// Header information about items
	this.have_text = {};	// 1 for those items for which we have text

	for (var i, n = localStorage.length; i < n; i++)
	{
		var key = localStorage.key(i);
		var matches;

		// The '(...) != null' construct is only there to stop
		// Firefox from issuing a warning about whether I
		// meant to use == instead of =.
		if ((matches = key.match(/^ihead:(\d+)$/)) != null)
		{
			// XXX - Wrap this in a try{}.
			this.items[match[1]] = JSON.parse(localStorage.getItem(key));
		} else if ((matches = key.match(/^ibody:(\d+)$/)) != null)
		{
			this.have_text[matches[1]] = 1;
		}
		// XXX - Check for unrecognized entries?
	}
}

/* feeds
 * Retrieve stored list of feeds.
 */
CacheManager.prototype.feeds = function()
{
	// Get the cached set of feeds from local storage
	var str;
	try {
		str = localStorage.getItem('feeds');
	} catch (e) {
		// Exception might be thrown if browser doesn't grok
		// localStorage.
		localStorage.removeItem('feeds');
		return null;
	}
	if (str == null)
		// No feeds stored yet
		return null;

	// Parse the retrieved value
	var a;		// Array of strings, rather than the array of
			// Feeds that we'll eventually return.
	try {
		a = JSON.parse(str);
	} catch (e) {
		// In case of syntax error or something
		localStorage.removeItem('feeds');
		return null;
	}

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
	var newfeeds = new Array();
	for (var i in value)
		newfeeds[i] = new Feed(value[i]);

	this.store_feeds(newfeeds);

	user_cb(newfeeds);
}

// XXX - Update saved data for one feed

// XXX - Get one feed (not all)

/* store_feeds
 * Save list of feeds to localStorage.
 */
CacheManager.prototype.store_feeds = function(feeds)
{
	var str = JSON.stringify(feeds);

	localStorage.setItem('feeds', str);
	// XXX - Error-checking. Make sure quota hasn't been exceeded.
}

// XXX - Retrieve feed metadata

// XXX - Save an article
CacheManager.prototype.store_item = function(item)
{
	/* Split item into body (summary and content, the two long
	 * fields) and head (everything else). Store them as
	 */
	var ihead = {};
	var ibody = {};

	for (var field in item)
	{
		if (!item.hasOwnProperty(field))
			// Don't iterate over inherited properties. See
			// http://stackoverflow.com/questions/5861763/how-to-tell-if-a-javascript-variable-is-a-function
			continue;
		if (typeof(item[field]) == "function")
			// Don't store functions
			continue;

		if (field == "summary" || field == "content")
			ibody[field] = item[field]
		else
			ihead[field] = item[field]
	}
	localStorage.setItem("ihead:"+item.id,
			     JSON.stringify(ihead));
	localStorage.setItem("ibody:"+item.id,
			     JSON.stringify(ibody));
}

/* get_item
 * Retrieve an article (by id)
 */
CacheManager.prototype.get_item = function(id)
{
	var ihead = localStorage.getItem("ihead:"+id);

	var ibody = localStorage.getItem("ibody:"+id);
	if (ihead == null || ibody == null)
		// Can't find it
		return null;

	// Copy fields from ibody to ihead
	for (var i in ibody)
		ihead[i] = ibody[i];

	return new Item(ihead);
}

// XXX - Get metadata for items
CacheManager.prototype.getitems = function()
{
	// XXX - Do something smart
	return null;
}

#endif	// _CacheManager_js_
