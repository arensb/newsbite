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

	# XXX - Error-checking
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

/* db_update_feed
 * Update a feed. $feed_id is the ID of the feed to update, and $feed
 * is a structure containing all the known information and articles in
 * the feed.
 */
function db_update_feed($feed_id, $feed)
{
	// XXX - Ought to update the feed itself, e.g., if its title
	// changes.

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

	/* Add/replace the items in the database. */
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
		if ($item['pub_time'] == 0)
		{
			if ($feed['pub_time'] != 0)
				$pub_time = $feed['pub_time'];
			else
				$pub_time = time();
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
?>
