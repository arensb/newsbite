<?
require_once("config.inc");
require_once("database.inc");
require_once("feed.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$feed_id = $_REQUEST["id"];

if (is_numeric($feed_id) && is_int($feed_id+0))
	update_feed($feed_id);
elseif ($feed_id == "all")
	update_all_feeds();
else {
	// XXX - Abort with an error message.
	echo "Invalid feed id: [$feed_id]\n";
}

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

	db_update_feed($feed_id, $feed);
	// XXX - Error-checking
}

/* update_all_feeds
 * As the name implies, this updates all the feeds listed in the
 * database.
 */
// XXX - This ought to use curl_multi_* to update multiple feeds in
// parallel.
// XXX - Ought to try to avoid refreshing feeds too often. Add a bool
// $force argument to force an update.
function update_all_feeds()
{
	global $PARALLEL_UPDATES;

	$feeds = db_get_feeds();
	// XXX - Error-checking

	/* XXX - Hack: the code below doesn't work if
	 * $PARALLEL_UPDATES is set to 1. If someone tries to disable
	 * parallel updates by setting it to 1, oblige them by doing
	 * it the naive way.
	 */
	if ($PARALLEL_UPDATES <= 1)
	{
		foreach ($feeds as $f)
			update_feed($f['id']);
		// XXX - Error-checking
	}

	/* Initialize curl_multi and some variables */
	$mh = curl_multi_init();	// Curl multi handle
	$pipeline = array();		// What's currently being fetched

	/* Seed the pipeline with the first $PARALLEL_UPDATES feeds */
	reset($feeds);			// So each() will work
	for ($i = 0; $i < $PARALLEL_UPDATES; $i++)
	{
		if (!(list($idx, $feed) = each($feeds)))
			// We don't have enough feeds to fill a pipeline.
			break;

		$url = $feed['feed_url'];
echo "Starting [", $feed['title'], "]<br/>\n"; flush();
		$ch = _open_curl_handle(	// Curl handle for this URL
			$url,
			$feed['username'],
			$feed['passwd']);
		$err = curl_multi_add_handle($mh, $ch);

		// Keep track of this feed handle
		$pipeline[$i] = array(
			"ch"	=> $ch,
			"index"	=> $idx,	// Index into $feeds
			"feed"	=> $feed	// Reference to the feed
			);
	}
//echo "Pipeline: ["; print_r($pipeline); echo "]\n";

	/* Tell curl_multi to process what we've given it so far */
	$active = NULL;		// Number of active connections, according
				// to curl_multi_exec()
	$exec_stat = NULL;	// Return status from curl_multi_exec

	do {
		$exec_stat = curl_multi_exec($mh, $active);
	} while ($exec_stat == CURLM_CALL_MULTI_PERFORM);
			// CURLM_CALL_MULTI_PERFORM basically means "I
			// have something else I need to do,
			// preferably immediately. So loop until
			// there's nothing more to do.

	/* Main loop */
	while ($active != 0 && $exec_stat == CURLM_OK)
	{
		/* Wait for something to happen */
		$err = curl_multi_select($mh);
		// curl_multi_select() is a wrapper: it assembles the
		// list of file descriptors in $mh by calling
		// curl_multi_fdset(), then select()s them, and
		// returns the result (the number of ready file
		// descriptors, or -1 in case of error, or 0 in case
		// of timeout).
		// Basically, this waits for something interesting to
		// happen, and prevents busy-waiting on
		// curl_multi_exec().
		if ($err == -1)
			// An error of some kind (from select())
			// XXX - Probably ought to figure out what happened
			continue;

		/* Check to see whether any transfers have completed */
		$in_queue = NULL;
		while ($err = curl_multi_info_read($mh, $in_queue))
		{
//echo "curl_multi_info_read returned ["; print_r($err); echo "], in_queue ($in_queue)\n";
			// XXX - The CURLMSG_DONE test isn't
			// particularly useful, since it's the only
			// status that curl_multi_info_read() returns.
			if ($err['msg'] != CURLMSG_DONE)
			{
				echo "    Warning: curl_multi_info_read() returned [";
				print_r($err);
				echo "], and I don't know how to handle that.\n";
				break;
			}

			// Find the appropriate handle, and set $handle as a
			// reference to its entry in $pipeline.
			unset($handle);		// Reset any previous connection
			$handle = NULL;
			for ($i = 0; $i < count($pipeline); $i++)
			{
//echo "Checking [", $pipeline[$i]['ch'], "] vs. [", $err['handle'], "]\n";
				if ($pipeline[$i]['ch'] == $err['handle'])
				{
					$handle = &$pipeline[$i];
						// Note: reference to
						// $pipeline[$i], not copy.
//echo "Now handle == ["; print_r($handle); echo "]\n";
					break;
				}
			}
			if (!isset($handle))
			{
				// Hopefully this will never happen
				echo "Error: couldn't find curl handle ",
					$err['handle'], " in pipeline\n";
				continue;
			}

			/* Check whether the transfer completed successfully
			 * 0 - OK
			 * 7 - Couldn't connect
			 * 47 - Too many redirects
			 */
			if ($err['result'] == CURLE_OK)
			{
				/* We've found the handle we
				 * want. Get its contents.
				 */
echo "Finished [", $handle['feed']['title'], "]<br/>\n"; flush();
				_save_handle($handle);

				/* We're done with this handle. */
				curl_multi_remove_handle(
					$mh,
					$handle['ch']);
				curl_close($handle['ch']);
			} else {
				// XXX - Better error-reporting
				echo "Error in [", $handle['feed']['title'],
					"]: ",
					curl_errno($handle['ch']),
					": [",
					curl_error($handle['ch']),
					"]\n";
			}

			/* See if there's another URL to fetch */
			if (list ($idx, $feed) = each($feeds))
			{
				// Yes. Open a Curl handle, and add it
				// to the multi-handle
echo "Starting [", $feed['title'], "]<br/>\n"; flush();
				$url = $feed['feed_url'];
				$ch = _open_curl_handle(
					$url,
					$feed['username'],
					$feed['passwd']);

				$err = curl_multi_add_handle($mh, $ch);
				$handle = array(
					"ch"	=> $ch,
					"index"	=> $idx,
					"feed"	=> $feed
					);
			} else {
				// We have no more feeds to update.
				// Mark this pipeline entry as NULL.
				$handle = NULL;
			}
		}

		/* Tell curl_multi to do its thing, as long as it has
		 * something else it wants to do right away.
		 */
		do {
			$exec_stat = curl_multi_exec($mh, $active);
		} while ($exec_stat == CURLM_CALL_MULTI_PERFORM);
	}
	/* End of main loop */

	/* Clean up any remaining handles. Normally, this is just the
	 * last handle that was closed. It wasn't caught by the code
	 * above.
	 */
	// XXX - Can't figure out how to deal with the last handle in
	// the code above. Hence this special addition at the end.
	for ($i = 0; $i < count($pipeline); $i++)
	{
		if (isset($pipeline[$i]))
		{
echo "Finished [", $pipeline[$i]['feed']['title'], "]<br/>\n"; flush();
			_save_handle($pipeline[$i]);
		}
	}

	curl_multi_close($mh);
}

/* _open_curl_handle
 * Private helper function to open a Curl handle to retrieve a URL,
 * and set the options we want.
 */
function _open_curl_handle($url, $username = NULL, $passwd = NULL)
{
	$ch = curl_init();
//	echo "  \$ch == ["; print_r($ch); echo "]\n";

	$err = curl_setopt_array(
		$ch,
		array(CURLOPT_URL	=> $url,	// Set the URL
		      CURLOPT_HEADER	=> TRUE,
				// Give me the header, so I can see
				// the HTTP status code
		      CURLOPT_RETURNTRANSFER => true,
				// Give me the result; don't print it.
		      // Follow up to 2 redirects. Beyond that, it's
		      // being unreasonable.
		      CURLOPT_FOLLOWLOCATION => true,	// Follow redirects
		      CURLOPT_MAXREDIRS	=> 2));
				// Max # redirects to follow

	/* Use authentication if required */
	if ($username != "" || $passwd != "")
	{
		curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANYSAFE);
			// Use reasonable security
		curl_setopt($ch, CURLOPT_USERPWD, "$feed[username]:$feed[passwd]");
	}

//echo "_open_curl_handle returning [$ch]\n";
	return $ch;
}

function _save_handle($handle)
{
//echo "_save_handle("; print_r($handle); echo ")\n";
	// Get the feed text from the curl handle
	$feed_text = curl_multi_getcontent($handle['ch']);
//echo "feed_text == [", substr($feed_text, 0, 512), "]\n";

	/* XXX - Get the HTTP header(s), for the status code, so we
	 * can find out whether something went wrong.
	 * A header is a set of CR-LF-terminated lines of the form
	 *	HTTP/<version> <code> <msg>
	 *	another line
	 *	yet another line
	 *	\r
	 * (IOW look for the first blank line (but ending with \r\n,
	 * not \n).
	 *
	 * For redirects, there'll be multiple headers: the first one
	 * gives the redirect, the next one gives the real location.
	 * This can be useful for telling the user if the feed URL has
	 * changed.
	 * XXX - OTOH, sometimes the redirect is intentional, and
	 * should be ignored (e.g., feeds at scienceblogs.com and
	 * other sites are apparently subcontracted to feedburner.com)
	 */
	// We only care about the last header.
	$http_status = -1;
	$http_error = "This error message intentionally left blank.";
	do {
		list($new_header, $new_text) =
			explode("\r\n\r\n", $feed_text, 2);
		$header = $new_header;
		$feed_text = $new_text;

		/* Dig the HTTP status code out of the header */
		if (preg_match('{^HTTP/\S+\s+(\d+)\s+(.*?)\r}',
			       $header,
			       $match))
		{
			$http_status = $match[1];
			$http_error  = $match[2];
			// XXX - Possible statuses:
			// 200 OK
			// 301 Moved permanently (followed by another header)
			// 302 Moved temporarily (followed by another header)
			// 401 Authentication required (followed by another header)
		}
	} while (substr($feed_text, 0, 5) == "HTTP/");

//echo "Final status: [$http_status] [$http_error]\n";
	if ($http_status != "200")
	{
		// XXX - Better error-reporting
		echo "Error: $http_status $http_error\nAborting.\n";
		return;
	}

	// XXX - Check the status code, and complain if there's an
	// error.

	/* Parse the feed */
	$feed_id = $handle['feed']['id'];
//echo "feed_id == [$feed_id]\n";
	$feed = parse_feed($feed_text);
	if (!$feed)
		// XXX - Better error-handling
		return;

	/* Add the feed to the database */
	db_update_feed($feed_id, $feed);
}
?>
