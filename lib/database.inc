<?php
/* database.inc
 * Database-related functions. Nothing else should access the database
 * directly.
 */
/* Some numbers, as of Jan 20 2016: 54% of the data in the 'items' table is
 * the 'summary' field, 26% is the 'content' field, and everything
 * else is that last quarter.
 * 'items' itself is 47Mb.
 * Currently about 98% of all items are marked read: there's 1.5Mb of data
 * for unread articles.
 */
require_once("config.inc");
require_once("hooks.inc");

define('DB_MAX_RESULTS', 50);	// Max # of results to return, if not
				// otherwise specified.

$dbh = NULL;		// Database handle

/* db_errno, db_errmsg
 * If one of the functions here returns an error, it should put an
 * error number and message here, so the caller can display something
 * smart.
 */
$db_errno = NULL;	// Error number
$db_errmsg = NULL;	// Error message

load_hooks(PLUGIN_DIR);

/* db_connect
 * Connect to the database if necessary. Return a handle to the
 * database.
 */
function db_connect()
{
	global $dbh;
	global $DB_HOST, $DB_USER, $DB_PASS, $DB_NAME;

	// If there's already a database handle, use that.
	if (isset($dbh))
		return $dbh;

	// Create a new connection
	@$dbh = new mysqli($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);
	if (mysqli_connect_errno())
	{
		abort("Connect failed: ". mysqli_connect_error());
		exit();
	}

	/* Explicitly set the character set */
	// XXX - There's probably a better way to do this.
	$dbh->set_charset('utf8mb4');

	return $dbh;
}

/* db_disconnect
 * Disconnect from the database.
 */
function db_disconnect()
{
	global $dbh;

	if (!isset($dbh))
		// Already disconnected.
		return;

	$dbh->close();
	$dbh = NULL;
	return;
}

/* db_get_feeds
 * Return a list of all feeds.
 * If $inactive is true, fetch inactive feeds as well.
 */
function db_get_feeds($inactive = FALSE)
{
	global $STALE_DAYS;

	$dbh = db_connect();

	$query = sprintf(<<<EOT
SELECT	*,
	UNIX_TIMESTAMP(last_update) as unix_last_update,
	IF(DATEDIFF(NOW(),
		    IFNULL(last_update, '1970-01-01')) > %d, 1, 0) as stale
FROM	feeds
EOT
		,
		$STALE_DAYS);
	if (!$inactive)
		$query .= "\nWHERE active";

	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		$row['active'] = $row['active'] ? TRUE : FALSE;
			# Make sure 'active' is properly boolean
		$retval[$row['id']] = $row;
	}

	// Also look up any entries in group_members: tell us which
	// groups a feed is in.
	$query = "SELECT * FROM group_members";
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	while (list($member, $parent) = $result->fetch_row())
	{
		if (!isset($retval[$member]))
			continue;
		$retval[$member]['groups'][] = $parent;
	}

	return $retval;
}

/* db_get_feed
 * Fetch the database record for a single feed, specified by its ID.
 * Returns NULL in case of error.
 */
function db_get_feed($id)
{
	global $STALE_DAYS;

	$dbh = db_connect();

	$query = sprintf(<<<EOT
SELECT	*,
	UNIX_TIMESTAMP(last_update) as unix_last_update,
	IF(DATEDIFF(NOW(),
		    IFNULL(last_update, '1970-01-01')) > %d, 1, 0) as stale
FROM	feeds
WHERE	id = %d
EOT
		,
		$STALE_DAYS,
		$id);
	$result = $dbh->query($query);
	if ($result->num_rows <= 0)
		// No matching feeds
		return NULL;
	$row = $result->fetch_assoc();
	$retval = $row;

	// Also look up any entries in group_members: tell us which
	// groups the feed is in.
	$retval['groups'] = array();
	$query = sprintf(<<<EOT
SELECT	parent
FROM	group_members
WHERE	member=%d
EOT
		,
		$id);
	$result = $dbh->query($query);
	while ($row = $result->fetch_row())
	{
		$retval['groups'][] = $row[0];
	}

	return $retval;
}

/* db_get_feed_counts
 * Get the number of read/unread articles in a feed, as well as the
 * latest publication time and update time of any article in the feed.
 *
 * Returns an array of the form
 *	array(
 *		'read'			=> 150,
 *		'unread'		=> 12,
 *	);
 */
function db_get_feed_counts($feed_id)
{
	$dbh = db_connect();

	/* Get the number of read/unread items */
	$query = sprintf(<<<EOT
SELECT	num_read AS `read`,
	total-num_read AS `unread`
FROM	counts
WHERE	feed_id = %d
EOT
			 ,
			 $feed_id);
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;
		return NULL;
	}
	$retval = $result->fetch_assoc();
		// XXX - Error-checking

	return $retval;
}

/* db_get_feed_options
 * Get the options pertaining to a feed.
 * Returns an array of $optionN => $valueN.
 */
/* XXX - $recursive == TRUE ought to check the feed's parent groups
 * for defaults for any option that isn't overridden elsewhere. That
 * is, if the "Washington Post" feed is in group "News", which is in
 * group "All", then options set in "All" can be overriden in "News",
 * and those can in turn be overridden by "Washington Post".
 */
function db_get_feed_options($feed_id, $recursive = FALSE)
{
	$dbh = db_connect();

	$query = sprintf(<<<EOT
SELECT	`name` AS `option`,
	`value`
FROM	feed_options
WHERE	feed_id = %d
EOT
	    ,
	    $feed_id);
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;
#echo "Got an error: $db_errno: [$db_errmsg]<br/>\n";
		return NULL;
	}

	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		$retval[$row['option']] = $row['value'];
	}

#echo "db_get_feed_options($feed_id) returning [", print_r($retval), "]<br/>\n";
	return $retval;
}

# XXX - db_set_feed_options($feed_id, $options (opt => value ...)
function db_set_feed_option($feed_id, $opt, $val)
{
	$query = <<<EOT
INSERT INTO	feed_options
	(feed_id, name, value)
VALUES	(?,       ?,     ?)
ON DUPLICATE KEY UPDATE
	name=?,
	value=?
EOT;
	$stmt = $dbh->prepare($query);
	$stmt->bind_param("dssss",
	    $feed_id,
	    $opt, $val,
	    $opt, $val);
	$err = $stmt->execute();
	if ($err)
		echo "OK (", print_r($err, true), ") errno ", $dbh->errno, ", error [", $dbh->error, "]<br/>\n";
	else
		echo "<b>Error: ", $dbh->errno, ": \"", $dbh->error;
}

/* db_get_all_feed_counts
 * Get the number of read/unread articles in all feeds, as well as
 * the last publication and update time for each feed.
 *
 * Returns an array of the form
 *	array(
 *		190	=> array(		// Feed ID 190
 *			'read'	=> 150,
 *			'unread'=> 12,
 *			),
 *		191	=> array( ... ),	// Feed ID 191
 *		...
 *	);
 * I.e., the same as calling db_get_feed_counts() once for every feed ID.
 */
function db_get_all_feed_counts()
{
	$dbh = db_connect();

	/* Get the number of read/unread items in each feed. This uses
	 * the `counts` table, which is updated by MySQL triggers.
	 */
	$query = <<<EOT
SELECT		feed_id AS `feed_id`,
		num_read AS `read`,
		total-num_read AS `unread`
FROM		counts
EOT;
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;
		return NULL;
	}

	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		// XXX - Error-checking
		$feed_id = $row['feed_id'];
		unset($row['feed_id']);
		$retval[$feed_id] = $row;
	}

	return $retval;
}

/* db_get_some_feed_items
 * Fetch items from the database that meet certain criteria.
 * The criteria are specified by the variable argument list: the list
 * is a list of {keyword, value} pairs.
 *
 * This function can also be called with a single argument, an assoc
 * of keyword => value pairs.
 *
 * feed_id => int id
 *	Which feed's items to fetch. If missing, fetch from all feeds
 * start_at => int n
 *	Skip the first n items.
 * max_items => int n
 *	How many items to fetch.
 * order => date_desc (default) | date_asc
 *	Order in which to return the items
 * read => string:
 *	"read": return only items with is_read == true.
 *	"unread": return only items with is_read == false.
 *	"both": return both.
 *	defaults to "unread"
 */
function db_get_some_feed_items(/* varargs */)
{
	$argc = func_num_args();

	if ($argc == 1)
	{
		/* We were called with a single argument: this is an
		 * assoc with keyword => value pairs.
		 */
		$args = func_get_arg(0);
	} else {
		/* We were called with n != 1 arguments. This means a
		 * variable number of arguments as keyword => value
		 * pairs.
		 */
		/* Sanity check: make sure we got an even number of
		 * arguments
		 */
		if ($argc % 2 != 0)
		{
			die("db_get_some_feed_items(): got $argc arguments. Expecting an even number.");
		}

		/* Turn the argument list into an assoc */
		$args = array();
		for ($i = 0; $i < $argc; $i += 2)
			$args[func_get_arg($i)] = func_get_arg($i+1);
	}

	/* Add limit and offset to query */
	if (!is_int($args['max_items']))
		$args['max_items'] = DB_MAX_RESULTS;

	/* About the query:
	 * The most common query is going to be asking for the last 25
	 * unread items in active feeds. This will wind up being
	 * something like:
	 *	SELECT items.*
	 *	FROM items
	 *	WHERE feed_id IN
	 *		(SELECT id
	 *		  FROM	feeds
	 *		 WHERE	active)
	 *	  AND !items.is_read
	 *	ORDER BY items.last_update DESC,
	 *		items.id DESC
	 *	LIMIT 25
	 *
	 * The inner "(SELECT FROM feeds" allows us to select the
	 * feeds that match certain conditions, like being active,
	 * being in a group, or having id==123.
	 */

	if (!isset($args['order']))
		$args['order'] = "date_desc";	// Stop PHP from complaining
						// later on.

	/* Beginning of query */
	/* SCHEMA */
	$query = <<<EOT
SELECT	items.id, items.feed_id, items.url, items.title,
	items.summary, items.content,
	items.author, items.category,
	items.comment_url, items.comment_rss, items.guid,
	UNIX_TIMESTAMP(items.pub_date) as `pub_date`,
	UNIX_TIMESTAMP(items.last_update) as `last_update`,
	IF(is_read, 1, 0) as `is_read`,
	UNIX_TIMESTAMP(mtime) as `mtime`
FROM	items

EOT;

	// $feed_conditions: list of conditions that the feeds must match.
	$feed_conditions = array(
		);

	/* Were we asked for just one feed ID? */
	if (isset($args['feed_id']) && is_numeric($args['feed_id']))
	{
		$c = sprintf("items.feed_id=%d\n", $args['feed_id']);
		array_push($feed_conditions, $c);
	} else {
		array_push($feed_conditions, "feeds.active");
	}

	// XXX - Ought to be able to select feed by group.
#	// XXX - "group" == <gid>.
#	// I don't think we want to accept a group name "e.g., group=News",
#	// since those need not be unique. Though if the argument isn't an
#	// integer, it might be worth looking to see whether it's a name.
#	if (isset($args['group']))
#	{
#		$gid = $args['group'];
#		error_log("Looking for feeds in group ".$gid);
#		// XXX - If gid is not an integer, try to find a unique
#		// gid that matches its name.
#		if (!is_int($gid))
#		{
#			error_log("Non-numeric gid ".$gid);
#			return NULL;
#		}
#
#		// XXX - Find all feeds in this group (i.e., find all
#		// descendant groups, and all feeds in those groups).
#		$feeds = db_feeds_in_group($gid);
#
#		// XXX - Append to $subconditions something like
#		// "feed_id in (1, 3, 7, 19, 28)"
#	}
	$feed_selector = "feed_id in (SELECT id FROM feeds WHERE\n" .
		implode("\nAND ", $feed_conditions) . "\n)";

	// Collect all the other conditions on the articles we're
	// interested in.
	$conditions = array(	// All of the "WHERE x AND y AND z" conditions
		$feed_selector,
		);

	/* Do we want just read items, or just unread ones, or both? */
	if (isset($args['read']))
	{
		switch ($args['read'])
		{
		    case "read":	# Return only read items
			array_push($conditions, "items.is_read");
			break;
		    case "unread":	# Return only unread items
			array_push($conditions, "!items.is_read");
			break;
		    case "both":
			break;
		    default:		# Invalid 'read' value
			error_log("db_get_some_feed_items: Invalid 'read' value: ".$args['read']);
			return NULL;
		}
	} else {
		// Default to returning only unread items
		array_push($conditions, "!items.is_read");
	}

	/* Add selection conditions to query */
	if (count($conditions) > 0)
	{
		$query .= "WHERE " . implode("\nAND ", $conditions) . "\n";
	}

	/* Specify order in query.
	 * Use id as a tiebreaker, to ensure a consistent order.
	 */
	switch ($args['order'])
	{
	    case "date_asc":
		$query .= "ORDER BY items.last_update ASC, id ASC\n";
		break;
	    case "date_desc":
	    default:
		// XXX - This fall-through means we don't notice bogus
		// values. Then again, we can probably trust the
		// caller.
		$query .= "ORDER BY items.last_update DESC, id DESC\n";
		break;
	}

	$query .= "LIMIT " . $args['max_items'] . "\n";

	if (isset($args['start_at']) && is_int($args['start_at']))
		$query .= "OFFSET " . $args['start_at'] . "\n";

	/* Run the SQL query */
	$dbh = db_connect();
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		# Make sure is_read is properly converted to boolean.
		$row['is_read'] = $row['is_read'] ? TRUE : FALSE;
		$retval[] = $row;
	}

	return $retval;
}

/* db_add_feed
 * Add a new feed entry to the database. $feed is an assoc of
 * key => value mappings, keyed by column name.
 *
 * This can be used to set as many or as few columns as desired. For
 * an ordinary subscription, this will typically just include the feed
 * URL. But other fields may be specified if importing an OPML file.
 */
function db_add_feed($feed)
{
	$dbh = db_connect();

	$fields = array();	// Fields we'll be adding
	$values = array();	// The fields' values
	foreach ($feed as $k => $v)
	{
		switch ($k)
		{
		    case "title":
		    case "subtitle":
		    case "nickname":
		    case "url":
		    case "feed_url":
		    case "description":
		    case "image":
		    case "username":
		    case "passwd":
			// These are strings
			$fields[] = $k;
			$values[] = "'" . $dbh->real_escape_string($v) . "'";
			break;

		    case "last_update":
			/* Normally, we ignore the last_update time
			 * from the feed itself (as opposed to the
			 * time from the individual items). But in
			 * this case, since we're adding a new feed,
			 * having a publisher-supplied value might be
			 * better than nothing.
			 */
			/* XXX - Then again, it might not. We could
			 * just as easily initialize it to now.
			 */
			// Convert from time_t to SQL date
			$fields[] = "last_update";
			$values[] = "FROM_UNIXTIME($v)";
			break;

		    default:
			// XXX - Complain about unknown field
//echo "Unknown field [$k]<br/>\n";
			break;
		}
	}

	$query = "INSERT INTO feeds (" .
		implode(",", $fields) .
		") VALUES (" .
		implode(",", $values) .
		")";
	$stmt = $dbh->prepare($query);
		// XXX - Error-checking
	$err = $stmt->execute();
	if (!$err)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;
		$stmt->close();
		return false;
	}
	$feed_id = $dbh->insert_id;
	$stmt->close();

	// XXX - Call user hooks

	return $feed_id;
}

/* db_delete_feed
 * Delete a feed from the database (both feed entry and articles).
 * Returns FALSE in case of error, or TRUE otherwise.
 */
// XXX - Should probably return something more interesting, like an
// error code.
function db_delete_feed($feed_id)
{
	$dbh = db_connect();

	// XXX - Call any hooks to be run before the feed is deleted

	/* First, delete all of the articles in the feed */
	$query = "DELETE FROM items WHERE feed_id = ?";
	$stmt = $dbh->prepare($query);
		// XXX - Error-checking
	$err = $stmt->bind_param("i", $feed_id);
		// XXX - Error-checking
	$err = $stmt->execute();
		// XXX - Error-checking
	$stmt->close();

	/* Delete the feed from all groups that it's in. */
	$query = "DELETE FROM group_members WHERE member = ?";
	$stmt = $dbh->prepare($query);
		// XXX - Error-checking
	$err = $stmt->bind_param("i", $feed_id);
		// XXX - Error-checking
	$err = $stmt->execute();
		// XXX - Error-checking
	$stmt->close();

	/* Delete the entry for the feed itself. */
	$query = "DELETE FROM feeds WHERE id = ?";
	$stmt = $dbh->prepare($query);
		// XXX - Error-checking
	$err = $stmt->bind_param("i", $feed_id);
		// XXX - Error-checking
	$err = $stmt->execute();
		// XXX - Error-checking
	$stmt->close();

	return TRUE;
}

/* db_update_feed_info
 * Update the record for a feed in the 'feeds' table. $feed_id is the
 * ID of the feed to update, and $fields is an assoc containing the
 * fields we care about. The keys are column names.
 *
 * Returns the number of columns updated, or FALSE in case of error.
 */
function db_update_feed_info($feed_id, $fields)
{
	$dbh = db_connect();

	/* Get info about existing feed */
	$old = db_get_feed($feed_id);
	if ($old === NULL)
		// No such feed
		return FALSE;

	// XXX - some fields are assumed to be HTML and should be
	// cleaned up before storing:
	//	- title (127)
	//	- subtitle (127)
	//	- nickname (127)
	//	- description (127)
	if (array_key_exists("title", $fields))
		run_hooks("clean-html", array(&$fields['title'], 127));
	if (array_key_exists("subtitle", $fields))
		run_hooks("clean-html", array(&$fields['subtitle'], 127));
	if (array_key_exists("nickname", $fields))
		run_hooks("clean-html", array(&$fields['nickname'], 127));
	if (array_key_exists("description", $fields))
		run_hooks("clean-html", array(&$fields['description'], 127));

	/* Go through the fields, and compare them to what's already
	 * in the database.
	 */
	// Can't use $stmt->bind_param(), because that takes a
	// variable number of arguments, and I don't think PHP has the
	// equivalent of Lisp's (apply).
	// XXX - Actually, see call_user_func_array()

	// Also, some of the fields require special processing anyway:
	// it's not a simple mapping from $feed[X] -> database column
	// X.

	// XXX - Should any of these check to see if the new value is
	// empty, and if so, set the column to NULL?
	$updates = array();		// Array of changes we're
					// going to make, as an array
					// of SQL fragments.
	/* Plain string fields */
	foreach (array('title', 'subtitle', 'nickname', 'url', 'feed_url',
		       'description', 'username', 'passwd')
		 as $f)
	{
		if (array_key_exists($f, $fields) &&
		    $fields[$f] != $old[$f])
		{
			if ($fields[$f] === NULL)
				$value = "NULL";
			else
				$value = "'" .
					$dbh->real_escape_string($fields[$f]) .
					"'";
			$updates[] = "$f = $value";
		}
	}

	/* Boolean fields */
	if (array_key_exists('active', $fields) &&
	    ((bool) $fields['active'] != (bool) $old['active']))
		// Is this feed active?
		$updates[] = "active = " .
			($fields['active'] ? "TRUE" : "FALSE");

	/* Other fields that require special processing */
	if (array_key_exists('image', $fields) &&
	    is_array($fields['image']) &&
	    array_key_exists('url', $fields['image']) &&
	    $fields['image']['url'] != $old['image'])
		// Image associated with the site
		$updates[] = "image = '" .
			$dbh->real_escape_string($fields['image']['url']) .
			"'";

// XXX - This function is called
// a) when the user changes feed information (like title or URL)
// b) when we update a feed.
// We don't want to change last_update in either of those cases; we
// only want to change last_update when an item with a pub_date newer
// than feeds.last_update comes in.
// So this is commented out until I'm sure this doesn't break
// anything.
//	if (array_key_exists('last_update', $fields) &&
//	    $fields['last_update'] > $old['unix_last_update'])
//		// Last update (time_t)
//		$updates[] = sprintf("last_update = FROM_UNIXTIME(%ld)", $fields['last_update']);

	/* Update the database, if necessary */
	if (count($updates) > 0)
	{
		$query = sprintf("UPDATE feeds SET\n%s\nWHERE id=%d",
				 implode(",\n", $updates),
				 $feed_id);
		$result = $dbh->query($query);
		if (!$result)
		{
			global $db_errno;
			$db_errno = $dbh->errno;
			global $db_errmsg;
			$db_errmsg = "Error setting feed information: " .
				$dbh->error;
			return FALSE;
		}
	}

	return count($updates);
}

/* db_update_feed
 * Update a feed. $feed_id is the ID of the feed to update, and $feed
 * is a structure containing all the known information and articles in
 * the feed.
 * Returns an assoc with keys "old", "new", and "updated", giving the
 * number of old, new, and updated items, respectively.
 * In case of error, returns FALSE.
 */
function db_update_feed($feed_id, $feed)
{
	$dbh = db_connect();

	/* Update information about the feed itself */
	db_update_feed_info($feed_id, $feed);

	/* Add/replace the items in the database. */
	if (!isset($feed['items']) ||
	    !is_array($feed['items']) ||
	    count($feed['items']) == 0)
	{
		// XXX - Should delete old items here. But I don't
		// want to duplicate code.
		/* Nothing to do. Return count of old/new/updated
		 * items (all zero)
		 */
		return array("old" => 0,
			     "new" => 0,
			     "updated" => 0);
	}

	$update_counts = array(
		"old"		=> 0,
		"new"		=> 0,
		"updated"	=> 0,
		);
		// Assoc saying how many items were updated, how many
		// are new, how many were already in the database.

	foreach ($feed['items'] as $item)
	{
		/* This query may look long and redundant, but
		 * basically it means:
		 *
		 * If this entry doesn't exist yet, create one. If the
		 * entry already exists, update it from the RSS
		 * information.
		 *
		 * The date fields (look for the IFNULL() calls) work
		 * as follows:
		 *
		 * When adding a new item:
		 * If a publication time was given, use that;
		 * otherwise, use the current time.
		 * If an update time was given, use that; otherwise,
		 * use the publication time.
		 *
		 * When updating an existing item: update pub_date and
		 * last_update to what it says in the feed. If it
		 * wasn't specified, leave the existing field
		 * unchanged.
		 * This is useful because a lot of RSS feeds don't
		 * include timestamps. But we don't want articles
		 * bubbling up to the last time the feed was fetched.
		 *
		 * NB: This code does something that may not be
		 * kosher: the line that sets the last_update field
		 * uses the previously-set value for 'pub_date'. I
		 * don't know whether this is portable or robust; it
		 * just seems to work.
		 */
		/* XXX - Should
		 * INSERT ... (col1, col2, ...) = (value1, value2...)
		 * be changed to
		 * INSERT ... set col1=value1, col2=value2, ...
		 * for readability?
		 */
		/* XXX - Unfortunately, this sends the summary and
		 * content to the database twice, which might be
		 * undesirable for feeds that typically have long
		 * posts. Is there a way around this?
		 */
		/* XXX - The following might happen:
		 * - article is published; pub_date = last_update = t0
		 * - user marks it read at t1
		 * - article is updated; pub_date = t0, last_update = t2
		 * There's now new information that the user might
		 * want to see; ought to mark the article as unread.
		 * Then again, the user might not: the update might be
		 * a trivial modification to fix a typo; or it might
		 * be a liveblog of an ongoing, uninteresting event.
		 *
		 * Perhaps a better compromise might be to use only
		 * the most recent of pub_date and last_update as the
		 * sole publication time. 
		 */
		/* Note that "ON DUPLICATE KEY" is a MySQL-ism */
		/* XXX - What with mtime, it's not obvious that "ON
		 * DUPLICATE KEY" really does everything we want. It
		 * might be better to do a SELECT by (feed_id, guid),
		 * and then either
		 * a) INSERT (if SELECT doesn't find anything), or
		 * b) figure out what needs to change and UPDATE (if
		 * SELECT finds an existing item).
		 *
		 * This would remove the non-standard "ON DUPLICATE
		 * KEY", probably make the code more readable, and
		 * also allow 'mtime' to work properly.
		 */

		$query = <<<EOT
INSERT INTO	items
		(feed_id, url, title, summary, content, author, category,
		 comment_url, comment_rss, guid,
		 pub_date,
		 last_update,
		 is_read)
VALUES		(?,       ?,   ?,     ?,       ?,       ?,      ?,
		 ?,           ?,           ?,
		 IFNULL(FROM_UNIXTIME(?), NOW()),
		 pub_date,
		 IFNULL(?, FALSE))
ON DUPLICATE KEY UPDATE
		url=?,
		title=?,
		summary=?,
		content=?,
		author=?,
		category=?,
		comment_url=?,
		comment_rss=?,
		pub_date=IFNULL(FROM_UNIXTIME(?), pub_date)
EOT;
		// XXX - Ought to update mtime iff any of the fields
		// have changed. Currently, we're setting mtime to
		// NOW() in any case. This means that the item gets
		// updated every time we fetch the RSS feed, until the
		// item drops out of the feed.

		$stmt = $dbh->prepare($query);
		$dummy_categories = NULL;	// XXX - Hack until I
						// figure out how to
						// deal with
						// categories

		/* Construct author to store in database */
		if (isset($item['author_name']))
		{
			$author = $item['author_name'];

			if (isset($item['author_email']))
			{
				$author .= " <$item[author_email]>";
			}
		} elseif (isset($item['author_email']))
			$author = "<$item[author_email]>";

//echo "<pre>\$author [$author]</pre>\n";

		if (isset($item['guid']))
		{
			$guid = $item['guid'];
			if (strlen($guid) > 127)
			{
				# error_log("Hashing overly-long guid $guid");
				$guid = md5($guid);
			}
		} else {
			// This item doesn't have a GUID, so make one.
			// URLs can get arbitrarily long, and all we
			// want is something unique, so just use a
			// hash of the long URL.
			$guid = md5($item['url']);
		}

		/* Something is quite picky about the value of
		 * is_read. Let's be very careful about setting it.
		 */
		if (!isset($item['is_read']))
			$item['is_read'] = false;
		$is_read = 0;
		switch ($item['is_read'])
		{
			case false:
				$is_read = 0;
				break;
			case 1:
				$is_read = 1;
				break;
			default:
error_log("Unknown value for is_read: [" . print_r($item['is_read'], true) . "]");
				break;
		}

		$stmt->bind_param("dsssssssssd" .
				  "ssssssssdi",
				  // Values for new items
				  $feed_id,
				  $item['url'],
				  $item['title'],
				  $item['description'],
				  $item['content'],
				  $author,
				  $dummy_categories,	// XXX - Deal with categories
				  $item['comment_url'],
				  $item['comment_feed'],
				  $guid,
				  $item['pub_time'],
				  $is_read,

				  // Values for updated items
				  $item['url'],
				  $item['title'],
				  $item['description'],
				  $item['content'],
				  $author,
				  $dummy_categories,	// XXX - Deal with categories
				  $item['comment_url'],
				  $item['comment_feed'],
				  $item['pub_time']
			);
		$err = $stmt->execute();
		// XXX - Can count number of affected rows. Apparently
		//	0-> duplicate entry (no change)
		//	1-> new entry
		//	2-> updated entry

		$nrows = $stmt->affected_rows;
		switch ($nrows)
		{
		    case 0:
			$update_counts['old']++;
			break;
		    case 1:
			$update_counts['new']++;
			break;
		    case 2:
			$update_counts['updated']++;
			break;
		    default:
			// This should never happen
			// XXX - Check for this?
			break;
		}

		// XXX - Better error-checking
		if ($err)
			;// echo "OK (", print_r($err), ") errno ", $dbh->errno, ", error [", $dbh->error, "]<br/>\n";
		else {
			error_log("MySQL error in feed $feed_id: " .
				  $dbh->errno . ': "' .
				  $dbh->error . '"');
			// What follows is very ad hoc.
			switch ($dbh->errno)
			{
			    case 1366:
				print_r($item);
				break;
			    case 1406:
				error_log("guid: " . $guid);
				break;
			    default:
				break;
			}
		}
//echo "Affected rows: [", $stmt->affected_rows, "]<br/>\n";
	}

	// Update feeds.last_update, so we can measure staleness
	/* XXX - Edge case: what if the feed is broken (or weird), so
	 * that currently feeds.last_update == Aug 1 (from the last
	 * update), but the most recent items.pub_date is Mar 1? Then
	 * we'd see feeds.last_update go back 4 months, even though
	 * the most recent post was Aug 1.
	 *
	 * Perhaps use
	 * SET last_update = MAX(last_update, (SELECT ...))
	 * But then if the feed is empty, then the IFNULL will default
	 * to NOW(), which is also bad. If the feed is empty, then
	 * feeds.last_update should remain unchanged. So perhaps
	 * should default to the epoch.
	 */
	$query = <<<EOT
UPDATE	feeds
	SET	last_update = 
		(SELECT	IFNULL(MAX(items.pub_date), NOW())
		 FROM	items
		 WHERE	items.feed_id = feeds.id)
	WHERE	id = ?
EOT;
	$stmt = $dbh->prepare($query);
			// XXX - Error-checking
	$err = $stmt->bind_param("i",
				 $feed_id);
			// XXX - Error-checking
	$err = $stmt->execute();
			// XXX - Error-checking

	/* Delete old items from database.
	 * Here, we look up the "autodelete" option for the feed in
	 * question, and delete the items older than that many days.
	 */
	// XXX - This is potentially a problem with
	// a) An article that has gone unread n+1 days, and was just
	// updated. In this case, you had n days to read it. Cope.
	// b) Feeds that don't give dates, such as Perry Bible
	// Fellowship. In this case, the feed parser should insert an
	// artificial date.
	$query = <<<EOT
DELETE FROM	items
WHERE		feed_id = ?
  AND		pub_date < NOW() -
		  INTERVAL get_option(feed_id, "autodelete") DAY
EOT;
	$stmt = $dbh->prepare($query);
	if ($stmt === FALSE)
	{
		// XXX - Better error-checking
		error_log("prepare error " . $dbh->errno .
			": \"" . $dbh->error .
			"\"<br/>\n";
	}
	$err = $stmt->bind_param("i",
				 $feed_id);
		// XXX - Error-checking
	$err = $stmt->execute();
		// XXX - Error-checking

	/* Return count of new/updated items */
	return $update_counts;
}

/* db_get_items
 * Given a list of item IDs, look up those items.
 */
function db_get_items($items)
{
	if (is_array($items) && count($items) == 0)
		# Degenerate case: no items to fetch.
		return array();

	$query = <<<EOT
SELECT	items.id, items.feed_id, items.url, items.title,
	items.summary, items.content,
	items.author, items.category,
	items.comment_url, items.comment_rss, items.guid,
	UNIX_TIMESTAMP(items.pub_date) as `pub_date`,
	UNIX_TIMESTAMP(items.last_update) as `last_update`,
	IF(is_read, 1, 0) as `is_read`,
	UNIX_TIMESTAMP(mtime) as `mtime`
FROM	items
WHERE	id
EOT;

	# We assume that $items is either an integer, or an array of integers.
	if (is_array($items))
		$query .= " in (" . implode(",", $items) . ")";
	else
		$query .= " = $items";

	$dbh = db_connect();
	$result = $dbh->query($query);

	$retval = array();

	while ($row = $result->fetch_assoc())
	{
		# Make sure is_read is properly converted to boolean.
		$row['is_read'] = $row['is_read'] ? TRUE : FALSE;
		$row['mtime'] += 0;	// Make sure it's an integer.
		$retval[] = $row;
	}

	return $retval;
}

/* db_mark_items
 * Mark a set of items as either read or unread.
 * $is_read is a boolean, either true (mark items as read) or false
 * (mark items as unread), and $items is an array of item IDs to mark.
 */
// XXX - Should return status.
// XXX - Would it be better to take an array of
//	item_id => is_read
// values?
function db_mark_items($is_read, $items)
{
	if (count($items) < 1)
		// Nothing to do
		return;

	$dbh = db_connect();

	// XXX - Would it be better to use a prepared statement, and
	// use bind_param() repeatedly to set items' is_read-ness one
	// by one?
	// Time it and see.
	$query = sprintf(<<<EOT
UPDATE	items
SET	is_read=%s,
	mtime=NOW()
WHERE	id IN(%s)
EOT
			 ,
			 ($is_read ? "TRUE" : "FALSE"),
			 implode(",", $items));
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = "Error updating feed information: " . $dbh->error;
	}
}

/*****************************************
 * Group-related functions
 *****************************************/

/* db_get_groups
 * Return a list of feed groups, indexed by group id.
 */
function db_get_groups()
{
	$dbh = db_connect();

	$query = "SELECT * FROM groups";
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}
	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		$retval[$row['id']] = $row;
	}

	// Get the members as well.
	// In each group's entry, 'members' is an array listing the
	// IDs of members of that group. Positive IDs corresond to
	// feeds, and negative IDs correspond to groups.
	$result = $dbh->query("SELECT * FROM group_members");
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	while ($row = $result->fetch_assoc())
	{
		// $row == (member, parent)
		// We take $row[parent] to get the parent, and append to
		// its 'members' array.
		$retval[$row['parent']]['members'][] = $row['member'];
	}

	return $retval;
}

/* db_get_group
 * Return information about a single group. Like db_get_groups(), but
 * for just one group.
 */
function db_get_group($gid)
{
	$dbh = db_connect();

	$query = sprintf("SELECT * FROM groups WHERE id=%d",
			 $gid);
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}
	$retval = $result->fetch_assoc();

	// Get the members as well.
	// In each group's entry, 'members' is an array listing the
	// IDs of members of that group. Positive IDs corresond to
	// feeds, and negative IDs correspond to groups.
	$query = sprintf("SELECT * FROM group_members WHERE parent=%d",
			 $gid);
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	while ($row = $result->fetch_assoc())
	{
		$retval['members'][] = $row['member'];
	}
	return $retval;
}

/* db_feeds_in_group
 * Given a(n integer) gid, return a list of all the feeds that are in
 * that group, including its descendents.
 */
function db_feeds_in_group($gid)
{
	$groups = db_get_groups();

	// XXX - For now, we don't really support nested groups well
	// (there's no interface for nesting groups), so just assume
	// a flat structure, and return the members.
	if (!isset($groups[$gid]))
	{
		return NULL;
	}
	$retval = $groups[$gid]['members'];

	return $retval;
}

/* db_add_group
 * Create a new group. Return its database entry.
 * Default parent is -1, the root group.
 */
function db_add_group($name, $parent = -1)
{
	$dbh = db_connect();

	// Create the group
	$query = "INSERT INTO groups (parent, name) VALUES (?,?)";
	$stmt = $dbh->prepare($query);
	$result = $stmt->bind_param("is", $parent, $name);
	$result = $stmt->execute();
	$stmt->close();
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	// Invert the group ID: auto_increment gives it a positive
	// value, but we want a negative one.
	$result = $dbh->query("UPDATE groups SET id = -id WHERE id = last_insert_id()");
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	// Fetch the group info so we can return it to the caller.
	$result = $dbh->query("SELECT * FROM groups WHERE id = -last_insert_id()");
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}
	$row = $result->fetch_assoc();

	return $row;
}

/* db_update_group_info
 * Similar to db_update_feed_info():
 * $gid is the ID of a group to modify.
 * $fields is an array (hash) of field => new_value pairs.
 */
function db_update_group_info($gid, $fields)
{
	$dbh = db_connect();

	/* Get info about existing feed */
	$old = db_get_group($gid);
	if ($old === NULL)
		// No such feed
		return FALSE;

	$new = $old;
	$updates = array();
	$bind_fmt = "";
	$bind_values = array();

	foreach (array('parent', 'name') as $f)
	{
		if (array_key_exists($f, $fields))
		{
			$new[$f] = $fields[$f];
		}
	}

	$query = "UPDATE groups SET parent=?, name=? WHERE id=?";
	$stmt = $dbh->prepare($query);
	$result = $stmt->bind_param("isi",
		$new['parent'],
		$new['name'],
		$gid);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}
	$result = $stmt->execute();
	$stmt->close();
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}
}

/* db_delete_group
 * Delete the group with id $gid.
 * Returns TRUE if successful. Otherwise, returns NULL and sets $db_errno
 * and $db_errmsg.
 */
function db_delete_group($gid)
{
	global $db_errno;
	global $db_errmsg;

	if ($gid == -1)
	{
		// Can't delete "All".
		$db_errno = 0;	# XXX - Is this kosher?
		$db_errmsg = "Can't delete group \"All\".";
		return FALSE;
	}

	$dbh = db_connect();

	/* Get info about existing feed */
	$old = db_get_group($gid);
	if ($old === NULL)
		// No such feed. I guess we can go home early.
		return TRUE;

	// Move any members of this group to its parent.
	// "IGNORE" says to ignore records where the operation would
	// fail. Thus, if we have
	//	"New York Times" in group "News"
	//	"New York Times" in group "Politics"
	// 	group "Politics" in group "News"
	// we have a feed (New York Times) in both a group (Politics)
	// and its parent (News).
	//
	// If we now delete "Politics", we want to move all members of
	// Politics up one level. So normally we'd wind up with two
	// instances of "New York Times in News". "IGNORE" says to
	// skip those cases. We'll handle them separately, next.
	$query = sprintf("UPDATE IGNORE group_members SET parent=%d WHERE parent=%d",
			 $old['parent'],
			 $gid);
	$result = $dbh->query($query);
	if (!$result)
	{
		$db_errno = $dbh->errno;
		$db_errmsg = $dbh->error;

		return NULL;
	}
	/* Delete any duplicates */
	$query = sprintf("DELETE FROM group_members WHERE parent=%d",
			 $gid);
	$result = $dbh->query($query);
	if (!$result)
	{
		$db_errno = $dbh->errno;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	/* If this group has any child groups, move them up one level
	 * as well, into the parent of $gid.
	 */
	$query = sprintf("UPDATE groups SET parent=%d WHERE parent=%d",
			 $old['parent'],
			 $gid);
	$result = $dbh->query($query);
	if (!$result)
	{
		$db_errno = $dbh->errno;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	/* Delete this group from `groups` */
	$query = sprintf("DELETE FROM groups WHERE id=%d",
			 $gid);
	$result = $dbh->query($query);
	if (!$result)
	{
		$db_errno = $dbh->errno;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	return TRUE;		// Success
}

/* db_group_add_member
 * Add a new member to a group. $member can be positive (a feed ID) or
 * negative (a group ID).
 * XXX - Actually, for now, we want the new member to be positive (a
 * feed).
 */
function db_group_add_member($group, $member)
{
	if ($group == -1)
		// Silently succeed. Everything is part of -1.
		return TRUE;

	$dbh = db_connect();

	$query = "INSERT INTO group_members (member, parent) VALUES (?,?)";
	$stmt = $dbh->prepare($query);
	$result = $stmt->bind_param("ii", $member, $group);
	$result = $stmt->execute();
	$stmt->close();
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	// Success
	return TRUE;
}

/* db_group_remove_member
 * Delete $member from group $gid.
 * Returns TRUE if successful. Otherwise, returns NULL and sets $db_errno
 * and $db_errmsg.
 */
function db_group_remove_member($gid, $member)
{
	$dbh = db_connect();

	$query = sprintf("DELETE FROM group_members where member=%d and parent=%d",
			 $member, $gid);
	$result = $dbh->query($query);
	if (!$result)
	{
		global $db_errno;
		$db_errno = $dbh->errno;
		global $db_errmsg;
		$db_errmsg = $dbh->error;

		return NULL;
	}

	return TRUE;
}

/* db_mark_url
 * Mark an item as read or unread, given the item's URL.
 * $is_read is a boolean, either true (mark item as read) or false
 * (mark item as unread), and $url is the URL of the article
 */
function db_mark_url($is_read, $url)
{
	$dbh = db_connect();
echo "Escaped: [", $dbh->real_escape_string("http://foo%20bar/"), "]\n";
return;

	$query = sprintf(<<<EOT
UPDATE	items
SET	is_read=?,
	mtime=NOW()
WHERE	url=?
EOT
		);
echo "query [$query]\n";
	$stmt = $dbh->prepare($query);
#echo "prepared stmt [$stmt]\n";
	$is_read_sql = $is_read ? "TRUE" : "FALSE";	# Needs to be passed by reference
$err =
	$stmt->bind_param("ss",
			  $is_read_sql,
			  $dbh->real_escape_string($url)
		);
echo "bind_param returned [$err]\n";
# XXX - Is this right?
	$result = $dbh->query($query);
echo "result [$result]\n";
	$err = $stmt->execute();
echo "execute returned [$err]\n";
	$nrows = $stmt->affected_rows;
echo "affected rows: [$nrows]\n";

	// XXX - This only works with the complete URL. But it should
	// also work with internal anchors, both in $url and in the
	// database. That is:
	// - Strip "#.*" from the end of the URL
	// - Strip "#.*" from the URL in the database
	// - Compare the two
	// (We don't want "foo.html#a" to match "foo.htmlXYZ").
}
?>
