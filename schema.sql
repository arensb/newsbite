/* CREATE TABLE users (
);
*/

CREATE TABLE options (
	name		CHAR(64)	NOT NULL,
	value		VARCHAR(255),
	PRIMARY KEY(name)
)
DEFAULT CHARSET=utf8mb4;

CREATE TABLE feed_options (
	feed_id		INT		NOT NULL,
	name		CHAR(64)	NOT NULL,
	value		VARCHAR(255),
	PRIMARY KEY(feed_id, name)
)
DEFAULT CHARSET=utf8mb4;

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
DEFAULT CHARSET=utf8mb4;

/* Create one mandtory group: "All", with ID -1 */
INSERT INTO groups (name, parent) VALUES ("All", -1);
UPDATE groups SET id=-1 WHERE id=last_insert_id();

/* group_members
 * Lists members of groups, as a simple "X is a member of Y" relationship.
 * If `member` is nonnegative, it's the ID of a feed. If it's negative,
 * then it's the ID of a group in table `groups`.
 * XXX - Actually, a feed can have multiple parents, but a group
 * should probably only have one parent, at least for now.
'*/
CREATE TABLE group_members (
	member		INT		NOT NULL,
	parent		INT		NOT NULL DEFAULT -1,
	UNIQUE KEY (member, parent)
)
DEFAULT CHARSET=utf8mb4;

CREATE TABLE feeds (
	id		INT		NOT NULL AUTO_INCREMENT,
					# Numeric ID
	title		VARCHAR(127),	# Official title of feed
	subtitle	VARCHAR(127),	# Official subtitle of feed
	nickname	VARCHAR(127),	# User-specified nickname (when the title blows)
	url		VARCHAR(255),	# Site URL
	feed_url	VARCHAR(255),	# RSS feed URL
	description	TEXT,		# Brief description of the feed
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
DEFAULT CHARSET=utf8mb4;

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
	title		TEXT,		# Title of the item
	summary		MEDIUMTEXT,	# Summary of the item
	content		MEDIUMTEXT,	# Full content of the item
	author		VARCHAR(127),	# Author of the item
			# XXX - Should this be broken down into author name,
			# URL, and email? Probably yes.
	category	VARCHAR(255),	# Categories the story goes in
	comment_url	VARCHAR(255),	# URL for page with comments
	comment_rss	VARCHAR(255),	# URL for RSS feed for comments
	guid		VARCHAR(64) NOT NULL,	# Globally-unique ID.
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
DEFAULT CHARSET=utf8mb4;

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
DEFAULT CHARSET=utf8mb4;

/* add_feed
 * Trigger to add a row to `counts` when we add a new feed.
 */
CREATE TRIGGER add_feed
AFTER INSERT ON feeds
FOR EACH ROW
	INSERT INTO counts
	SET	feed_id = NEW.id,
		total = 0,
		num_read = 0;

/* drop_feed
 * Trigger: when we delete a feed, delete its row in `counts`.
 */
CREATE TRIGGER drop_feed
AFTER DELETE ON feeds
FOR EACH ROW
	DELETE FROM counts
	WHERE	feed_id = OLD.id;

/* add_item
 * When we add a new item, it's initially unread. Increment `counts.total`.
 * '
 */
CREATE TRIGGER add_item
AFTER INSERT ON items
FOR EACH ROW
	UPDATE	counts
	SET	total = total + 1
	WHERE	feed_id = NEW.feed_id;

/* del_item
 * When we delete an item, decrement `counts.total`, and also num_read,
 * but only if the item being deleted was read.
 */
DELIMITER $$
CREATE TRIGGER del_item
AFTER DELETE ON items
FOR EACH ROW
    BEGIN
        UPDATE counts
        SET total = total - 1,
	    num_read = num_read - IF(OLD.is_read, 1, 0)
	WHERE counts.feed_id = OLD.feed_id;
    END$$
DELIMITER ;

/* update_item
 * Update `counts` when an item gets updated.
 * The UPDATE here uses a trick: the fact that booleans are also integers:
 * OLD.is_read	NEW.is_read	=> counts.num_read
 * 0		0		+0
 * 0		1		+1
 * 1		0		-1
 * 1		1		+0
 */
CREATE TRIGGER update_item
AFTER UPDATE ON items
FOR EACH ROW
	UPDATE counts
	SET num_read = num_read + NEW.is_read - OLD.is_read
	WHERE counts.feed_id = OLD.feed_id;

/* get_option function
 * Get the value of option `opt` for feed with ID `fid`, and return it.
 * If it's not explicitly set for the feed, get the default.
 * If there's no default, return NULL.
 */
/* XXX - If the feed doesn't have the option, ought to see whether the
 * feed is in a group that has the option set, and whether *that* group
 * is in a group that has the option set, and so on, recursively.
 * 	This raises another question: what if the feed is in two groups
 * that have different values for $option? It's tempting to say that
 * the result is undefined. Is there a better answer?
 */
DELIMITER //
CREATE FUNCTION `get_option`
    (fid INT,
     opt CHAR(64))
    RETURNS	CHAR(255)
BEGIN
	SET @retval = NULL;

	# Get the option value for this particular feed.
	SELECT	`value` into @retval
	FROM	`feed_options`
	WHERE	`feed_id` = fid;

	IF @retval IS NOT NULL THEN
	   RETURN @retval;
	END IF;

	SELECT	`value` into @retval
	FROM	`feed_options`
	WHERE	`feed_id` = 0;

	RETURN @retval;
END //
DELIMITER ;
