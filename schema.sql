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
	image		VARCHAR(255),	# URL to image to use
	active		BOOLEAN		# Is this feed active? Inactive feeds
					# are usually seasonal ones, e.g.
					# political ones that we only care about
					# at certain times, and want to ignore
					# the rest of the time, without
					# deleting it entirely.
		DEFAULT	1,		# Feeds are active by default
	username	char(32),	# Username, for authentication
	passwd		char(32),	# Password, for authentication
	PRIMARY KEY(id)
)
DEFAULT CHARSET=utf8;

/* counts
 * Holds the number of read and unread items in each feed. This is for
 * caching, really, since counting the items takes a long time (seconds).
 */
CREATE TABLE counts (
	feed_id		INT		NOT NULL,
	total		INT,
	num_read	INT,
	PRIMARY KEY(feed_id)
)
DEFAULT CHARSET=utf8;

# XXX - Trigger for when rows are removed: if the item is read,
# decrement counts.num_read.

/* trig_add_feed
 * Trigger to add a row to `counts` when we add a new feed.
 */
CREATE TRIGGER trig_add_feed
AFTER INSERT ON feeds
FOR EACH ROW
	INSERT INTO counts
	SET	feed_id = NEW.id,
		total = 0,
		num_read = 0;

/* trig_drop_feed
 * Trigger: when we delete a feed, delete its row in `counts`.
 */
CREATE TRIGGER trig_drop_feed
AFTER DELETE ON feeds
FOR EACH ROW
	DELETE FROM counts
	WHERE	feed_id = OLD.id;

# XXX - I want to create a trigger to update `counts` whenever `items`
# gets updated, but can't seem to get the syntax right.
#
# The thing to to should be:
# - When an item is added, it's unread, so increment counts.total and
# counts.num_unread.
# - When an item is deleted, decrement counts.total, and also
# counts.num_read if it was read.
# - When an item is updated, its is_read may have changed. If so, update
# num_read either up or down.

#CREATE TRIGGER trig_update_item
#AFTER UPDATE ON items
#FOR EACH ROW
#	DO
#		IF OLD.is_read
#		THEN
#			UPDATE counts
#			SET	num_read = IFNULL(num_read, 1) - 1
#		ELSE
#			UPDATE counts
#			SET	num_unread = IFNULL(num_unread, 1) - 1
#		ENDIF,
#		IF NEW.is_read
#		THEN
#			UPDATE counts
#			SET	num_read = IFNULL(num_read, 0) + 1
#		ELSE
#			UPDATE counts
#			SET	num_unread = IFNULL(num_unread, 0) + 1
#		ENDIF
#	;

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
	url		VARCHAR(511),	# Link to the full item
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
	mtime		TIMESTAMP,	# When the item was last altered
	PRIMARY KEY(id),
	UNIQUE KEY(feed_id, guid),	# Having (feed_id, guid)
					# instead of (guid) may be
					# overkill, but it's to ensure
					# that if two feeds have the
					# same item (e.g., one
					# contains the other), then
					# they'll be considered
					# separate items.
	# Indexes to speed up lookups
	KEY `last_update` (`last_update`),
	KEY `is_read` (`is_read`),
	KEY `mtime` (`mtime`)
)
DEFAULT CHARSET=utf8;
