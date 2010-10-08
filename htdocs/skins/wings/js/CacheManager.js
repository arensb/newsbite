/* CacheManager.js
 */
#ifndef _CacheManager_js_
#define _CacheManager_js_

#include "xhr.js"

/* Class CacheItem
 * This is a piece of data that's stored on the server, and can be
 * retrieved as a JSON string.
 * CacheItem interposes itself between the caller and the server: it
 * retrieves the item, caches a copy in localStorage as it arrives,
 * and parses the JSON before passing the data back to the caller.
 */
function CacheItem(key, url, params)
{
debug("new CacheItem("+key+","+url+")");
	this.key = key;		// Lookup key in localStorage
	this.url = url;		// URL whence to fetch it
	this.params = params;
				// Additional parameters to pass when
				// fetching
}

/* CacheItem member functions */
CacheItem.prototype = {
};

/* get
 * Return the currently cached version of the given key, submit a
 * request to the server to get the latest version of the data. When
 * the data arrives, call the given callback.
 */
CacheItem.prototype.get = function(callback)
{
debug("get()ing "+this.key);
	var retval = localStorage[this.key];
				// Get the current value of the thing

	// XXX - What to do when we're offline (and we know it)?
	// XXX - What to do when the request fails?
	var request = get_json_data(this.url,
				    this.params,
				    function(value)
				    {
					    CacheItem._get_callback(this.key,
								    value,
								    callback);
				    },
				    true);
	return retval;
}

CacheItem.prototype._get_callback = function(value, callback)
{
debug("_get_callback("+value+")");
	localStorage[this.key] = value;	// Store the new value

	if (typeof callback == "function")
		callback();
}

CacheItem._get_callback = function(key, value, callback)
{
debug("_get_callback("+value+")");
	localStorage[key] = value;	// Store the new value

	if (typeof callback == "function")
		callback();
}

/* CacheManager class
 * The object itself shouldn't try to store anything, at least not
 * user-level data like the list of feeds, or the content of articles.
 * Let the caller handle that.
 */
function CacheManager()
{
	// XXX - Should this try to do anything?
}

CacheManager.get_feeds = function(callback)
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
	localStorage["feeds"] = JSON.stringify(value);

	if (callback)
		callback(value);
}

#endif	// _CacheManager_js_
