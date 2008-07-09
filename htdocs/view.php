<?
/* view.php
 * Display a feed.
 */
require_once("config.inc");
require_once("database.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
elseif ($feed_id == "all")
	;
else {
	// XXX - Abort more gracefully
	echo "<p>Error: invalid feed ID.</p>\n";
	exit(0);
}

$start = $_REQUEST['s'];		// Skip first $start items
/* Make sure $feed_id is an integer */
if (!is_numeric($start) || !is_integer($start+0))
	/* Ignore illegal values. */
	$start = 0;
$start = (int) $start;

if ($feed_id == "all")
{
	/* Showing items from all feeds.
	 * Construct a pseudo-feed to put the items in
	 */
	$feed = array(
		"title"	=> "All feeds"
		);
	$feeds = db_get_feeds();
} else {
	$feed = db_get_feed($feed_id);
	if (!$feed)
	{
		// XXX - Abort more gracefully
		echo "No such feed: $feed_id<br/>\n";
		exit(0);
	}
}

$num_items = 25;		// How many items to show
		// XXX - Should probably be a parameter

$get_feed_args = array(
	"states"	=> "new,unread,updated",
	"max_items"	=> $num_items,
	"start_at"	=> $start
	);
if (is_integer($feed_id))
	$get_feed_args['feed_id'] = $feed_id;
// XXX - Put these selection criteria in a JS object, which we can put
// at the top of the rendered page, so that scripts on that page can
// tell what we're displaying.
$items = db_get_some_feed_items($get_feed_args);
$feed['items'] = $items;

// XXX - Find out how many items there are in this list, so we can put
// up a navigation bar.
// XXX - At least, ought to find out how many items there are, so as
// not to add "earlier" link when there's nothing earlier.
/* Construct URL for earlier and later pages, and names for those links */
$prev_link = $_SERVER['PHP_SELF'] . "?id=$feed_id&s=" .
	($start + $num_items);
$prev_link_text = "Earlier";

if ($start > $num_items)
{
	$next_link = $_SERVER['PHP_SELF'] . "?id=$feed_id&s=" .
		($start - $num_items);
	$next_link_text = "Later";
} else {
	$next_link = $next_link_text = NULL;
}

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
$skin = "default";
$smarty->template_dir	= "skins/$skin";
$smarty->compile_dir	= SMARTY_PATH . "templates_c";
$smarty->cache_dir	= SMARTY_PATH . "cache";
$smarty->config_dir	= SMARTY_PATH . "configs";

$smarty->assign('skin', $skin);
$smarty->assign('feed', $feed);
if (isset($feeds))
	$smarty->assign('feeds', $feeds);
$smarty->assign('items', $feed['items']);
$smarty->assign('prev_link', $prev_link);
$smarty->assign('prev_link_text', $prev_link_text);
$smarty->assign('next_link', $next_link);
$smarty->assign('next_link_text', $next_link_text);
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
