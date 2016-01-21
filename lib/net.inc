<?php
/* net.inc
 * Network-related functions.
 */
require_once("config.inc");
require_once("database.inc");
require_once("parse-feed.inc");

// XXX - Move the HTTP-fetching stuff to a separate function, so that
// we can retrieve URLs separately.
//
// The calls to parse_feed() and db_update_feed() should be moved out
// into the callback function.

/* update_feed
 * Update a feed: fetch the RSS, parse it, and add/update items in the
 * database.
 */
function update_feed($feed_id, $feed = NULL)
{
	global $out_fmt;
	global $fetch_error;
	global $_save_handle_error;

	/* Get the feed from the database */
	if ($feed == NULL)
	{
		$feed = db_get_feed($feed_id);
		if (!$feed)
			return array('status'	=> -1,
				     'errmsg'	=> "No such feed: $feed_id"
				);
	}

	/* Fetch the feed */
//	error_log("Updating feed $feed_id, url " .
//		  $feed['feed_url']);
	$feed_text = fetch_url($feed['feed_url'],
			       $feed['username'],
			       $feed['passwd']);
	if ($feed_text === FALSE)
		return array('status'	=> -1,
			     'errmsg'	=> $fetch_error);

	/* Check the HTTP error code, save a cached copy of the feed,
	 * parse it, and add it to the database.
	 */
	$err = _save_handle($feed_id, $feed_text);
		// Error-handling: returns FALSE on error, or array on
		// success
	if (!$err)
		return array('status'	=> -1,
			     'errmsg'	=> "_save_handle(id $feed_id) failed: $_save_handle_error"
			);

	// Get updated version of feed, to return to caller.
	$feed = db_get_feed($feed_id);
	$counts = db_get_feed_counts($feed_id);
		// XXX - Error-checking

	$feed['counts'] = $counts;

	// XXX - Return something intelligent:
	// - update status
	// - HTTP error message, if any
	return $feed;
}

/* feed_update_handler
 * An abstract class for dealing with updating multiple feeds. When
 * something happens, like starting to update a feed, or an error, one
 * of the handler's methods is called.
 */
abstract class feed_update_handler
{
	/* start_feed: we're about to start updating the given feed */
	abstract function start_feed($feed_id, $feed_title);

	/* end_feed: we've finished updating the given feed */
	abstract function end_feed(&$feed);

	/* error: an error has occurred */
	abstract function error($feed_id, $feed_title, $msg);
}

/* update_all_feeds
 * As the name implies, this updates all the feeds listed in the
 * database.
 */
// XXX - This code is way too complicated. Can it be simplified?
// XXX - Ought to try to avoid refreshing feeds too often. Add a bool
// $force argument to force an update.
function update_all_feeds($handler)
{
	global $PARALLEL_UPDATES;
	global $out_fmt;
	global $verbose;

	$feeds = db_get_feeds();
		// XXX - Error-checking

	/* Shuffle the feeds. This way, if the PHP execution timeout
	 * is too short to allow all feeds to update, they all have a
	 * crack at being updated before the script times out.
	 */
	shuffle($feeds);

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
		return;	// XXX - Return something intelligent
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

		$handler->start_feed($feed['id'], $feed['title']);
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
		{
			// An error of some kind (from select())
			// XXX - Probably ought to figure out what happened

			// According to comments at
			// http://us3.php.net/manual/en/function.curl-multi-select.php
			// you really really need to have
			// curl_multi_exec() after any
			// curl_multi_select(). So here it is.
			do {
				$exec_stat = curl_multi_exec($mh, $active);
#echo "2nd curl_multi_exec returned [$exec_stat]\n";
			} while ($exec_stat == CURLM_CALL_MULTI_PERFORM);
			continue;
		}

		/* Check to see whether any transfers have completed */
		$in_queue = NULL;
		while ($err = curl_multi_info_read($mh, $in_queue))
		{
			// XXX - The CURLMSG_DONE test isn't
			// particularly useful, since it's the only
			// status that curl_multi_info_read() returns.
			if ($err['msg'] != CURLMSG_DONE)
			{
				$handler->error(NULL, NULL,
						"curl_multi_info_read() returned $err[msg]");
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
				$handler->error(NULL, NULL,
						"couldn't find curl handle $err[handle] in pipeline");
				$handler->error(NULL, NULL,
						"err == [".$err."]");
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
				$feed_text = curl_multi_getcontent($handle['ch']);
				if ($feed_text === false)
				{
					// XXX - Error-handling
					$handler->error($handle['feed']['id'],
							$handle['feed']['title'],
							"Couldn't get content");
				} else {
					list($fetch_http_status,
					     $fetch_http_error,
					     $fetch_http_extra) =
						_http_status($feed_text);
						// XXX - Error-checking
					if ($fetch_http_status != "200")
					{
						$handler->error($handle['feed']['id'],
								$handle['feed']['title'],
								"HTTP Error $fetch_http_status: $fetch_http_error");
					} else {
						# Check whether
						# $fetch_http_extra is set,
						# and if so, update the feed
						# URL.
						if (isset($fetch_http_extra['redirect']))
						{
							// XXX - The feed URL doesn't get redirected.

							// XXX - This only
							// works for "update
							// all feeds". Need
							// the same thing when
							// updating a single
							// feed.

							if ($verbose)
							  error_log("Feed " .
								    $handle['feed']['id'] .
								    ", \"" .
								    $handle['feed']['title'] .
								    "\": " .
								    $handle['feed']['feed_url'] .
								    " redirects to " .
								    $fetch_http_extra['redirect'] .
								    ". Consider updating the feed.");
							$handle['feed']['feed_url'] = $fetch_http_extra['redirect'];
							// XXX - Instead of updating the feed, ought to notify the
							// user, ideally by creating a new post with a link that'll
							// update the feed to the new URL.
#							db_update_feed($handle['feed']['id'], $handle['feed']);
						}

						$err = _save_handle(
							$handle['feed']['id'],
							$feed_text);
							// XXX - Error-handling: returns FALSE
							// on error, array on success.

						// Refresh feed from database, and get article counts
						$handle['feed'] = db_get_feed($handle['feed']['id']);
						$counts = db_get_feed_counts($handle['feed']['id']);
						$handle['feed']['counts'] = $counts;
						$handler->end_feed($handle['feed']);
					}
				}
			} else {
				$handler->error($handle['feed']['id'],
						$handle['feed']['title'],
						"\$err[result] == [$err[result]], url [".$handle['feed']['feed_url']."], curl error " .
						curl_errno($handle['ch']) .
						": " .
						curl_error($handle['ch']));
			}

			/* We're done with this handle. */
			curl_multi_remove_handle(
				$mh,
				$handle['ch']);
			curl_close($handle['ch']);

			/* See if there's another URL to fetch */
			if (list ($idx, $feed) = each($feeds))
			{
				// Yes. Open a Curl handle, and add it
				// to the multi-handle
				// XXX - Prettier output
				$handler->start_feed($feed['id'], $feed['title']);
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
	// XXX - Do we need to worry about "Location:" redirect
	// headers at this point?
	for ($i = 0; $i < count($pipeline); $i++)
	{
		if (isset($pipeline[$i]))
		{
			$feed_text = curl_multi_getcontent($pipeline[$i]['ch']);
			if ($feed_text === false)
			{
				// XXX - Error-handling
				$handler->error($pipeline[$i]['feed']['id'],
						$pipeline[$i]['feed']['title'],
						"Couldn't get content");
			}
			$err = _save_handle($pipeline[$i]['feed']['id'],
					    $feed_text);
				// XXX - Error-handling: returns FALSE
				// on error, array on success.
			// Refresh feed from database, and get article
			// counts
			$pipeline[$i]['feed'] = db_get_feed($pipeline[$i]['feed']['id']);
			$counts = db_get_feed_counts($pipeline[$i]['feed']['id']);
			$pipeline[$i]['feed']['counts'] = $counts;
			$handler->end_feed($pipeline[$i]['feed']);
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
		// Limit the types of authentication allowed. The idea
		// is to make it hard to send the password in the
		// clear.

		// XXX - Problem: twitter.com only allows Basic
		// authentication for feeds (at least, the @me feed).
		// This is the one authentication type that isn't in
		// CURLAUTH_ANYSAFE.
		//
		// To work around this, see if the URL uses https; if
		// it does, allow any authentication.
		$url_scheme = parse_url($url, PHP_URL_SCHEME);

		if ($url_scheme == "https")
			curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
		else
			curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANYSAFE);
		curl_setopt($ch, CURLOPT_USERPWD, "$username:$passwd");
	}

	// Some sites insist on a non-empty user agent
	curl_setopt($ch, CURLOPT_USERAGENT, "dummy agent");

	return $ch;
}

function _save_handle($feed_id, &$feed_text)
{
	global $_save_handle_error;	# Error message saying why this
					# failed.
	global $feed_errmsg;		# Set in lib/parse-feed.inc, when
					# parse_feed() fails.

	$retval = array();

	/* Save a copy of the feed text for debugging */
	if (defined("FEED_CACHE") && is_dir(FEED_CACHE))
	{
		// This will fail if permissions aren't right. Deal
		// with it. It's a debugging feature, so you should be
		// paying attention to error messages anyway.
		// XXX - OTOH, the error messages arguably mess up
		// JSON output. Should this be inside a try {} block
		// or something?
		@$fh = fopen(FEED_CACHE . "/$feed_id", "w");
		if ($fh)
		{
			fwrite($fh, $feed_text);
			fclose($fh);
		}
	}

	/* Parse the feed */
	$feed = parse_feed($feed_text);
	if (!$feed)
	{
		// XXX - When parse_feed() fails, it sets $feed_errmsg.
		// Make this propagate down to caller somehow.
		$_save_handle_error = $feed_errmsg;
		return FALSE;
	}

	/* See when the feed was last updated, i.e., when the most
	 * recent article was posted.
	 */
	$last_update = NULL;
	if (isset($feed['items']) &&
	    is_array($feed['items']))
	{
		foreach ($feed['items'] as $item)
		{
			if (isset($item['pub_time']) &&
			    $item['pub_time'] > $last_update)
				$last_update = $item['pub_time'];
		}
	}
	if (isset($last_update))
		// This will be fed to db_update_feed(), so we can
		// identify stale feeds.
		$feed['last_update'] = $last_update;

	if (!isset($feed['subtitle']))
		$feed['subtitle'] = NULL;

	/* Add the feed to the database */
	// XXX - What does this do? Is it redundant?
	// XXX - The callers call db_get_feed() to get a fresh copy of
	// the feed info. Presumably this should be done here. Or
	// db_update_feed() should be changed to a) take $feed by
	// reference, and b) update it with the latest info. Then the
	// callers needn't make redundant queries.
	$counts = db_update_feed($feed_id, $feed);
	$feed['counts'] = $counts;

	$_save_handle_error = "OK";
	return $feed;
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

	list($fetch_http_status,
	     $fetch_http_error,
	     $fetch_http_extra) = _http_status($feed_text);
	if ($fetch_http_status != "200")
	{
		$fetch_error = "HTTP error $fetch_http_status in [$url]: $fetch_http_error";
		curl_close($ch);
		return false;
	}
	// XXX - Check whether $fetch_http_extra is set and return it
	// somehow. Perhaps accept an optional pass-by-reference
	// argument, and write to that?

	curl_close($ch);
	return $feed_text;
}

function _http_status(&$text)
{
	$content_encoding = NULL;
	$extra = array();	# Extra data we might return.
				# Only the location of the redirect, so far.

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
			# At least one site uses LF instead of CR-LF
			preg_split("/\r?\n\r?\n/", $text, 2);
		$header = $new_header;
		$text = $new_text;

		/* Dig the HTTP status code out of the header */
		if (preg_match('{^HTTP/\S+\s+(\d+)\s+(.*?)\r?\n}',
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

			// XXX - For status 301 (but not 302), look
			// for a "Location: $url" header in $header.
			// Use this to update the database to point to
			// the new location. Or at least suggest a new
			// location to the user.

			// How to pass the location down to the
			// caller? Just return a 3d element to the
			// caller, which has access to the feed
			// information and can do something about it?

			if ($http_status[0] == "3" &&	# It's a 3xx status
			    preg_match('{^Location: (.*?)\r?\n}m',
				       $header,
				       $match))
			{
				$extra["redirect"] = $match[1];
#error_log("Redirected to [$extra[redirect]]");
			}

			# Get the Content-Encoding header, if any.
			if (preg_match('{^Content-Encoding:\s*(\S+)(\r?\n|$)}m', $header , $match))
				$content_encoding = $match[1];
		}
	} while (substr($text, 0, 5) == "HTTP/");

	# Uncompress the body, if necessary, and if we know how.
	switch ($content_encoding)
	{
	    case "gzip":
#		$text = gzuncompress($text);
#		$text = gzinflate($text);

		# XXX - Ye gods, what an ugly hack. For some reason,
		# gzuncompress($text) and gzinflate($text) complain of
		# "data error" in Jalopnik's RSS feed.
		# This code writes the compressed feed out to a temporary
		# file, then reads it back in with "compress.zlib://",
		# and for some reason this works.
		$tmpf = tempnam("/tmp", "gz_fix");
		file_put_contents($tmpf, $text);
		$text = file_get_contents("compress.zlib://$tmpf");
		unlink($tmpf);			# Delete temp file
		break;
	    case NULL:
	    default:
		break;
	}

	return array($http_status, $http_error, $extra);
}
?>
