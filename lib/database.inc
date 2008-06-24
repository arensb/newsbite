<?
require_once("config.inc");

$dbh = NULL;		# Database handle

/* db_connect
 * Connect to the database if necessary. Return a handle to the
 * database.
 */
function db_connect()
{
	global $dbh;
	global $DB_HOST, $DB_USER, $DB_PASS, $DB_NAME;

	# If there's already a database handle, use that.
	if (isset($dbh))
		return $dbh;

	# Create a new connection
	$dbh = new mysqli($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);
	if (mysqli_connect_errno())
	{
		printf("Connect failed: %s<br/>\n", mysqli_connect_error());
		exit();
	}
	return $dbh;
}

/* db_disconnect
 * Disconnect from the database.
 */
function db_disconnect()
{
	global $dbh;

	if (!isset($dbh))
		# Already disconnected.
		return;

	$dbh->close();
	$dbh = NULL;
	return;
}

/* db_get_feeds
 * Return a list of all feeds.
 */
function db_get_feeds()
{
	$dbh = db_connect();

	$query = <<<EOT
SELECT	*
FROM	feeds
EOT;

	$result = $dbh->query($query);
	if (!$result)
	{
		echo "Error ", $dbh->errno, ": \"",
			$dbh->error, "\"<br/>\n";
		return NULL;
	}

	// XXX - Error-checking
	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		$retval[] = $row;
	}
	return $retval;
}

/* db_get_feed
 * Fetch the database record for a single feed, specified by its ID.
 */
function db_get_feed($id)
{
	$dbh = db_connect();

	$query = sprintf(<<<EOT
SELECT	*
FROM	feeds
WHERE	id = %d
EOT
		,
		$id);
	$result = $dbh->query($query);
	$row = $result->fetch_assoc();
	return $row;
}

/* db_get_feed_counts
 * Get the number of new/read/unread articles in a feed.
 *
 * Returns an array of the form
 *	array(
 *		'new'	=> 10,
 *		'read'	=> 150,
 *		'unread'=> 12,
 *	);
 */
function db_get_feed_counts($feed_id)
{
	$dbh = db_connect();

	$query = sprintf(<<<EOT
SELECT		SUM(state='new') as `new`,
		SUM(state='unread') as `unread`,
		SUM(state='read') `read`,
		SUM(state='updated') `updated`,
		SUM(state='deleted') as `deleted`
FROM		items
WHERE		feed_id = %d
GROUP BY	feed_id
EOT
			 ,
			 $feed_id);
	$result = $dbh->query($query);
	if (!$result)
	{
		echo "Error ", $dbh->errno, ": \"",
			$dbh->error, "\"<br/>\n";
		return NULL;
	}

	// XXX - Error-checking
	$row = $result->fetch_assoc();
	return $row;
}

/* db_get_all_feed_counts
 * Get the number of new/read/unread articles in all feeds.
 *
 * Returns an array of the form
 *	array(
 *		190	=> array(		// Feed ID 190
 *			'new'	=> 10,
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

	// XXX - When a feed doesn't have any items, this returns NULL
	// for all the counts. We fix this below with a hack, but
	// there's got to be a better way.
	$query = <<<EOT
SELECT		feeds.id as `feed_id`,
		SUM(items.state='new') as `new`,
		SUM(items.state='unread') as `unread`,
		SUM(items.state='read') `read`,
		SUM(items.state='updated') `updated`,
		SUM(items.state='deleted') as `deleted`
FROM		feeds
LEFT JOIN	items
  ON		feeds.id=items.feed_id
GROUP BY	feeds.id
EOT;
	$result = $dbh->query($query);
	if (!$result)
	{
		echo "Error ", $dbh->errno, ": \"",
			$dbh->error, "\"<br/>\n";
		return NULL;
	}

	// XXX - Error-checking
	$retval = array();
	while ($row = $result->fetch_assoc())
	{
		// XXX - Hack to make sure that NULL values come out as 0.
		// I'm sure there's a better way to do this.
		foreach ($row as $k => $v)
			if ($v == NULL)
				$row[$k] = 0;

		$feed_id = $row['feed_id'];
		unset($row['feed_id']);
		$retval[$feed_id] = $row;
	}
	return $retval;
}

/* db_get_feed_items
 * Fetch the items from the database for a given feed.
 * If $feed is an integer, it's a feed ID. Retrieve both the 'feeds'
 * entry and the items. Otherwise, assume it's an assoc (as returned
 * by db_get_feed()) and just fill in the items.
 */
// XXX - Should take arguments to limit what is returned, e.g., only
// latest 50 items, only new/unread items, etc.
function db_get_feed_items(&$feed)
{
	/* Figure out whether we need to get the 'feeds' entry */
	if (is_numeric($feed) && is_int($feed))
	{
		$tmp = db_get_feed($feed['id']);
		if (!$tmp)
			return NULL;
		$feed = $tmp;
	}
	$feed['items'] = array();	// Clear list of items.

	/* Get the feed items */
	$dbh = db_connect();

	$query = sprintf(<<<EOT
SELECT	*
FROM	items
WHERE	feed_id=%d
ORDER BY	pub_date DESC
LIMIT	50
EOT
			 ,
			 $feed['id']);
	$result = $dbh->query($query);
	while ($row = $result->fetch_assoc())
	{
		$feed['items'][] = $row;
	}

	return $feed;
}

/* db_get_some_feed_items
 * Fetch items from the database that meet certain criteria.
 * The criteria are specified by the variable argument list: the list
 * is a list of {keyword, value} pairs.
 * !!! Note that unlike db_get_feed_items(), this just returns a list
 * of items. It doesn't fill in the 'items' field in a feed list.
 *
 * feed_id => int id
 *	Which feed's items to fetch. If missing, fetch from all feeds
 * start_at => int n
 *	Skip the first n items.
 * max_items => int n
 *	How many items to fetch.
 * order => date_desc (default) | date_asc
 *	Order in which to return the items
 * states => string
 *	Comma-separated list of states. We only want to see items in one
 *	of these states. Example:
 *		db_get_some_feed_items(
 *			'states', 'new,unread'
 *			);
 */
function db_get_some_feed_items(/* varargs */)
{
	$argc = func_num_args();

	/* Sanity check: make sure we got an even number of arguments */
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

	/* Beginning of query */
	$query = "SELECT * FROM items\n";

	$conditions = array();	// All of the "WHERE x AND y AND z" conditions

	/* Were we asked for just one feed ID? */
	if (is_numeric($args['feed_id']))
	{
		$c = sprintf("feed_id=%d\n", $args['feed_id']);
		array_push($conditions, $c);
	}

	/* Which states does caller want? */
	// XXX - Presumably we can trust the caller, and don't need to
	// sanitize input.
	if (isset($args['states']))
	{
		$states = explode(",", $args['states']);
		$c = sprintf("state IN('%s')\n",
			     implode("','", $states));
		array_push($conditions, $c);
	}

	/* Add selection conditions to query */
	if (count($conditions) > 0)
	{
		$query .= "WHERE " . implode("\nAND ", $conditions) . "\n";
	}

	/* Specify order in query */
	switch ($args['order'])
	{
	    case "date_asc":
		$query .= "ORDER BY pub_date ASC\n";
		break;
	    case "date_desc":
	    default:
		// XXX - This fall-through means we don't notice bogus
		// values
		$query .= "ORDER BY pub_date DESC\n";
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
		$retval[] = $row;
	}

	return $retval;
}

/* db_update_feed
 * Update a feed. $feed_id is the ID of the feed to update, and $feed
 * is a structure containing all the known information and articles in
 * the feed.
 */
function db_update_feed($feed_id, $feed)
{
	/* Delete old items from database */
	// XXX - This should probably go at the end: otherwise, with
	// feeds that have items older than 90 days, you can have:
	// Monday, 9:00: refresh, get new and old items.
	// Monday, 10:00: mark old items as read.
	// Tuesday, 9:00: refresh:
	//	Old items deleted
	//	Items added from feed
	// As a result, messages that were posted > 90 days ago, but
	// are still in the feed, show up as new.
	$sth = db_connect();

	global $AUTODELETE_DAYS;
	if ($AUTODELETE_DAYS != 0)
	{
		$query = <<<EOT
DELETE FROM	items
WHERE		feed_id = ?
  AND		last_update < NOW() - INTERVAL ? DAY
EOT;
		$stmt = $sth->prepare($query);
			// XXX - Error-checking
		$err = $stmt->bind_param("dd",
				  $feed_id,
				  $AUTODELETE_DAYS);
			// XXX - Error-checking
		$err = $stmt->execute();
			// XXX - Error-checking
echo "Deleted [", $stmt->affected_rows, "] items.<br/>\n";
	}

	/* Update information about the feed itself */
	// I don't think we can use $stmt->bind_param(), because that
	// takes a variable number of arguments, and I don't think PHP
	// has the equivalent of Lisp's (apply).

	// At the same time, some of the fields require special
	// processing anyway: it's not a simple mapping from
	// $feed[X] -> database column X.
	$updates = array();
	if (isset($feed['title']))
		// Feed title
		$updates[] = "title = '" .
			$sth->real_escape_string($feed['title']) .
			"'";
	if (isset($feed['subtitle']))
		// Feed subtitle
		$updates[] = "subtitle = '" .
			$sth->real_escape_string($feed['subtitle']) .
			"'";
	if (isset($feed['url']))
		// URL for the site
		$updates[] = "url = '" .
			$sth->real_escape_string($feed['url']) .
			"'";
	if (isset($feed['description']))
		// Description of the site
		$updates[] = "description = '" .
			$sth->real_escape_string($feed['description']) .
			"'";
	if (isset($feed['ttl']))
		// Description of the site
		$updates[] = sprintf("ttl = %ld", $feed['description']);
	if (isset($feed['image']['url']))
		// Image associated with the site
		$updates[] = "image = '" .
			$sth->real_escape_string($feed['image']['url']) .
			"'";
	if (count($updates) > 0)
	{
		$query = sprintf("UPDATE feeds SET\n%s\nWHERE id=%d",
				 implode(",\n", $updates),
				 $feed_id);
		$result = $sth->query($query);
		if (!$result)
		{
			echo "Error updating feed information: ",
				$sth->errno, ": \"",
				$sth->error, "\"<br/>\n";
		}
	}

	/* Add/replace the items in the database. */
	if (count($feed['items']) == 0)
		/* Nothing to do */
		// XXX - Return something sensible?
		return;

	foreach ($feed['items'] as $item)
	{
echo "Need to update item: [$item[title]]<br/>\n";
//echo "<pre>\$item: ["; print_r($item); echo "]\n</pre>\n<br/>\n";

		// This query may look long and redundant, but
		// basically it means:

		// If this entry doesn't exist yet, create one, and set the
		// state to 'new'. If the entry already exists, update it from
		// the RSS information, and set the state to 'updated' if
		// necessary.

		// XXX - Unfortunately, this sends the summary and
		// content to the database twice, which might be
		// undesirable for feeds that typically have long
		// posts. Is there a way around this?
		$query = <<<EOT
INSERT INTO	items
		(feed_id, url, title, summary, content, author, category,
		 comment_url, comment_rss, guid, pub_date,
		 last_update, state)
VALUES		(?, ?, ?, ?, ?, ?, ?,
		 ?, ?, ?, FROM_UNIXTIME(?),
		 FROM_UNIXTIME(?), 'new')
ON DUPLICATE KEY UPDATE
		url=?,
		title=?,
		summary=?,
		content=?,
		author=?,
		category=?,
		comment_url=?,
		comment_rss=?,
		pub_date=FROM_UNIXTIME(?),
		last_update=FROM_UNIXTIME(?),
		state=IF(state='read' or state='unread',
			 'updated', state)
EOT;
		$stmt = $sth->prepare($query);
		$dummy_categories = NULL;	// XXX - Hack until I
						// figure out how to
						// deal with
						// categories
		if (!isset($item['guid']))
		{
			// This item doesn't have a GUID, so make one.
			$item['guid'] = $item['url'];
		}
		if (isset($item['build_time']))
			$build_time = $item['build_time'];
		else
			$build_time = time();

		/* Make sure the item has a publication time. If the
		 * item doesn't have one, try to inherit from the feed
		 * itself. If that fails, use the current time.
		 */

		$pub_time = $item['pub_time'];
		if ($pub_time == 0)
		{
			if ($feed['pub_time'] != 0)
			{
				$pub_time = $feed['pub_time'];
			} else {
				$pub_time = time();
			}
		}

		$stmt->bind_param("dsssssssssdd" .
				  "ssssssssdd",
				  // Values for new items
				  $feed_id,
				  $item['url'],
				  $item['title'],
				  $item['description'],
				  $item['content'],
				  $item['author_email'],	// XXX - Can do better?
				  $dummy_categories,	// XXX - Deal with categories
				  $item['comment_url'],
				  $item['comment_feed'],
				  $item['guid'],
				  $pub_time,
				  $build_time,
				  // 'state' set automatically

				  // Values for updated items
				  $item['url'],
				  $item['title'],
				  $item['description'],
				  $item['content'],
				  $item['author_email'],	// XXX - Can do better?
				  $dummy_categories,	// XXX - Deal with categories
				  $item['comment_url'],
				  $item['comment_feed'],
				  $pub_time,
				  $build_time
			);
		$err = $stmt->execute();
		// XXX - Can count number of affected rows. Apparently
		//	0-> duplicate entry (no change)
		//	1-> new entry
		//	2-> updated entry
if ($err != 11)
echo "<b>stmt-&gt;execute returned [$err]</b><br/>\n";
		if ($err)
			echo "OK (", print_r($err), ") errno ", $sth->errno, ", error [", $sth->error, "]<br/>\n";
		else
			echo "<b>Error: ", $sth->errno, ": \"", $sth->error, "\"</b><br/>\n";
echo "Affected rows: [", $stmt->affected_rows, "]<br/>\n";
	}
}

/* db_mark_items
 * Change the state of a set of items. $state is one of "new",
 * "unread", or "read" (or potentially any other state that
 * items.state might have in the database), and $items is an array of
 * item IDs to mark.
 */
function db_mark_items($state, $items)
{
	if (count($items) < 1)
		// Nothing to do
		return;

	$dbh = db_connect();

	/* XXX - Ought to check that $state is a valid state */

	$query = sprintf(<<<EOT
UPDATE	items
SET	state='%s'
WHERE	id IN(%s)
EOT
			 ,
			 $state,
			 implode(",", $items));
	$result = $dbh->query($query);
	if (!$result)
	{
		echo "Error updating feed information: ",
			$sth->errno, ": \"",
			$sth->error, "\"<br/>\n";
	}
}
?>
