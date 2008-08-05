<?
/* update.php
 * Update one feed, or all feeds.
 */
// XXX - Move the guts of this file to a separate .inc file, so that a
// newly-added feed can be automatically updated.
require_once("config.inc");
require_once("common.inc");
require_once("database.inc");
require_once("feed.inc");

/* See what kind of output the user wants */
switch ($_REQUEST['o'])
{
    case "json":
	$out_fmt = "json";
	header("Content-type: text/plain; charset=utf-8");
	break;
    default:
	header("Content-type: text/html; charset=utf-8");
	$out_fmt = "html";
	break;
}

$feed_id = $_REQUEST["id"];

if (is_numeric($feed_id) && is_int($feed_id+0))
{
	update_feed($feed_id);

	// XXX - Prettier output
	if ($out_fmt == "html")
		echo "<p><a href=\"view.php?id=$feed_id\">Read feed</a></p>\n";
} elseif ($feed_id == "all")
{
	update_all_feeds();

	// XXX - Prettier output
	if ($out_fmt == "html")
		echo "<p><a href=\"view.php?id=$feed_id\">Read feeds</a></p>\n";
} else {
	/* Abort with an error message */
	abort("Invalid feed ID: $feed_id");
}

// XXX - Move the HTTP-fetching stuff to a separate function, so that
// we can retrieve URLs separately.
//
// Need some sort of callback mechanism, so that caller can decide
// what to do when status changes:
//	- Start fetching a feed
//	- Finished fetching a feed
//
// In fact, the calls to parse_feed() and db_update_feed() should be
// moved out into the callback function.

/* update_feed
 * Update a feed: fetch the RSS, parse it, and add/update items in the
 * database.
 */
function update_feed($feed_id)
{
	global $out_fmt;
	global $fetch_error;

	/* Get the feed from the database */
	$feed = db_get_feed($feed_id);
	if (!$feed)
		abort("No such feed: $feed_id.");

	// XXX - Prettier output
	switch ($out_fmt)
	{
	    case "html":
		echo "<h3>Updating feed [$feed[title]]</h3>\n";
		break;
	    case "json":
		echo jsonify('state',	"start",
			     'feed_id',	$feed_id,
			     'title',	$feed['title']),
			"\n";
		flush();
		break;
	}

	/* Fetch the feed */
	$feed_text = fetch_url($feed['feed_url'],
			       $feed['username'],
			       $feed['passwd']);
	if ($feed_text === false)
	{
		switch ($out_fmt)
		{
		    case "html":
			echo "<b>$fetch_error</b><br/>\n";
			break;
		    case "json":
			echo jsonify('state',	"error",
				     'feed_id',	$feed_id,
				     'error',	$fetch_error),
				"\n";
			flush();
			break;
		}
	}

	/* Check the HTTP error code, save a cached copy of the feed,
	 * parse it, and add it to the database.
	 */
	_save_handle($feed_id, $feed_text);
		// XXX - Error-handling
		// XXX - Get item counts returned by _save_handle

	// XXX - Prettier output
	switch ($out_fmt)
	{
	    case "json":
		echo jsonify('state',	"end",
			     'feed_id',	$feed_id),
			"\n";
		flush();
		break;
	    case "html":
	    default:
		echo "Finished [$feed[title]]<br/>\n";
		break;
	}

	// XXX - Return something intelligent
}

/* update_all_feeds
 * As the name implies, this updates all the feeds listed in the
 * database.
 */
// XXX - Ought to try to avoid refreshing feeds too often. Add a bool
// $force argument to force an update.
function update_all_feeds()
{
	global $PARALLEL_UPDATES;
	global $out_fmt;

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

		// XXX - Prettier output
		switch ($out_fmt)
		{
		    case "html":
			echo "Starting ($feed[id]) [", $feed['title'], "]<br/>\n";
			flush();
			break;
		    case "json":
			echo jsonify('state',	"start",
				     'feed_id',	$feed['id'],
				     'title',	$feed['title']),
				"\n";
			flush();
			break;
		}
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
			// XXX - The CURLMSG_DONE test isn't
			// particularly useful, since it's the only
			// status that curl_multi_info_read() returns.
			if ($err['msg'] != CURLMSG_DONE)
			{
				// XXX - Better error-reporting
				switch ($out_fmt)
				{
				    case "html":
					echo "    Warning: curl_multi_info_read() returned [";
					print_r($err);
					echo "], and I don't know how to handle that.\n";
					break;
				    case "json":
					// XXX - Better error-reporting
					echo jsonify('state',	"error",
//						     'curl_err',var_export($err)),
						     'error_type', "curl",
						     'error',var_export($err)),
						"\n";
					flush();
					break;
				}
				break;	// End of enclosing while loop
			}

			// Find the appropriate handle, and set $handle as a
			// reference to its entry in $pipeline.
			unset($handle);		// Reset any previous connection
			$handle = NULL;
			for ($i = 0; $i < count($pipeline); $i++)
			{
				if ($pipeline[$i]['ch'] == $err['handle'])
				{
					$handle = &$pipeline[$i];
						// Note: reference to
						// $pipeline[$i], not copy.
					break;
				}
			}
			if (!isset($handle))
			{
				// Hopefully this will never happen
				// XXX - Better error-reporting
				switch ($out_fmt)
				{
				    case "html":
					echo "<b>Error: couldn't find curl handle ",
						$err['handle'], " in pipeline</b><br/>\n";
					break;
				    case "json":
					// XXX - Better error-reporting
					echo jsonify('state',	"error",
						     'error',	"Error: couldn't find curl handle " . $err['handle'], " in pipeline"),
						"\n";
					flush();
					break;
				}
				continue;
			}

			/* Check whether the transfer completed successfully
			 * 0 - OK
			 * 7 - Couldn't connect
			 * 47 - Too many redirects
			 */
			if ($err['result'] == CURLE_OK)
			{
				/* We've found the handle we want. Get
				 * its contents.
				 */
				// XXX - Better error-reporting
				// XXX - Move this to later, in case
				// things fail, so we can report
				// properly.
				switch ($out_fmt)
				{
				    case "html":
					echo "Finished (",
						$handle['feed']['id'],
						") [",
						$handle['feed']['title'],
						"]<br/>\n";
					flush();
					break;
				    case "json":
					echo jsonify('state',	"end",
						     'feed_id',	$handle['feed']['id']),
						"\n";
					flush();
					break;
				}
				$feed_text = curl_multi_getcontent($handle['ch']);
				if ($feed_text === false)
				{
					// XXX - Error-handling
					echo "<b>Couldn't get content.</b><br/>\n";
				} else {
					list($fetch_http_status,
					     $fetch_http_error) =
						_http_status($feed_text);
						// XXX - Error-checking
					if ($fetch_http_status != "200")
					{
						switch ($out_fmt)
						{
						    case "json":
							// XXX -
							// What's the
							// Right Thing
							// to do?
							break;
						    case "html":
						    default:
							echo "<b>HTTP Error ($fetch_http_status): $fetch_http_error</b><br/>\n";
							break;
						}
					} else
						_save_handle($handle['feed']['id'],
							     $feed_text);
							// XXX - Error-handling
							// XXX - Get
							// item counts
							// returned by
							// _save_handle
				}

				/* We're done with this handle. */
				curl_multi_remove_handle(
					$mh,
						$handle['ch']);
				curl_close($handle['ch']);
			} else {
				// XXX - Better error-reporting
				switch ($out_fmt)
				{
				    case "html":
					echo "<b>Error in [", $handle['feed']['title'],
						"]: ",
						curl_errno($handle['ch']),
						": [",
						curl_error($handle['ch']),
						"]</b><br/>\n";
					break;
				    case "json":
					echo jsonify('state',	"error",
						     'feed_id',	$handle['feed']['id'],
						     'error',	"XXX - Insert error message here"),
						"\n";
					flush();
					break;
				}
			}

			/* See if there's another URL to fetch */
			if (list ($idx, $feed) = each($feeds))
			{
				// Yes. Open a Curl handle, and add it
				// to the multi-handle
				// XXX - Prettier output
				switch ($out_fmt)
				{
				    case "html":
					echo "Starting ($feed[id]) [", $feed['title'], "]<br/>\n";
					flush();
					break;
				    case "json":
					echo jsonify('state',	"start",
						     'feed_id',	$feed['id'],
						     'title',	$feed['title']),
						"\n";
					flush();
					break;
				}
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
			// XXX - Prettier output
			switch ($out_fmt)
			{
			    case "html":
				echo "Finished (", $pipeline[$i]['feed']['id'], ") [", $pipeline[$i]['feed']['title'], "]<br/>\n"; flush();
				break;
			    case "json":
				echo jsonify('state',	"end",
					     'feed_id',	$pipeline[$i]['feed']['id']),
					"\n";
				flush();
				break;
			}
			$feed_text = curl_multi_getcontent($pipeline[$i]['ch']);
			_save_handle($pipeline[$i]['feed']['id'],
				      $feed_text);
				// XXX - Error-handling
				// XXX - Get item counts returned by
				// _save_handle
		}
	}

	curl_multi_close($mh);

	// XXX - Return something intelligent
}

/* _open_curl_handle
 * Private helper function to open a Curl handle to retrieve a URL,
 * and set the options we want.
 */
function _open_curl_handle($url, $username = NULL, $passwd = NULL)
{
	$ch = curl_init();

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
		curl_setopt($ch, CURLOPT_USERPWD, "$username:$passwd");
	}

	// Some sites insist on a non-empty user agent
	curl_setopt($ch, CURLOPT_USERAGENT, "dummy agent");

	return $ch;
}

function _save_handle($feed_id, &$feed_text)
{
	global $out_fmt;

	/* Save a copy of the feed text for debugging */
	if (defined("FEED_CACHE") && is_dir(FEED_CACHE))
	{
		// This will fail if permissions aren't right. Deal
		// with it. It's a debugging feature, so you should be
		// paying attention to error messages anyway.
		$fh = fopen(FEED_CACHE . "/$feed_id", "w");
		fwrite($fh, $feed_text);
		fclose($fh);
	}

	/* Parse the feed */
	$feed = parse_feed($feed_text);
	if (!$feed)
	{
		switch ($out_fmt)
		{
		    case "json":
			// XXX - What to do?
			break;
		    case "html":
		    default:
			// XXX - Better error-handling
			echo "<b>feed $feed_id: parse_feed() returned ";
			if ($feed === false) echo "FALSE";
			if ($feed === null) echo "NULL";
			if ($feed === "") echo "(empty string)";
			echo "</b><br/>\n";
			break;
		}

		return FALSE;
	}

	/* Add the feed to the database */
	return db_update_feed($feed_id, $feed);
}

/* fetch_url
 * Fetch the contents of the given URL, and return the page as a
 * string. As a side effect, strips off the HTTP header(s) from the
 * beginning of the string.
 * In case of error, returns FALSE and sets $fetch_error to an error
 * message.
 */
function fetch_url($url, $username = NULL, $passwd = NULL)
{
	global $fetch_error;
	global $fetch_http_status, $fetch_http_error;

	/* Initialize Curl */
	$ch = _open_curl_handle($url, $username, $passwd);

	/* Fetch the RSS feed */
	$feed_text = curl_exec($ch);

	// Check for curl errors
	if ($feed_text === false)
	{
		$fetch_error = "Curl error (" .
			curl_errno($ch) .
			"): " .
			curl_error($ch);
		curl_close($ch);
		return false;
	}

	list($fetch_http_status, $fetch_http_error) = _http_status($feed_text);
	if ($fetch_http_status != "200")
	{
		$fetch_error = "HTTP error $fetch_http_status: $fetch_http_error";
		curl_close($ch);
		return false;
	}

	curl_close($ch);
	return $feed_text;
}

function _http_status(&$text)
{
	/* Get the HTTP header(s), for the status code, so we can find
	 * out whether something went wrong.
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
			explode("\r\n\r\n", $text, 2);
		$header = $new_header;
		$text = $new_text;

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
	} while (substr($text, 0, 5) == "HTTP/");

	return array($http_status, $http_error);
}
?>
