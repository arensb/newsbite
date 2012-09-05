<?
/* view.php
 * Display a feed.
 */
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
elseif ($feed_id == "all")
	;
else
	abort("Invalid feed ID: \"$feed_id\".");

if ($feed_id == "all")
{
	/* Showing items from all feeds.
	 * Construct a pseudo-feed to put the items in
	 */
	$feed = array(
		"title"	=> "All feeds",
		"id"	=> "all",
		);
} else {
	$feed = db_get_feed($feed_id);
	if ($feed === NULL)
		abort("No such feed: $feed_id.");
}

# Things we don't want to send to the client
unset($feed['username']);
unset($feed['passwd']);

/* If we get this far, user has requested HTML output */
$skin = new Skin();

## XXX - Debugging
$skin->assign('auth_user', $auth_user);
$skin->assign('auth_expiration', strftime("%c", $auth_expiration));
## XXX - end debugging
$skin->display("view");

db_disconnect();

?>
