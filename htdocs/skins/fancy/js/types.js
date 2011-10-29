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
	for (field in arg)
		this[field] = arg[field]
}

Feed.prototype = {
	// id (int)
	// title
	// subtitle
	// nickname
	// url
	// feed_url
	// description
	// last_update (time)
	// ttl
	// image (url to image)
	// active (bool)
};

/* displaytitle
 * Get the printable version of the feed title. If the user has set a
 * nickname, use that. Otherwise, use the title. If there's no
 * nickname and no title, use "[no title]".
 */
Feed.prototype.displaytitle = function()
{
	if (this.nickname != null && this.nickname != "")
		return this.nickname;
	else if (this.title != null && this.title != "")
		return this.title;
	else
		return "[no title]";
}

/* sortname
 * Given a feed, return its sortname.
 */
Feed.prototype.sortname = function()
{
return;
//	if (this.sortname != undefined)
//		// Memoized value
//		return this.sortname;
//
//	var sortname = this.displaytitle();
//	// XXX - See mkalbum.
}

// XXX - Would it be useful to have a FeedList class, for an array of Feeds?

/* XXX - Item class
 * Class based on 'items' table in the database.
 */
function Item(arg)
{
	if (typeof(arg) != "object")
		return;

	// Copy all the fields from 'arg' to 'this'
	for (field in arg)
		this[field] = arg[field]
}
#endif	// _types_js_
