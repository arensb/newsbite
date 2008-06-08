<?
require_once("config.inc");
require_once("database.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$smarty = new Smarty();
$smarty->template_dir	= SMARTY_PATH . "templates";
$smarty->compile_dir	= SMARTY_PATH . "templates_c";
$smarty->cache_dir	= SMARTY_PATH . "cache";
$smarty->config_dir	= SMARTY_PATH . "configs";

$dbh = db_connect();
$feeds = db_get_feeds();
$smarty->assign('feeds', $feeds);
$smarty->display("feeds.tpl");
db_disconnect();
?>
