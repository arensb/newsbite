<?
/* view.php
 * Display a feed.
 */
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");
require_once("hooks.inc");

load_hooks(PLUGIN_DIR);

/* See what kind of output the user wants */
switch ($_REQUEST['o'])
{
    case "json":
	// XXX - If another browser comes along that can't deal with
	// standard JSON, add a "hack" parameter or something to say
	// how to work around it.
	$out_fmt = "json";
	header("Content-type: text/plain; charset=utf-8");
	break;
    case "xml":
	$out_fmt = "xml";
	header("Content-type: text/xml; charset=utf-8");
	break;
    default:
	header("Content-type: text/html; charset=utf-8");
	$out_fmt = "html";
	break;
}

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
elseif ($feed_id == "all")
	;
else
	abort("Invalid feed ID: \"$feed_id\".");

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
		"title"	=> "All feeds",
		"id"	=> "all",
		);
	$feeds = db_get_feeds();
} else {
	$feed = db_get_feed($feed_id);
	if ($feed === NULL)
		abort("No such feed: $feed_id.");
}

# Things we don't want to send to the client
unset($feed['username']);
unset($feed['passwd']);

$num_items = 25;		// How many items to show
		// XXX - Should probably be a parameter

$get_feed_args = array(
	"read"		=> false,
	"max_items"	=> $num_items,
	"start_at"	=> $start
	);
if (is_integer($feed_id))
	$get_feed_args['feed_id'] = $feed_id;
// XXX - Put these selection criteria in a JS object, which we can put
// at the top of the rendered page, so that scripts on that page can
// tell what we're displaying.
$items = db_get_some_feed_items($get_feed_args);

// XXX - Find out how many items there are in this list, so we can put
// up a navigation bar.
// XXX - At least, ought to find out how many items there are, so as
// not to add "earlier" link when there's nothing earlier.
/* Construct URL for earlier and later pages, and names for those links */
$prev_link = htmlentities($_SERVER['PHP_SELF'] . "?id=$feed_id&s=" .
			  ($start + $num_items));
$prev_link_text = "Earlier";
//$prev_link_text = "Earlier &#x2397;";	// With "previous page" symbol

if ($start >= $num_items)
{
	$next_link = htmlentities($_SERVER['PHP_SELF'] . "?id=$feed_id&s=" .
				  ($start - $num_items));
	$next_link_text = "Later";
//	$next_link_text = "Later &#x2398;";	// With "next page" symbol
} else {
	$next_link = $next_link_text = NULL;
}

/* If we're reading this on a mobile device, try to convert the URL to
 * a mobile-friendly version.
 */
// XXX - This should go in a separate plugin
// XXX - Is it worth adding a 'mobile_url' column to the feeds table,
// and generating these URLs when the feed is fetched?
if ($mobile &&
    $mobile != "iPad")		// XXX - Hack!
{
	// Wunderground has two mobile sites: 'i' for iPhone, 'm' for
	// generic mobile.
	if ($mobile == "iPhone")
		$m_wund = "i.wund.com";		# iPhone mobile site
	else
		$m_wund = "m.wund.com";		# Generic mobile site
	foreach ($items as &$i)
	{
		// WaPo stuff is ugly.
		// "spf=1" is to show the full article. Leave it off
		// to show only the first page.
		$i['url'] = preg_replace(',^http://www\.washingtonpost\.com/wp-dyn/content/article/(\d+)/(\d+)/(\d+)/(.*)\.html\??,',
					 'http://mobile.washingtonpost.com/rss.jsp?rssid=578819&item=http%3a%2f%2fwww.washingtonpost.com%2fwp-syndication%2farticle%2f\1%2f\2%2f\3%2f\4_mobile.xml&cid=1&spf=1&',
					 $i['url']);
		$i['url'] = preg_replace(',^http://(www\.)?factcheck\.org/,',
					 "http://m.factcheck.org/",
					 $i['url']);
		$i['url'] = preg_replace(',^http://(www\.)?thinkprogress\.org/,',
					 "http://m.thinkprogress.org/",
					 $i['url']);
		$i['url'] = preg_replace(',^http://www\.wunderground\.com/,',
					 "http://$m_wund/",
					 $i['url']);
		$i['url'] = preg_replace(',^http://(\w+)\.livejournal\.com/(.*),',
					 '\0?format=light',
					 $i['url']);
		$i['url'] = preg_replace(',^http://(www\.)?theonion\.com/,',
					 "http://mobile.theonion.com/",
					 $i['url']);
	}
}

if ($mobile)
{
	// Other transformations that apply to iPad as well.
	foreach ($items as &$i)
	{
		$i['url'] = preg_replace(',^http://www\.lemonde\.fr/,',
					 'http://mobile.lemonde.fr/',
					 $i['url']);
		# ABC News URLs:
		# Google News link:
	  	# http://news.google.com/news/url?sa=t&fd=R&usg=AFQjCNGhqEe8eCdraXpBBGJwMHdr9LLqdA&url=http://abcnews.go.com/US/nantucket-massachusetts-mom-accused-killing-toddler-daughter-exorcism/story?id%3D13158775
		#
		# Plain link:
		# http://abcnews.go.com/US/nantucket-massachusetts-mom-accused-killing-toddler-daughter-exorcism/story?id%3D13158775
		#
		# iPad link:
		# http://abcnews.go.com/US/nantucket-massachusetts-mom-accused-killing-toddler-daughter-exorcism/t/story?id=13158775
		#
		# iPhone link:
		# http://abcnews.go.com/m/story?id=13158775
		if ($mobile == "iPad")
			$i['url'] = preg_replace(',(http://abcnews\.go\.com/.*)/story,',
						 '\1/t/story',
						 $i['url']);
		else
			$i['url'] = preg_replace(',http://abcnews\.go\.com/.*/story,',
						 'http://abcnews.go.com/m/story',
						 $i['url']);

		$i['url'] = preg_replace(',^http://(www\.)?npr\.org/templates/story/story.php?storyId=(129780261),',
					 'http://www.npr.org/tablet/#story?storyId=\2',
					 $i['url']);
	}
}

if ($out_fmt == "json")
{
	echo jsonify($feed);

	db_disconnect();
	exit(0);
}

if ($out_fmt == "xml")
{
	require_once("xml-output.inc");
				// Get print_xml() only when necessary

	print_xml($feed);
	db_disconnect(0);
	exit(0);
}

/* If we get this far, user has requested HTML output */
$skin = new Skin();

$skin->assign('start', $start);
$skin->assign('feed', $feed);
if (isset($feeds))
	$skin->assign('feeds', $feeds);
$skin->assign('items', $items);
$skin->assign('prev_link', $prev_link);
$skin->assign('prev_link_text', $prev_link_text);
$skin->assign('next_link', $next_link);
$skin->assign('next_link_text', $next_link_text);
$skin->assign('mobile', $mobile);
# XXX - Debugging
$skin->assign('auth_user', $auth_user);
$skin->assign('auth_expiration', strftime("%c", $auth_expiration));
# XXX - end debugging
$skin->display("view");

db_disconnect();

?>
