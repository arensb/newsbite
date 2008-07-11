<?
require_once("config.inc");
require_once("database.inc");
require_once("skin.inc");

$skin = new Skin();

$dbh = db_connect();
$feeds = db_get_feeds();
$counts = db_get_all_feed_counts();

$skin->assign('feeds', $feeds);
$skin->assign('counts', $counts);
$skin->display("feeds.tpl");
db_disconnect();
?>
