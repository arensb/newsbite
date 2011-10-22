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

/* feeds
 * Retrieve stored list of feeds.
 */
CacheManager.prototype.feeds = function()
{
	var retval;

	// Get the cached set of feeds from local storage
	try {
		retval = localStorage.getItem('feeds');
	} catch (e) {
		localStorage.removeItem('feeds');
		return null;
	}
	if (retval == null)
		// No feeds stored yet
		return null;

	// Parse the retrieved value
	try {
		retval = JSON.parse(retval);
	} catch (e) {
		localStorage.removeItem('feeds');
		return null;
	}
	return retval;
}

// XXX - Update saved data for one feed

// XXX - Get one feed (not all)

/* store_feeds
 * Save list of feeds to localStorage.
 */
CacheManager.prototype.store_feeds = function(feeds)
{
	var str = JSON.stringify(feeds);
			  // XXX - Error-checking? Or should we just assume
			  // that strinfigy will Do the Right Thing?
	localStorage.setItem('feeds', str);
	// XXX - Error-checking
}

// XXX - Retrieve feed metadata

// XXX - Save an article

// XXX - Retrieve an article (by id)

#endif	// _CacheManager_js_
