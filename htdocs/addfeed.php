<?php
/* addfeed.php
 * Add a feed.
 */
// XXX - Should accept OPML file.
// XXX - There should be a bookmarklet for this
// Basically, just go through <head> and look for
// <link rel="alternate" type="application/rss+xml" title="DOGHOUSE Feed" href="http://www.thedoghousediaries.com/?feed=rss2" />
// There may be several of them, either for different flavors of RSS
// (RSS 0.92 vs. RSS 2.0 vs. Atom), given by "type="; or for different
// feeds (articles vs. comments), indicated by "title=".
// If there are multiple feeds, ought to present the user with a list
// saying which one(s) to subscribe to.
//
// Might be simplest to decompose the page text with SimpleXML.
//
// The bookmarklet should probably look something like
// javascript:void(location.href='http://www.ooblick.com/newsbite/addfeed.php?url='+escape(location.href))

require_once("common.inc");
require_once("database.inc");
require_once("net.inc");
require_once("skin.inc");

$feed_url = $_REQUEST['feed_url'];
	// XXX - Probably needs to be escaped. Can there be quotes in URLs?
$page_url = $_REQUEST['page_url'];
	// URL of page whose feed to subscribe to.

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
	redirect_to("view.php?id=$feed_id");
	exit(0);
}

// We were given a page URL rather than a direct link to the RSS. Find
// the RSS links.

// XXX - Will need changes to the skin: pass a link of feed URLs and
// allow the user to pick the one to subscribe to.

if (isset($page_url))
{
	// Read the page
	$page = file_get_contents($page_url);

	// Parse as XML
	$dom = new DOMDocument();
	@$dom->loadHTML($page);
		// Tends to return lots of warnings on bad HTML.
		// Suppress this output.
	$links = $dom->getElementsByTagName("link");
		// XXX - This isn't limited to the head element. Do we
		// care?
echo $links->length, " links<br/>\n";
foreach ($links as $link)
{
	$rel = $link->getAttribute("rel");
	if ($rel != "alternate")
		continue;
	$type = $link->getAttribute("type");
	$title = $link->getAttribute("title");
	$href = $link->getAttribute("href");
	echo "rel=[$rel] type=[$type] title=[$title] href=[$href]<br/>\n";
}
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
$subscribe_url .= $_SERVER['REQUEST_URI'] . '?feed_url=%s';

/* Display a form for adding a URL */
$skin = new Skin();
$skin->assign("subscribe_url", $subscribe_url);
$skin->display("addfeed.tpl");
?>
