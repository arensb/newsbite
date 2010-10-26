/* CacheManager.js
 */
#ifndef _CacheManager_js_
#define _CacheManager_js_

#include "xhr.js"

// XXX - Is CacheItem even used? I don't think so.
///* Class CacheItem
// * This is a piece of data that's stored on the server, and can be
// * retrieved as a JSON string.
// * CacheItem interposes itself between the caller and the server: it
// * retrieves the item, caches a copy in localStorage as it arrives,
// * and parses the JSON before passing the data back to the caller.
// */
//function CacheItem(key, url, params)
//{
//debug("new CacheItem("+key+","+url+")");
//	this.key = key;		// Lookup key in localStorage
//	this.url = url;		// URL whence to fetch it
//	this.params = params;
//				// Additional parameters to pass when
//				// fetching
//}
//
///* CacheItem member functions */
//CacheItem.prototype = {
//};
//
///* get
// * Return the currently cached version of the given key, submit a
// * request to the server to get the latest version of the data. When
// * the data arrives, call the given callback.
// */
//CacheItem.prototype.get = function(callback)
//{
//debug("get()ing "+this.key);
//	var retval = localStorage[this.key];
//				// Get the current value of the thing
//
//	// XXX - What to do when we're offline (and we know it)?
//	// XXX - What to do when the request fails?
//	var request = get_json_data(this.url,
//				    this.params,
//				    function(value)
//				    {
//					    CacheItem._get_callback(this.key,
//								    value,
//								    callback);
//				    },
//				    true);
//	return retval;
//}
//
//CacheItem.prototype._get_callback = function(value, callback)
//{
//debug("_get_callback("+value+")");
//	localStorage.removeItem(this.key);
//	localStorage[this.key] = value;	// Store the new value
//
//	if (typeof callback == "function")
//		callback();
//}
//
//CacheItem._get_callback = function(key, value, callback)
//{
//debug("_get_callback("+value+")");
//	localStorage.removeItem(key);
//	localStorage[key] = value;	// Store the new value
//
//	if (typeof callback == "function")
//		callback();
//}

/* CacheManager class
 * The object itself shouldn't try to store anything, at least not
 * user-level data like the list of feeds, or the content of articles.
 * Let the caller handle that.
 */
function CacheManager()
{
	// XXX - Should this try to do anything?

	this.scan_cache();
}

CacheManager.prototype = {
	feeds: [],
	items: [],
};

/* scan_cache
 * Look at local storage, and see what's already there.
 */
CacheManager.prototype.scan_cache = function()
{

	/* Scan local storage and see what we inherited from the last
	 * run
	 */
	for (var i = 0; i < localStorage.length; i++)
	{
		var key = localStorage.key(i);
		var value = localStorage.getItem(key);
		var matches;		// For regexp matches, further down

		if (key == "feeds")
		{
			var feeds;
			// List of feeds we know about
			try {
				feeds = JSON.parse(value);
			} catch (e) {
				// XXX - Do something smart
				feeds = [];
			}
			for (var f in feeds)
			{
				var feed = feeds[f];
				this.feeds[feed.id] = feed;
			}
			continue;
		}

		matches = key.match(/^item\/(\d+)$/);
		if (matches)
		{
			// Found an entry with key "item/12345". This
			// is an article.
			var item;
			try {
				item = JSON.parse(value);
			} catch (e) {
				// XXX - Do something smart
				continue;
			}

			// Get some interesting bits of the article,
			// and add them to the in-memory array of
			// items.
			this.items[item.id] =
				{
					id:		item.id,
					feed_id:	item.feed_id,
					pub_date:	item.pub_date,
					last_update:	item.last_update,
					is_read:	item.is_read,
				};
			continue;
		}

debug("Unidentified key ["+key+"]");
		localStorage.removeItem(key);
	}
}

CacheManager.prototype.get_feeds = function(callback)
{
	var retval;

	/* Get the current value */
	try {
		retval = JSON.parse(localStorage["feeds"]);
	} catch (e) {
		delete localStorage["feeds"];
		retval = undefined;
	}

	/* Start a request for an updated version */
	// XXX - Unless we're offline?
	get_json_data("feeds.php",
		      { "o": "json" },
		      function(value)
		      {
			CacheManager._get_feeds_callback(value, callback)
		      },
		      true);

	/* Return the current version */
	return retval;
};

CacheManager._get_feeds_callback = function(value, callback)
{
	/* Save the new version */
	// XXX - Should have a separate function for storing locally,
	// depending on whether openDatabase exists or not.
	localStorage.removeItem("feeds");
			// This is required on Safari. Otherwise it
			// sometimes complains of exceeded quota when
			// we write the new value.
	localStorage["feeds"] = JSON.stringify(value);

	if (callback)
		callback(value);
}

// XXX - Instead of 'feed_id', should probably take an object holding
// criteria saying which items to get. E.g., feed N; group M; only
// read/read and unread; etc.
CacheManager.prototype.get_items = function(feed_id, callback)
{
	var retval = [];

	// Get the cached items in this feed.
	for (var i in this.items)
	{
		var item = this.items[i];

		if (item.feed_id == feed_id)
		{
			// XXX - Is it safe to assume that JSON.parse
			// won't throw an error?
			retval.push(JSON.parse(localStorage["item/"+item.id]));
		}
	}

	/* Start a request for new values */
	var cm2 = this;		// So the callback can call an object method
	get_json_data("view.php",
		      { "id": feed_id,
		        "o": "json"
		      },
		      function(value)
		      {
			      CacheManager._get_items_callback(cm2, value, callback)
		      },
		      true);

	/* Return the current version */
	return retval;
}

CacheManager._get_items_callback = function(cm, value, callback)
{
debug("Inside _get_items_callback, cm == "+cm);

	for (var i = 0; i < value.items.length; i++)
	{
		var item = value.items[i];

		// XXX - Update entries in cm.items
		if (cm.items[item.id])
		{
debug("Updating item "+item.id);
			// XXX - Do something smart
		} else {
debug("New item "+item.id);
			// XXX - This is a new item. Remember it
			cm.items[item.id] =
				{
					id:		item.id,
					feed_id:	item.feed_id,
					pub_date:	item.pub_date,
					last_update:	item.last_update,
					is_read:	item.is_read,
				};
		}

		// XXX - Should have a separate function for storing
		// locally, depending on whether openDatabase exists
		// or not.
		localStorage.removeItem("item/"+item.id);
		localStorage["item/"+item.id] = JSON.stringify(item);
			// XXX - This might fail. If so, probably need
			// to expire cache.
	}

	if (callback)
		callback(value);
}

#endif	// _CacheManager_js_
