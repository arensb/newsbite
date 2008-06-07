<?
require_once("config.php");

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
// XXX - $feed should probably be passed by reference.
function db_get_feed_items($feed)
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
?>
