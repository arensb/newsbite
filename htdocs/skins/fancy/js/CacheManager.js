#ifndef _CacheManager_js_
#define _CacheManager_js_
/* CacheManager.js
 * Manage local storage.
 */
/* XXX - Need some kind of consistency check a la fsck.
 * Store a value indicating whether local storage is clean or not,
 * in case a non-atomic operation gets interrupted.
 */

function CacheManager()
{
	/* XXX - Load known objects? Or would that affect execution speed? */
}

// XXX - Save list of feeds

// XXX - Retrieve stored list of feeds
CacheManager.prototype.feeds = function()
{
	var retval;
	try {
		retval = localStorage.getItem('feeds');
	} catch (e) {
		localStorage.removeItem('feeds');
		return null;
	}
	retval = JSON.parse(retval);
			 // XXX - Error-checking.
	return retval;
}

// XXX - Update saved data for one feed

// XXX - Get one feed (not all)

// XXX - Save feed metadata
CacheManager.prototype.store_feeds = function(feeds)
{
	var str = JSON.stringify(feeds);
			  // XXX - Error-checking? Or should we just assume
			  // that strinfigy will Do the Right Thing?
	localStorage.setItem('feeds', str);
	// XXX - Error-checking
}

// XXX - Retrieve feed metadata

// XXX - Save an item

// XXX - Retrieve an item (by id)

#endif	// _CacheManager_js_
