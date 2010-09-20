<?php
/* unsubscribe.php
 * Remove a feed.
 */
// XXX - Should be possible to unsubscribe from a feed, but keep it in
// the database for later. This can be good for subscribing to
// political feeds only during election season, or sports feeds only
// during playoffs, that sort of thing.
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");
require_once("hooks.inc");

load_hooks(PLUGIN_DIR);

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
else
	abort("Invalid feed ID: $feed_id");

/* Has confirmation been given? */
$confirm = $_REQUEST['confirm'];
//echo "confirm == [$confirm]<br/>\n";
if ($confirm == "yes")
{
	/* Go ahead and unsubscribe */
	db_delete_feed($feed_id);
		// XXX - Error-checking

	/* Redirect back to the main page */
	redirect_to("index.php");
	exit(0);
}

/* Confirmation has not been given. Show feed info and ask for
 * confirmation.
 */
// We've already established above that $feed_id is numeric
$feed_info = db_get_feed($feed_id);
if ($feed_info === NULL)
	/* No such feed. Abort */
	abort("No such feed: $feed_id.");

$skin = new Skin();

$skin->assign('feed', $feed_info);
$skin->display("unsubscribe");
?>
