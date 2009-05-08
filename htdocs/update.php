<?
/* update.php
 * Update one feed, or all feeds.
 */
// XXX - Move the guts of this file to a separate .inc file, so that a
// newly-added feed can be automatically updated.
require_once("config.inc");
require_once("common.inc");
//require_once("database.inc");
require_once("net.inc");
//require_once("parse-feed.inc");
require_once("skin.inc");

/* See what kind of output the user wants */
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

$feed_id = $_REQUEST["id"];

if (is_numeric($feed_id) && is_int($feed_id+0))
{
	$feed = db_get_feed($feed_id);

	switch ($out_fmt)
	{
	    case "html":
		echo "<h3>Updating feed [$feed[title]]</h3>\n";
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
		switch ($out_fmt)
		{
		    case "json":
			// XXX - What to do?
			break;
		    case "html":
		    default:
			// XXX - Better error-handling
			$err_msg = "feed $feed_id: parse_feed() returned ";
			if ($feed === false) $err_msg .= "FALSE";
			if ($feed === null)  $err_msg .= "NULL";
			if ($feed === "")    $err_msg .= "(empty string)";
			abort($err_msg);
		}
	}

	if (isset($err['status']) && $err['status'] != 0)
		abort($err['errmsg']);

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
	update_all_feeds();

	// XXX - Prettier output
	if ($out_fmt == "html")
		echo "<p><a href=\"view.php?id=$feed_id\">Read feeds</a></p>\n";
} else {
	/* Abort with an error message */
	abort("Invalid feed ID: $feed_id");
}

if ($out_fmt == "json")
	/* Close the "<![CDATA[" from above */
	echo "]]>\n";
?>
