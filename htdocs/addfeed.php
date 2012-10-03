<?php
/* addfeed.php
 * Add a feed.
 */
// XXX - Should accept OPML file.

require_once("common.inc");
require_once("database.inc");
require_once("net.inc");
require_once("skin.inc");

$feed_url = $_REQUEST['feed_url'];
	// XXX - Probably needs to be escaped. Can there be quotes in URLs?
$page_url = $_REQUEST['page_url'];
	// URL of page whose feed to subscribe to.

/* If we were given the URL to a content page, rather than directly to
 * the feed, download the page and look for links to RSS feeds.
 */
if (isset($page_url))
{
	global $feeds;		// Array of feeds found in this page
	$feeds = Array();

	// Read the page
	@$page = file_get_contents($page_url);
	if ($page === false)
	{
		// XXX - Better error-reporting
		// error_get_last()['message'] arguably gives too much
		// information, so it'd be nice to pare it down to
		// just what the user needs to know.
		// ['type'] is a numeric error message, but for both
		// "no such file" and "authorization required", it has
		// value 2. So not very useful.
		$errors = error_get_last();
		$errmsg = $errors['message'];
		echo "Error: ", $errmsg, "<br/>\n";
		exit(0);
	}

	// Parse as XML
	$dom = new DOMDocument();
	@$dom->loadHTML($page);
		// Tends to return lots of warnings on bad HTML.
		// Suppress this output.
	$head = $dom->getElementsByTagName("head");
	if ($head)
		$head = $head->item(0);
	$links = $head->getElementsByTagName("link");

	foreach ($links as $link)
	{
		$rel = $link->getAttribute("rel");
		if ($rel != "alternate")
			continue;
		$type = $link->getAttribute("type");
		$title = $link->getAttribute("title");
		$href = $link->getAttribute("href");

		// Append this feed to the list
		$feeds[] = Array("type" => $type,
				 "title" => $title,
				 "url" => $href);

	}

	if (count($feeds) == 0)
	{
		/* If there aren't any feed links, put up an error
		 * message to that effect.
		 */
		abort("Couldn't find any RSS links in that page.");
	} elseif (count($feeds) == 1)
	{
		/* Exactly one RSS feed link. Assume that that's what
		 * the user wants to subscribe to.
		 */
		$feed_url = $feeds[0]['url'];
	}
}

/* If we were given a single feed_url, subscribe to it, and refresh it
 * immediately.
 */
if (isset($feed_url))
{
	$params = Array();

	$params['feed_url'] = $feed_url;
	if (isset($_REQUEST['username']))
		$params['username'] = $_REQUEST['username'];
	if (isset($_REQUEST['password']) &&
	    $_REQUEST['password'] != "")
		$params['passwd'] = $_REQUEST['password'];

	// XXX - Check whether we're already subscribed to that URL

	$feed_id = db_add_feed($params);
	if ($feed_id === false)
		abort("Error adding feed.");

	/* Refresh the new feed, to get info and new articles */
	$err = update_feed($feed_id);
	if (!$err)
		// XXX - Better error reporting: include error message
		abort("Error updating new feed.");
	if (isset($err['status']) && $err['status'] != 0)
		abort($err['errmsg']);

	/* Redirect to the feed's page */
	redirect_to("view.php#id=$feed_id");
	exit(0);
}

/* Construct the URL for subscribing to a feed (i.e., the URL of this
 * script), so we can pass it to JavaScript magic.
 */
$subscribe_url = "http://";
if ($_SERVER['SERVER_NAME'] != "")
	$subscribe_url .= $_SERVER['SERVER_NAME'];
else
	$subscribe_url .= $_SERVER['SERVER_ADDR'];
if ($_SERVER['SERVER_PORT'] != "" && $_SERVER['SERVER_PORT'] != 80)
	$subscribe_url .= ":$_SERVER[SERVER_PORT]";
$subscribe_url .= $_SERVER['SCRIPT_NAME'];

/* Display a form for adding a URL */
$skin = new Skin();
$skin->assign("subscribe_url", $subscribe_url);
$skin->assign("feed_list", $feeds);
$skin->display("addfeed");
?>
