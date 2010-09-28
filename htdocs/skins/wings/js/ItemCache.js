#ifndef _ItemCache_js_
#define _ItemCache_js_
/* ItemCache.js
 * Module for manipulating local storage.
 */
/* XXX - Things this module's API needs:
 * - Periodically refresh the cache from the server, either
 *	- as a worker thread (desktop browsers)
 *	- as part of the main thread (mobile Safari)
 * - Get/set set of known feeds
 * ? Get a feed by ID
 * - Get/set an item by ID
 * - Get the N most recent items in feed M/all.
 * - Get the N items in feed M/all that are older/younger than item J.
 * - Send an event when:
 *	- A new item in feed F appears.
 *	- Status of item J has been updated.
 *	- Status of feed F has changed.
 * - Maintenance:
 *	- Remove items from nonexistent feeds
 * - Expire old items from cache
 * - Add/remove listeners for interesting changes
 *   i.e., "let me know when feed F changes", "let me know if item J
 *   changes".
 * - Mark an item as read/unread.
 *   Need status to see whether the change went through.
 * - Synchronize time with server?
 * - Flush any pending changes that couldn't go through because the
 *   browser was offline (or the server was unreachable, or whatever).
 */
/* XXX - Set a prefix for keys, in case we need to have two storage
 * databases?
 */
/* XXX - Store transitory data in sessionStorage, and more permanent
 * data in localStorage?
 */
/* XXX - Naming convention for functions, to keep things straight.
 * get_X()	read X from local storage
 * fetch_X()	get updated X from server
 */
/* XXX - In normal (non-SQLite) localStorage, keys can only be strings.
 * How should data be subdivided?
 * - item, id 123	"item/123"
 */
/* XXX - See
 * http://hacks.mozilla.org/2009/06/localstorage/
 * for un/serializing using native JSON.
 * JSON.stringify(somevalue)
 * JSON.parse(json_string)
 *
 * Apparently defined everywhere, including mobile Safari.
 */
/* XXX - Ideally, should know how to use SQLite.
 * How to tell if this is available?
 */

function ItemCache()
{
	// XXX
}

ItemCache.scan_cache = function()
{
	// XXX
}

#endif	// _ItemCache_js_
