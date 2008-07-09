<?php
/* unsubscribe.php
 * Remove a feed.
 */
// XXX - Should be possible to unsubscribe from a feed, but keep it in
// the database for later. This can be good for subscribing to
// political feeds only during election season, or sports feeds only
// during playoffs, that sort of thing.
require_once("config.inc");
require_once("database.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
else {
	// XXX - Abort more gracefully
	echo "<p>Error: invalid feed ID.</p>\n";
	exit(0);
}

/* Has confirmation been given? */
$confirm = $_REQUEST['confirm'];
echo "confirm == [$confirm]<br/>\n";
if ($confirm == "yes")
{
	// XXX - Go ahead and unsubscribe
echo "All-righty. Unsubscribing.<br/>\n";
	db_delete_feed($feed_id);
		// XXX - Error-checking
	// XXX - Should redirect someplace interesting, like the main
	// page.
	exit(0);
}

/* Confirmation has not been given. Show feed info and ask for
 * confirmation.
 */
// We've already established above that $feed_id is numeric
$feed_info = db_get_feed($feed_id);
	// XXX - Abort if no such feed

$smarty = new Smarty();
$skin = "default";
$smarty->template_dir	= "skins/$skin";
$smarty->compile_dir	= SMARTY_PATH . "templates_c";
$smarty->cache_dir	= SMARTY_PATH . "cache";
$smarty->config_dir	= SMARTY_PATH . "configs";

$smarty->assign('skin', $skin);
$smarty->assign('feed', $feed_info);
$smarty->display("unsubscribe.tpl");
?>
