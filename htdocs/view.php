<?
/* view.php
 * Display a feed.
 */
require_once("config.php");
require_once("database.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$feed_id = $_REQUEST["id"];
# XXX - Error-checking: make sure '$feed_is' is an integer.

$feed = db_get_feed($feed_id);
if (!$feed)
{
	// XXX - Abort more gracefully
	echo "No such feed: $feed_id<br/>\n";
	exit(0);
}

$feed = db_get_feed_items($feed);

#echo "feed: ["; print_r($feed); echo "]<br/>\n";

$smarty = new Smarty();
$smarty->template_dir	= SMARTY_PATH . "templates";
$smarty->compile_dir	= SMARTY_PATH . "templates_c";
$smarty->cache_dir	= SMARTY_PATH . "cache";
$smarty->config_dir	= SMARTY_PATH . "configs";

$smarty->assign('feed', $feed);
$smarty->assign('items', $feed['items']);
$smarty->display("view.tpl");
db_disconnect();

?>
