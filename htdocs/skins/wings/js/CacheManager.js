/* CacheManager.js
 */
#ifndef _CacheManager_js_
#define _CacheManager_js_

#include "xhr.js"

/* Class CacheItem
 */
function CacheItem()
{
	// XXX - Does anything go here?
}

/* CacheItem class functions */
CacheItem = {
	/* get: Return the currently cached version of the given key,
	 * submit a request to the server to get the latest version of
	 * the data. When the data arrives, call the given callback.
	 */
	get: function(key, url, params, callback)
	{
		var retval = localStorage[key];
				// Get the current value of the thing
		var handler;

		if (callback)
		{
			handler = function(text)
			{
			}
		}
		if (url)
		{
			var request = get_json_data(url,
						    params,
						    function(text) {
							    // XXX
						    },
						    true);
		}
		return retval;
	},
};

/* CacheItem member functions */
CacheItem.prototype = {
};

#endif	// _CacheManager_js_
