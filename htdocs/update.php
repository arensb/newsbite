<?
require_once("config.php");
require_once("database.inc");
require_once("rss.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$feed_id = $_REQUEST["id"];
# XXX - Error-checking: make sure '$feed_is' is an integer.

if (is_numeric($feed_id) && is_int($feed_id+0))
	update_feed($feed_id);
elseif ($feed_id == "all")
	update_all_feeds();
else {
	// XXX - Abort with an error message.
	echo "Invalid feed id: [$feed_id]\n";
}

// XXX - Put everything that follows in a single 'update_feed()' function
/* update_feed
 * Update a feed: fetch the RSS, parse it, and add/update items in the
 * database.
 */
function update_feed($feed_id)
{
echo "Inside update_feed($feed_id)<br/>\n";
	/* Get the feed from the database */
	$feed = db_get_feed($feed_id);
	if (!$feed)
	{
		// XXX - Better error-reporting
		echo "No such feed: $feed_id<br/>\n";
		exit(1);
	}
echo "<h3>Updating feed [$feed[title]]</h3>\n";
#print_r($feed);

	/* Initialize Curl */
	// XXX - Use curl_multi_*() to fetch multiple URLs at once.
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $feed['feed_url']);
		// Set the URL
	curl_setopt($ch, CURLOPT_HEADER, FALSE);
		// Don't give me the header
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		// Give me the result; don't print it.
	curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
		// Follow redirects, up to 2. Beyond that isbeing
		// unreasonable.
	curl_setopt($ch, CURLOPT_MAXREDIRS, 2);

	// Use authentication if required
	if (isset($feed['username']) || isset($feed['passwd']))
	{
		curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANYSAFE);
			// Use reasonable security
		curl_setopt($ch, CURLOPT_USERPWD, "$feed[username]:$feed[passwd]");
	}

	/* Fetch the RSS feed */
	$feed_text = curl_exec($ch);

	// Check for curl errors
	$err = curl_error($ch);
	if ($feed_text === false)
	{
		// XXX - Better error-reporting
		echo "Curl error [", curl_errno($ch), "]: ", htmlspecialchars(curl_error($ch)), "\n";
		curl_close($ch);
		return false;
	}

	// Check the HTTP return code
	$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
	if ($http_code != 200)
	{
		// XXX - Better error-reporting
		echo "HTTP error [$http_code], feed_text [[$feed_text]]\n";
		curl_close($ch);
		exit(2);
	}

	curl_close($ch);

	/* Parse the feed */
	$feed = parse_feed($feed_text);

	// XXX - Delete old items (> 90 days?) from feed

	/* Add/replace the items in the database. */
	$sth = db_connect();
	foreach ($feed['items'] as $item)
	{
echo "Need to update item: [$item[title]]<br/>\n";
#echo "<pre>\$item: ["; print_r($item); echo "]\n</pre>\n<br/>\n";

		// This query may look long and redundant, but
		// basically it means:

		// If this entry doesn't exist yet, create one, and set the
		// state to 'new'. If the entry already exists, update it from
		// the RSS information, and set the state to 'updated' if
		// necessary.
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
		if (isset($item['build_time']))
			$build_time = $item['build_time'];
		else
			$build_time = time();

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
				  $item['pub_time'],
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
				  $item['pub_time'],
				  $build_time
			);
		$err = $stmt->execute();
if ($err != 11)
echo "<b>stmt-&gt;execute returned [$err]</b><br/>\n";
		if ($err)
			echo "OK (", print_r($err), ") errno ", $sth->errno, ", error [", $sth->error, "]<br/>\n";
		else
			echo "<b>Error: ", $sth->errno, ": \"", $sth->error, "\"</b><br/>\n";
	}
}

function update_all_feeds()
{
	$feeds = db_get_feeds();
	// XXX - Error-checking

	foreach ($feeds as $f)
		update_feed($f['id']);
		// XXX - Error-checking
}
?>
