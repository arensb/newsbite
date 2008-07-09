<?
require_once("config.inc");
require_once("database.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$smarty = new Smarty();
$skin = "default";
$smarty->template_dir	= "skins/$skin";
$smarty->compile_dir	= SMARTY_PATH . "templates_c";
$smarty->cache_dir	= SMARTY_PATH . "cache";
$smarty->config_dir	= SMARTY_PATH . "configs";
$smarty->compile_id = "default-skin";

$dbh = db_connect();
$feeds = db_get_feeds();
$counts = db_get_all_feed_counts();
$smarty->assign('skin', $skin);
$smarty->assign('feeds', $feeds);
$smarty->assign('counts', $counts);
$smarty->display("feeds.tpl");
db_disconnect();
?>
