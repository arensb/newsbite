<?php
/* addfeed.php
 * Add a feed.
 */
// XXX - Should accept OPML file.
// XXX - There should be a bookmarklet for this
require_once("config.inc");
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");

// XXX - Display form asking for URL, (nickname), username, password.
// XXX - Get completed form.
// XXX - First pass: just add the supplied URL.

// XXX - Check whether we're already subscribed to that URL?

$feed_url = $_REQUEST['feed_url'];
	// XXX - Probably needs to be escaped. Can there be quotes in URLs?

if (isset($feed_url))
{
	// XXX
	db_add_feed(array(
			    "feed_url" => $feed_url)
		);
		// XXX - Error-checking
	// XXX - Refresh the new feed, to get info and new articles

	/* Redirect to the index page */
	// XXX - Is there a better place to redirect to? */
	redirect_to("index.php");
	exit(0);
}

// If we get this far, $feed_url is not set.

/* Construct the URL for subscribing to a feed, so we can pass it to
 * JavaScript magic.
 */
$subscribe_url = "http://";
if ($_SERVER['SERVER_NAME'] != "")
	$subscribe_url .= $_SERVER['SERVER_NAME'];
else
	$subscribe_url .= $_SERVER['SERVER_ADDR'];
if ($_SERVER['SERVER_PORT'] != "" && $_SERVER['SERVER_PORT'] != 80)
	$subscribe_url .= ":$_SERVER[SERVER_PORT]";
$subscribe_url .= $_SERVER['REQUEST_URI'] . '?feed=%s';

/* Display a form for adding a URL */
$skin = new Skin();
$skin->assign("subscribe_url", $subscribe_url);
$skin->display("addfeed.tpl");
?>
