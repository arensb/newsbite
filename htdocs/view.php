<?
/* view.php
 * Display a feed.
 */
require_once("config.inc");
require_once("database.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$feed_id = $_REQUEST["id"];
# XXX - Error-checking: make sure '$feed_is' is an integer.

if (!is_numeric($feed_id) || !is_integer($feed_id+0))
{
	// XXX - Abort more gracefully
	echo "<p>Error: non-numeric feed ID.</p>\n";
	exit(0);
}

$feed = db_get_feed($feed_id);
if (!$feed)
{
	// XXX - Abort more gracefully
	echo "No such feed: $feed_id<br/>\n";
	exit(0);
}

$items = db_get_some_feed_items(
	"feed_id",	$feed['id'],
	"states",	"new,unread",
	"max_items",	25
	);
$feed['items'] = $items;

// Remove FeedBurner bugs.
// XXX - This belongs in a separate FeedBurner plugin.
// XXX - In fact, it should be done before adding items to database.
foreach ($feed['items'] as &$i)
{
	$i['summary'] = defeedburn($i['summary']);
	$i['content'] = defeedburn($i['content']);
	unset($i);	// Otherwise, $feed['items'] messed up: last
			// item removed, replaced with copy of
			// next-to-last item.
}

$smarty = new Smarty();
$smarty->template_dir	= SMARTY_PATH . "templates";
$smarty->compile_dir	= SMARTY_PATH . "templates_c";
$smarty->cache_dir	= SMARTY_PATH . "cache";
$smarty->config_dir	= SMARTY_PATH . "configs";

$smarty->assign('feed', $feed);
$smarty->assign('items', $feed['items']);
$smarty->display("view.tpl");

/* Now that these items have been sent to the browser, mark them as
 * "unread".
 */
$unread = array();
foreach ($feed['items'] as $i)
{
	if ($i['state'] == "new")
		$unread[] = $i['id'];
}
if (count($unread) > 0)
	db_mark_items("unread", $unread);

db_disconnect();

/* defeedburn
 * Remove bugs and links that FeedBurner adds to articles.
 * Not only does this help prevent FeedBurner from following your
 * reading habits, it also speeds up loading the page, since the
 * browser doesn't have to load a jillion external images.
 */
// XXX - This belongs in a FeedBurner plugin.
function defeedburn(&$str)
{
	$str = preg_replace(
		'{<p><a href="http://feeds.feedburner.com/.*?">.*?</a></p>}s',
		'',
		$str);

	$str = preg_replace(
		'{<div class="feedflare">(\s*<a href=".*?"><img src=".*?" border="0"></img></a>)*\s*</div>}s',
		'',
		$str);

	$str = preg_replace(
		'{<img src="http://feeds.feedburner.com/.*?" height="1" width="1"/>}s',
		'',
		$str);

	return $str;
}

?>
