<?
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");

/* Try to guess whether we're viewing this on an iPhone or other
 * mobile device
 */
$mobile = false;
if (preg_match(',Mozilla/\S+ \(iPod;,', $_SERVER['HTTP_USER_AGENT']))
{
	$mobile = "iPhone";
} else if (preg_match(',Mozilla/\S+ \(iPad;,', $_SERVER['HTTP_USER_AGENT']))
{
	$mobile = "iPad";
}

$dbh = db_connect();
$feeds = db_get_feeds(TRUE);
$counts = db_get_all_feed_counts();

$skin = new Skin();

$skin->assign('feeds', $feeds);
$skin->assign('counts', $counts);
$skin->assign('mobile', $mobile);
$skin->display("feeds.tpl");
db_disconnect();
?>
