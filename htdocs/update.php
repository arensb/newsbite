<?php
/* update.php
 * Update one feed, or all feeds.
 */
// XXX - At some point, this script stopped outputting proper HTML
// when HTML output was requested. Huh. Or maybe it never did.
$verbose = FALSE;		// Enables verbose output

$default_fmt = "html";
$feed_id = NULL;
if (php_sapi_name() == "cli")
{
	$default_fmt = "console";	// If we're on the command line, default to text output.

	// Get feed ID from command line
	// -o fmt	Output format
	// -i NNN	NNN: feed number (integer) or "all" for all feeds.
	// -v		Be verbose
	// -q		Be quiet
	$opts = getopt("o:i:vq");
	if (isset($opts['o']))
		$out_fmt = $opts["o"];
	else
		$out_fmt = $default_fmt;
	$feed_id = $opts["i"];
	$verbose = isset($opts["v"]);
	$quiet   = isset($opts["q"]);
} else {
	// We're running from CGI. Get feed ID from the HTTP request.
	if (isset($_REQUEST["id"]))
		$feed_id = $_REQUEST["id"];
}

require_once("lib/common.inc");
require_once("lib/net.inc");

// XXX - Use the {html,json}_output_handler classes throughout, so we
// can eliminate a bunch of "switch ($out_fmt)" statements.

// XXX - Perhaps register a shutdown function with
// register_shutdown_function(). Inside it,
// connection_status() & 2 == 1 if the script timed out.
// This could be used to tell the user that it's probably a good idea
// to increase the max run time.
// Feeds are updated in the database as soon as they're fetched, so if
// the script times out, we've lost at most one update.
// See http://www.php.net/manual/en/features.connection-handling.php

/* html_output_handler
 * Used by update_all_feeds() to handle HTML output. Subclass of
 * feed_update_handler, defined in lib/net.inc.
 */
class html_output_handler extends feed_update_handler
{
	function start_feed($feed_id, $feed_title)
	{
		echo "Starting ($feed_id) [$feed_title]<br/>\n";
		flush();
	}

	function end_feed(&$feed)
	{
		echo "Finished (",
			$feed['id'],
			") [",
			$feed['title'],
			"]<br/>\n";
		flush();
	}

	function error($feed_id, $feed_title, $msg)
	{
		echo "<b>Error";
		if (isset($feed_id))
			echo " in $feed_title ($feed_id)";
		else
			echo " in \$feed_id == undef";
		echo ": $msg</b><br/>\n";
	}
}

/* console_output_handler
 * Used by update_all_feeds() to handle console output. Subclass of
 * feed_update_handler, defined in lib/net.inc.
 */
class console_output_handler extends feed_update_handler
{
	function start_feed($feed_id, $feed_title)
	{
		global $quiet;

		if (!$quiet)
			echo "Starting ($feed_id) [$feed_title]\n";
	}

	function end_feed(&$feed)
	{
		global $quiet;

		if (!$quiet)
			echo "Finished (",
				$feed['id'],
				") [",
				$feed['title'],
				"]\n";
	}

	function error($feed_id, $feed_title, $msg)
	{
		global $quiet;

		echo "Error";
		if (isset($feed_id))
			echo " in $feed_title ($feed_id)";
		else
			echo " in \$feed_id == undef";
		echo ": $msg\n";
	}
}

/* json_output_handler
 * Used by update_all_feeds() to handle JSON output. Subclass of
 * feed_update_handler, defined in lib/net.inc.
 */
class json_output_handler extends feed_update_handler
{
	function start_feed($feed_id, $feed_title)
	{
		// XXX - Convert to print_struct()?
		echo jsonify('state',	"start",
			     'feed_id',	$feed_id,
			     'title',	$feed_title),
			"\n";
		flush();
	}

	function end_feed(&$feed)
	{
		// XXX - Convert to print_struct()?
		echo jsonify('state',	"end",
			     'feed_id',	$feed['id'],
			     'title', $feed['title'],
			     'counts',	$feed['counts']
			),
			"\n";
		flush();
	}

	function error($feed_id, $feed_title, $msg)
	{
		// XXX - Convert to print_struct()?
		echo jsonify('state',	"error",
			     'feed_id',	$feed_id,
			     'title',	$feed_title,
			     'error',	$msg),
			"\n";
	}
}

/* Initialize output handler */
switch ($out_fmt)
{
    case "json":
	$handler = new json_output_handler();
	break;
    case "html":
	$handler = new html_output_handler();
	break;
    case "console":
	$handler = new console_output_handler();
	break;
    default:
	$handler = new html_output_handler();
	break;
}

/* See which feeds we're updating */
if (is_numeric($feed_id) && is_int($feed_id+0))
{
	/* Just update one feed */
	/* XXX - In HTML mode, probably shouldn't print anything
	 * unless there's an error. That way, can redirect back to the
	 * feed.
	 * Except that I normally only use HTML mode to see progress and
	 * error messages.
	 * Except not really: In single-feed mode, clicking "Update
	 * feed" should launch an Ajax call, and update the display.
	 * So Ajax; much pretty. If the user opens update.php in a
	 * separate window or tab, asking for HTML mode, then
	 * presumably they want to see the full HTML, with pretty
	 * output and such.
	 */
	/* XXX - Would this be desirable in "update all feeds" mode?
	 * Maybe not. Just try one-feed mode for now.
	 *
	 * OTOH, I use this extensively when debugging plugins to
	 * remove ads. So perhaps this stuff should be left alone,
	 * save that if the user left-clicks on the "Update feed"
	 * link, it should use JS and update in the background, while
	 * if ve middle-clicks, it should use traditional HTML.
	 */
	if ($verbose)
		error_log("Updating feed_id $feed_id");
	$feed = db_get_feed($feed_id);
	if ($feed === NULL)
	{
		// Invalid feed ID, presumably..
		error_log("Invalid feed ID: $feed_id");
		abort("Invalid feed ID: $feed_id");
	}

	$handler->start_feed($feed_id, $feed['title']);
	$err = update_feed($feed_id, $feed);

	if (!$err)
	{
		// XXX - This should never happen
		$handler->error($feed_id, $feed['title'],
			"parse_feed() returned " .
				($err == "" ?
				 var_export($err) : $err)
			);
	}

	/* Error-checking */
	if (isset($err['status']) && $err['status'] != 0)
	{
		switch ($out_fmt)
		{
		    case "json":
			// XXX - Convert to print_struct()?
			echo jsonify('state',	"error",
				     'feed_id',	$feed_id,
				     'error',	$err['errmsg']
				);
			exit(1);
		    case "html":
		    default:
			abort($err['errmsg']);
		}
		$handler->error($feed_id, $feed['title'], $err['errmsg']);
		// XXX - Do we exit(1) here?
	}

	$feed = $err;
	$handler->end_feed($feed);

	// XXX - Prettier output
	if ($out_fmt == "html")
		echo "<p><a href=\"view.php#id=$feed_id\">Read feed</a></p>\n";
} elseif ($feed_id == "all")
{
	/* Update all feeds */
	update_all_feeds($handler);

	// XXX - Prettier output
	if ($out_fmt == "html")
		echo "<p><a href=\"view.php#id=$feed_id\">Read feeds</a></p>\n";
} else {
	/* Invalid feed ID. Abort with an error message */
	error_log("Error: Invalid feed $feed_id");
	abort("Invalid feed ID: $feed_id");
}
?>
