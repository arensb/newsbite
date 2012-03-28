<?
/* database.inc
 * Database-related functions. Nothing else should access the database
 * directly.
 */
require_once("config.inc");

$dbh = NULL;		// Database handle

/* db_errno, db_errmsg
 * If one of the functions here returns an error, it should put an
 * error number and message here, so the caller can display something
 * smart.
 */
$db_errno = NULL;	// Error number
$db_errmsg = NULL;	// Error message

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
	$dbh = new mysqli($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);
	if (mysqli_connect_errno())
	{
		printf("Connect failed: %s<br/>\n", mysqli_connect_error());
		exit();
	}

	/* Explicitly set the character set */
	// XXX - There's probably a better way to do this.
	$dbh->set_charset('utf8');

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

	// XXX - Error-checking
	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		$row['active'] = $row['active'] ? TRUE : FALSE;
			# Make sure 'active' is properly boolean
		$retval[$row['id']] = $row;
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
	$row = $result->fetch_assoc();
	return $row;
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
// XXX - Should accept $feed_id == NULL, to count all items.
function db_get_feed_counts($feed_id)
{
	$dbh = db_connect();

	/* Get the number of read/unread items */
	$query = sprintf(<<<EOT
SELECT		IFNULL(SUM(is_read), 0) as `read`,
		IFNULL(SUM(!is_read), 0) as `unread`
FROM		items
WHERE		feed_id = %d
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

	/* Get the number of read/unread items in each feed */
	// XXX - When a feed doesn't have any items, this returns NULL
	// for all the counts. We fix this below with a hack, but
	// there's got to be a better way.
	$query = <<<EOT
SELECT		feeds.id AS `feed_id`,
		IFNULL(SUM(items.is_read), 0) AS `read`,
		IFNULL(SUM(!items.is_read), 0) AS `unread`
FROM		feeds
LEFT JOIN	items
  ON		feeds.id = items.feed_id
GROUP BY	feeds.id
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
 * read => bool
 *	If false, returns only unread items. If true, also returns read
 *	items.
 */
// XXX - Would it be useful to have a keyword for "just give me the
// summary, not the content"?
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
		// XXX - There might be a better way to do this, but I don't
		// know what it is. None of the array functions seem to do the
		// job.
		$args = array();
		for ($i = 0; $i < $argc; $i += 2)
			$args[func_get_arg($i)] = func_get_arg($i+1);
	}

	/* Beginning of query */
	# XXX - Ought to return dates in ISO-8601 ("2011-11-23T10:09:17Z")
	# or time_t format.
	$query = <<<EOT
SELECT	items.*
FROM	items
  LEFT JOIN feeds
ON	items.feed_id = feeds.id

EOT;

	$conditions = array();	// All of the "WHERE x AND y AND z" conditions

	/* Were we asked for just one feed ID? */
	// XXX - Would be nice to request some, but not all, feeds, by
	// listing their IDs.
	if (isset($args['feed_id']) && is_numeric($args['feed_id']))
	{
		$c = sprintf("items.feed_id=%d\n", $args['feed_id']);
		array_push($conditions, $c);
	} else {
		array_push($conditions, "feeds.active");
	}

	/* Do we want read items, or just unread ones? */
	// Without an explicit condition, SQL will return both read
	// and unread items, but that's not the default behavior we
	// want. So we really test for !($args['read'] == true)
	/* XXX - Ought to be able to select "give me only read items",
	 * "give me only unread items", or "give me both".
	 */
	if (!isset($args['read']) || !$args['read'])
		array_push($conditions, "!items.is_read");

	/* Add selection conditions to query */
	if (count($conditions) > 0)
	{
		$query .= "WHERE " . implode("\nAND ", $conditions) . "\n";
	}

	/* Specify order in query.
	 * Use id as a tiebreaker, to ensure a consistent order.
	 */
	if (!isset($args['order']))
		$args['order'] = "date_desc";	// Stop PHP from complaining
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

	/* Add limit and offset to query */
	if (is_int($args['max_items']))
		$query .= "LIMIT " . $args['max_items'] . "\n";
	else
		// XXX - Should probably put the default limit in a
		// constant.
		$query .= "LIMIT 50\n";

	if (is_int($args['start_at']))
		$query .= "OFFSET " . $args['start_at'] . "\n";

	/* Run the SQL query */
	$dbh = db_connect();
	$result = $dbh->query($query);
			// XXX - Error-checking
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

		    case "ttl":
			// Integer value
			$fields[] = "ttl";
			$values[] = $v;
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
	$err = $stmt->bind_param("d", $feed_id);
		// XXX - Error-checking
	$err = $stmt->execute();
		// XXX - Error-checking
	$stmt->close();

	/* Next, delete the entry for the feed itself. */
	$query = "DELETE FROM feeds WHERE id = ?";
	$stmt = $dbh->prepare($query);
		// XXX - Error-checking
	$err = $stmt->bind_param("d", $feed_id);
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

	if (array_key_exists('ttl', $fields) &&
	    $fields['ttl'] != $old['ttl'])
		// Time to live (seconds)
		$updates[] = sprintf("ttl = %ld", $fields['ttl']);

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

		if (!isset($item['guid']))
		{
			// This item doesn't have a GUID, so make one.
			$item['guid'] = $item['url'];
		}

		if (!isset($item['is_read']))
			$item['is_read'] = false;
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
				  $item['guid'],
				  $item['pub_time'],
				  $item['is_read'],

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
		else
			;// echo "<b>Error: ", $dbh->errno, ": \"", $dbh->error, "\"</b><br/>\n";
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
	$err = $stmt->bind_param("d",
				 $feed_id);
			// XXX - Error-checking
	$err = $stmt->execute();
			// XXX - Error-checking

	/* Delete old items from database */
	// XXX - This is potentially a problem with
	// a) An article that has gone unread n+1 days, and was just
	// updated. In this case, you had n days to read it. Cope.
	// b) Feeds that don't give dates, such as Perry Bible
	// Fellowship. In this case, the feed parser should insert an
	// artificial date.
	global $AUTODELETE_DAYS;

	if ($AUTODELETE_DAYS != 0)
	{
		$query = <<<EOT
DELETE FROM	items
WHERE		feed_id = ?
  AND		pub_date < NOW() - INTERVAL ? DAY
EOT;
		$stmt = $dbh->prepare($query);
			// XXX - Error-checking
		$err = $stmt->bind_param("dd",
				  $feed_id,
				  $AUTODELETE_DAYS);
			// XXX - Error-checking
		$err = $stmt->execute();
			// XXX - Error-checking
	}

	/* Return count of new/updated items */
	return $update_counts;
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
SET	is_read=%s
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
SET	is_read=?
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
