<?
/* update.php
 * Update one feed, or all feeds.
 */
require_once("common.inc");
require_once("net.inc");
require_once("skin.inc");

// XXX - Perhaps could use the {html,json}_output_handler classes
// throughout, so we can eliminate a bunch of "switch ($out_fmt)"
// statements.

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
		echo  "Finished (",
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
		echo ": $msg</b><br/>\n";
	}
}

/* json_output_handler
 * Used by update_all_feeds() to handle HTML output. Subclass of
 * feed_update_handler, defined in lib/net.inc.
 */
class json_output_handler extends feed_update_handler
{
	function start_feed($feed_id, $feed_title)
	{
		echo jsonify('state',	"start",
			     'feed_id',	$feed_id,
			     'title',	$feed_title),
			"\n";
		flush();
	}

	function end_feed(&$feed)
	{
		$skin = new Skin();
		$skin->assign('feed', $feed);
		$skin->assign('feed_id', $feed['id']);
		$skin->assign('counts', $feed['counts']);
		$count_display = $skin->fetch("feed-title.tpl");
		echo jsonify('state',	"end",
			     'feed_id',	$feed['id'],
			     'count_display',	$count_display
			),
			"\n";
		flush();
	}

	function error($feed_id, $feed_title, $msg)
	{
		echo jsonify('state',	"error",
			     'feed_id',	$feed_id,
			     'title',	$feed_title,
			     'error',	$msg),
			"\n";
	}
}

class console_output_handler extends feed_update_handler
{
	function start_feed($feed_id, $feed_title)
	{
		echo "Starting feed ($feed_id): [$feed_title]\n";
	}

	function end_feed(&$feed)
	{
		echo  "Finished (",
			$feed['id'],
			") [",
			$feed['title'],
			"]\n";
	}

	function error($feed_id, $feed_title, $msg)
	{
		error_log($msg);
	}
}


/* See what kind of output the user wants */

if ($_ENV['CRON'] == "yes")
{
	$out_fmt = "console";
} else {
	switch ($_REQUEST['o'])
	{
	    case "json":
		$out_fmt = "json";
		// The "+xml" here is bogus: apparently there's a bug in
		// Firefox (2.x) such that if the response is "text/plain", it
		// apparently assumes that it's ISO8859-1 or US-ASCII or some
		// such nonsense.
		header("Content-type: text/plain+xml; charset=utf-8");

		// The stupid "+xml" hack above means that Firefox will try to
		// interpret what it sees as XML. And since JSON isn't
		// well-formed XML, we need to wrap the JSON in very minimal
		// XML: < ?xml ? ><![CDATA[ {json} ]]>
		echo "<", '?xml version="1.0" encoding="UTF-8"?', ">\n";
		echo "<![CDATA[\n";
		break;
	    default:
		header("Content-type: text/html; charset=utf-8");
		$out_fmt = "html";
		break;
	}
}

$feed_id = $_REQUEST["id"];

/* See which feeds we're updating */
if (is_numeric($feed_id) && is_int($feed_id+0))
{
	/* Just update one feed */
	/* XXX - In HTML mode, probably shouldn't print anything
	 * unless there's an error. That way, can redirect back to the
	 * feed.
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
	$feed = db_get_feed($feed_id);

	switch ($out_fmt)
	{
	    case "html":
		echo "<h3>Updating feed [$feed[title]]</h3>\n";
		break;
	    case "console":
		echo "Updating feed [$feed[title]]\n";
		break;
	    case "json":
		$skin = new Skin();
		echo jsonify('state',	"start",
			     'feed_id',	$feed_id,
			     'title',	$feed['title']),
			"\n";
		flush();
		break;
	}
	$err = update_feed($feed_id, $feed);

	if (!$err)
	{
		// XXX - This should never happen
		switch ($out_fmt)
		{
		    case "json":
			// XXX - What to do?
			break;
		    case "console":
			// XXX - Better error-handling
			$err_msg = "feed $feed_id: $feed[title] ($feed[feed_url]): parse_feed() returned ";
			if ($err === false) $err_msg .= "FALSE";
			if ($err === null)  $err_msg .= "NULL";
			if ($err === "")    $err_msg .= "(empty string)";
			error_log($err_msg);
			exit(1);
		    case "html":
		    default:
			// XXX - Better error-handling
			$err_msg = "feed $feed_id: $feed[title] (<a href=\"$feed[feed_url]\">RSS</a>]): parse_feed() returned ";
			if ($err === false) $err_msg .= "FALSE";
			if ($err === null)  $err_msg .= "NULL";
			if ($err === "")    $err_msg .= "(empty string)";
			abort($err_msg);
			break;
		}
	}

	/* Error-checking */
	if (isset($err['status']) && $err['status'] != 0)
	{
		switch ($out_fmt)
		{
		    case "json":
			// XXX - What to do?
			exit(1);
		    case "console":
			error_log($err['errmsg']);
			exit(1);
		    case "html":
		    default:
			abort($err['errmsg']);
		}
	}

	$feed = $err;
	switch ($out_fmt)
	{
	    case "json":
		// XXX - This should go in calling function.
		$skin->assign('feed', $feed);
		$skin->assign('feed_id', $feed_id);
		$skin->assign('counts', $feed['counts']);
		$count_display = $skin->fetch("feed-title.tpl");
		echo jsonify('state',	"end",
			     'feed_id',	$feed_id,
			     'count_display',	$count_display
			     ),
			"\n";
		flush();
		break;
	    case "console":
		echo "Finished [$feed[title]]\n";
		break;
	    case "html":
	    default:
		echo "Finished [$feed[title]]<br/>\n";
		break;
	}

	// XXX - Prettier output
	if ($out_fmt == "html")
		echo "<p><a href=\"view.php?id=$feed_id\">Read feed</a></p>\n";
} elseif ($feed_id == "all")
{
	/* Update all feeds */
	switch ($out_fmt)
	{
	    case "json":
		$handler = new json_output_handler();
		break;
	    case "console":
		$handler = new console_output_handler();
		break;
	    case "html":
	    default:
		$handler = new html_output_handler();
		break;
	}

	update_all_feeds($handler);

	// XXX - Prettier output
	if ($out_fmt == "html")
		echo "<p><a href=\"view.php?id=$feed_id\">Read feeds</a></p>\n";
} else {
	/* Invalid feed ID. Abort with an error message */
	abort("Invalid feed ID: $feed_id");
}

if ($out_fmt == "json")
	/* Close the "<![CDATA[" from above */
	echo "]]>\n";
?>
