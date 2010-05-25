/* CREATE TABLE users (
);
*/

CREATE TABLE options (
	name		CHAR(64)	NOT NULL,
	value		VARCHAR(255),
	PRIMARY KEY(name)
)
DEFAULT CHARSET=utf8;

CREATE TABLE feed_options (
	feed_id		INT		NOT NULL,
	name		CHAR(64)	NOT NULL,
	value		VARCHAR(255),
	PRIMARY KEY(feed_id, name)
)
DEFAULT CHARSET=utf8;

/* groups
 * For grouping feeds into nested groups.
 * 'parent' says which group this group belongs to. The root group has
 * id 0.
 */
CREATE TABLE groups (
	id		INT		NOT NULL AUTO_INCREMENT,
	parent		INT		NOT NULL,
	name		VARCHAR(127),
	PRIMARY KEY(id)
)
DEFAULT CHARSET=utf8;

/* XXX - Need another table to specify which feeds go in which groups
 * (plural) and the relative order within each group.
 */

CREATE TABLE feeds (
	id		INT		NOT NULL AUTO_INCREMENT,
					# Numeric ID
	title		VARCHAR(127),	# Official title of feed
	subtitle	VARCHAR(127),	# Official subtitle of feed
	nickname	VARCHAR(127),	# User-specified nickname (when the title blows)
	url		VARCHAR(255),	# Site URL
	feed_url	VARCHAR(255),	# RSS feed URL
	description	TINYTEXT,	# Brief description of the feed
	last_update	DATETIME,	# When this feed was last updated
	ttl		TIME,		# Time to live
	image		VARCHAR(255),	# URL to image to use
	active		BOOLEAN		# Is this feed active? Inactive feeds
					# are usually seasonal ones, e.g.
					# political ones that we only care about
					# at certain times, and want to ignore
					# the rest of the time, without
					# deleting it entirely.
		DEFAULT	1,		# Feeds are active by default
#	skip_hours	SET('0','1', ..., '23'),	# Hours when not to refresh
#	skip_days	SET('Sunday', 'Monday',	# Days when not to refresh
#			'Tuesday', 'Wednesday', Thursday', 'Friday',
#			'Saturday'),
	username	char(32),	# Username, for authentication
	passwd		char(32),	# Password, for authentication
	PRIMARY KEY(id)
)
DEFAULT CHARSET=utf8;

/* items
 * An item is a story or article in a feed.
 */
CREATE TABLE items (
	id		INT		NOT NULL AUTO_INCREMENT,
					# Unique identifier for this item
					# XXX - Should we use GUID, in case
					# the same article shows up in two
					# different feeds?
	feed_id		INT NOT NULL,	# ID of feed
	url		VARCHAR(255),	# Link to the full item
	title		TINYTEXT,	# Title of the item
	summary		TEXT,		# Summary of the item
	content		TEXT,		# Full content of the item
	author		VARCHAR(127),	# Author of the item
			# XXX - Should this be broken down into author name,
			# URL, and email? Probably yes.
	category	VARCHAR(255),	# Categories the story goes in
	comment_url	VARCHAR(255),	# URL for page with comments
	comment_rss	VARCHAR(255),	# URL for RSS feed for comments
	guid		VARCHAR(127) NOT NULL,	# Globally-unique ID.
	pub_date	DATETIME,	# Publication date
	last_update	DATETIME,	# Time when item was last updated
	is_read		BOOLEAN,	# Has the item been read?
	PRIMARY KEY(id),
	UNIQUE KEY(feed_id, guid)	# Having (feed_id, guid)
					# instead of (guid) may be
					# overkill, but it's to ensure
					# that if two feeds have the
					# same item (e.g., one
					# contains the other), then
					# they'll be considered
					# separate items.
)
DEFAULT CHARSET=utf8;
