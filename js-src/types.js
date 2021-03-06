#ifndef _types_js_
#define _types_js_
/* types.js
 * Useful types and classes.
 */

/* Feed class
 * Represents a feed, basically as represented by the 'feeds' table in
 * the database.
 */
function Feed(arg)
{
	if (typeof(arg) != "object")
		return;

	// Copy all the fields from 'arg' to 'this'
	for (var field in arg)
		this[field] = arg[field]
}

Feed.prototype = {
	// From database:
	// id (int)
	// title
	// subtitle
	// nickname
	// url
	// feed_url
	// description
	// last_update (time)
	// image (url to image)
	// active (bool)

	// Other fields:
	// _displaytitle (cached value from displaytitle())
	// _sortname (cached value from sortname())
};

/* displaytitle
 * Get the printable version of the feed title. If the user has set a
 * nickname, use that. Otherwise, use the title. If there's no
 * nickname and no title, use "[no title]".
 */
Feed.prototype.displaytitle = function()
{
	if (this._displaytitle != undefined)
		// Memoized value
		return this._displaytitle;

	if (this.nickname != null && this.nickname != "")
		// Use the user-specified nickname
		this._displaytitle = this.nickname;
	else if (this.title != null && !this.title.match(/^\s*$/))
		// Use the non-empty name that the feed has chosen for
		// itself
		this._displaytitle = this.title;
	else
		// Use a placeholder name
		this._displaytitle = "[no title]";

	return this._displaytitle;
}

/* sortname
 * Given a feed, return its sortname.
 */
Feed.prototype.sortname = function()
{
	if (this._sortname != undefined)
		// Memoized value
		return this._sortname;

	/* XXX - This sorting is very English-centric. Ought to allow
	 * different rules for different languages.
	 */
	var sorted = this.displaytitle();
	sorted = sorted.toLocaleUpperCase();
			// Pretty much the same as toUpperCase(),
			// except in Turkish, but hey, i18n is good.
	sorted = sorted.replace(/^(THE|A|AN|OF)\b\s*(.*)$/, "$2 $1");
			// Move "The" and others to the end
	sorted = sorted.replace(/\'(S|RE|LL)\b/g, "$1");	// ' STFU cpp
			// Remove apostrophe in "John's", "we're", "they'll"
	sorted = sorted.replace(/N\'T\b/g, "NT");		// ' STFU cpp
			// Remove apostrophe in "isn't", "hasn't", etc.
	sorted = sorted.replace(/&/g, " AND ");
	sorted = sorted.replace(/-/g, " ");
			// Hyphen becomes space.
	sorted = sorted.replace(/[^\w\s]/g, "");
			// Remove weird characters
	sorted = sorted.replace(/\s+/g, " ");
			// Collapse multiple spaces into one
	sorted = sorted.replace(/^\s+/, "");
			// Remove leading whitespace
	sorted = sorted.replace(/\s+$/, "");
			// Remove trailing whitespace
	this._sortname = sorted;
			// Memoize value for next time
	return sorted;
			// XXX - Could also chain calls:
			// return this.displaytitle()
			//	.toLocaleUpperCase()
			//	.replace(...)
			//	...
			// From the discussions I've seen, this is
			// less a matter of efficiency than of style
			// and readability. Do chained calls make the
			// code more readable?
}

// XXX - Would it be useful to have a FeedList class, for an array of Feeds?
// - Look up an existing feed by name, ID, whatever
// - Update an existing feed from a { } object.
// - Are details/tools/empty feeds/stale feeds currently displayed?
// - How are we sorting?

/* Item class
 * Class based on 'items' table in the database.
 */
function Item(arg)
{
	if (typeof(arg) != "object")
		return;

	// Copy all the fields from 'arg' to 'this'
	for (var field in arg)
		this[field] = arg[field]

	// Convert dates from time_t to Date objects.
	if (this['pub_date'] != undefined)
		this.pub_date    = this._toDate(this.pub_date);
	if (this['last_update'] != undefined)
		this.last_update = this._toDate(this.last_update);
	if (this['mtime'] != undefined)
		this.mtime       = this._toDate(this.mtime);
}

Item.prototype.displaytitle = function()
{
	if (this.title == null || this.title == "")
		return "[no title]";
	return this.title;
}

/* _toDate
 * Helper function to convert various things to Date.
 * We might get a date in various formats: an existing Date object, a
 * string in ISO8601 format, a string representing a time_t. Try to
 * figure out what we were given, and return the corresponding Date
 * object.
 */
Item.prototype._toDate = function(value)
{
	if (typeof value == "number")
	{
		// It's an integer (or at least a number). Assume it's
		// a time_t. Convert seconds to milliseconds, and
		// create a Date.
		return new Date(parseInt(value)*1000)
	}
	if (typeof value == "string")
	{
		// 'value' might be an ISO8601 date, or it might be a
		// time_t.
		if (isNaN(value))
			// It's not an integer; assume it's an ISO8601
			// date from Date.toJSON().
			return new Date(value);
		else
			// It's an integer; assume it's a time_t.
			// Convert seconds to milliseconds, and create
			// a Date.
			return new Date(parseInt(value)*1000)
	}
	if (value instanceof Date)
		// It's already a Date. Return it.
		return value;

	// It's none of the above.
	return null;
}
#endif	// _types_js_
