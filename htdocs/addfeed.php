<?php
/* addfeed.php
 * Add a feed.
 */
require_once("config.inc");
require_once("database.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

// XXX - Display form asking for URL, (nickname), username, password.
// XXX - Get completed form.
// XXX - First pass: just add the supplied URL.

// XXX - Check whether we're already subscribed to that URL?

$feed_url = $_REQUEST['feed_url'];
	// XXX - Probably needs to be escaped. Can there be quotes in URLs?
echo "feed_url == [$feed_url]<br/>\n";

if (isset($feed_url))
{
	// XXX
	db_add_feed(array(
			    "feed_url" => $feed_url)
		);
	exit(0);
}

// XXX - $feed_url is not set.
/* Display a form for adding a URL */
$smarty = new Smarty();
$smarty->template_dir	= SMARTY_PATH . "templates";
$smarty->compile_dir	= SMARTY_PATH . "templates_c";
$smarty->cache_dir	= SMARTY_PATH . "cache";
$smarty->config_dir	= SMARTY_PATH . "configs";
$smarty->display("addfeed.tpl");
?>
