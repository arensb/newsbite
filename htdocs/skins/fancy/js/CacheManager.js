#ifndef _CacheManager_js_
#define _CacheManager_js_
/* XXX - Perhaps organize things as follows:
 * localStorage:
 * - feeds: basically a mirror of the 'feeds' table.
 * - ihead:NNN: basically a mirror of 'items where id == NNN', but
 *	without the summary or content.
 * - ibody:NNN: { "summary": <summary-text>,
 *			"content": <content-text>
 *		}
 */
#include "types.js"		/* Feed and Item classes */

/* CacheManager.js
 * Manage local storage.
 */
/* XXX - Need some kind of consistency check a la fsck.
 * Store a value indicating whether local storage is clean or not,
 * in case a non-atomic operation gets interrupted.
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
	for (f in a)
		retval.push(new Feed(a[f]));
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
