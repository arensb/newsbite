<?
/* view.php
 * Display a feed.
 */
require_once("config.inc");
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

/* See what kind of output the user wants */
switch ($_REQUEST['o'])
{
    case "json":
	$out_fmt = "json";
	// The "+xml" here is bogus: apparently there's a bug in
	// Firefox (2.x) such that if the response is "text/plain", it
	// apparently assumes that it's ISO8859-1 or US-ASCII or some
	// such nonsense.
	header("Content-type: text/plain+xml; charset=utf-8");

	// The stupid "+xml" hack above means that Firefox will try to
	// interpret what it sees as XML. And since JSON isn't
	// well-formed XML, we need to wrap the JSON in very minimal
	// XML: < ?xml ? ><![CDATA[ {json} ]]>
	echo "<", '?xml version="1.0" encoding="UTF-8"?', ">\n";
	echo "<![CDATA[\n";
	break;
    case "jsonr":	// Raw JSON
	// XXX - A better approach would be to have a "hack" parameter
	// that turns on the +xml hack above. Perhaps the calling
	// script can auto-determine which hacks it needs.
	$out_fmt = "jsonr";
	header("Content-type: text/plain; charset=utf-8");
	break;
    case "xml":
	$out_fmt = "xml";
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
		"title"	=> "All feeds"
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
$feed['items'] = $items;

// XXX - Find out how many items there are in this list, so we can put
// up a navigation bar.
// XXX - At least, ought to find out how many items there are, so as
// not to add "earlier" link when there's nothing earlier.
/* Construct URL for earlier and later pages, and names for those links */
$prev_link = $_SERVER['PHP_SELF'] . "?id=$feed_id&s=" .
	($start + $num_items);
$prev_link_text = "Earlier";
//$prev_link_text = "Earlier &#x2397;";	// With "previous page" symbol

if ($start >= $num_items)
{
	$next_link = $_SERVER['PHP_SELF'] . "?id=$feed_id&s=" .
		($start - $num_items);
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

// XXX - Bleah. The Onion has a mobile site, but there's no obvious
// way to get the articles from the RSS feed: the feed uses URLs with
// the subject in the URL, while the mobile site only uses internal
// identifiers, which don't seem to appear anywhere else.
if ($mobile)
{
	// Wunderground has two mobile sites: 'i' for iPhone, 'm' for
	// generic mobile.
	if ($mobile == "iPhone")
		$m_wund = "i.wund.com";		# iPhone mobile site
	else
		$m_wund = "m.wund.com";		# Generic mobile site
	foreach ($feed['items'] as &$i)
	{
		$i['url'] = preg_replace(',^http://www\.lemonde\.fr/,',
					 'http://mobile.lemonde.fr/',
					 $i['url']);
		// WaPo stuff is ugly.
		// "spf=1" is to show the full article. Leave it off
		// to show only the first page.
		$i['url'] = preg_replace(',^http://www\.washingtonpost\.com/wp-dyn/content/article/(\d+)/(\d+)/(\d+)/(.*)\.html\??,',
					 'http://mobile.washingtonpost.com/rss.jsp?rssid=578819&item=http%3a%2f%2fwww.washingtonpost.com%2fwp-syndication%2farticle%2f\1%2f\2%2f\3%2f\4_mobile.xml&cid=1&spf=1&',
					 $i['url']);
		$i['url'] = preg_replace(',^http://www\.wunderground\.com/,',
					 "http://$m_wund/",
					 $i['url']);
		$i['url'] = preg_replace(',^http://(\w+)\.livejournal\.com/(.*),',
					 '\0?format=light',
					 $i['url']);
	}
}

// XXX - It would be great to normalize the HTML parts of the item, to
// protect against malformed HTML, mismatched tags, etc. The fragment
// below tries to do this by
//	- Load the HTML fragment as a DOMDocument
//	- (Let DOMDocument worry about errors)
//	- Dump the DOMDocument back to HTML

// However, there are several problems:
// 1) It adds DOCTYPE, <html>, and <body> tags. These can easily be
// stripped out, though.
// 2) It apparently assumes that the original HTML document uses
// ISO-8859-1 encoding, and so any UTF-8 characters get mangled. I
// haven't found a workaround for this.
// XXX - Should normalization be done when the item is added to the
// database?
#$doc = new DOMDocument('1.0', 'utf-8');
#foreach ($feed['items'] as &$i)
#{
#	if ($i['content'] != "")
#	{
##		$doc = new DOMDocument('1.0', 'utf-8');
#		@$doc->loadHTML($i['content']);
#		# Remove unwanted HTML parts.
#		$i['content'] = preg_replace('{^.*?<body>(.*)</body>.*?$}s', '\1', $doc->saveHTML());
#	}
#	if ($i['summary'] != "")
#	{
##		$doc = new DOMDocument('1.0', 'utf-8');
#		@$doc->loadHTML($i['summary']);
#		# Remove unwanted HTML parts.
#		$i['summary'] = preg_replace('{^.*?<body>(.*)</body>.*?$}s', '\1', $doc->saveHTML());
#	}
#}

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

if ($out_fmt == "json")
{
	echo jsonify($feed);
	/* Close the "<![CDATA[" from above */
	echo "\n]]>\n";

	db_disconnect();
	exit(0);
}

if ($out_fmt == "jsonr")
{
	echo jsonify($feed);

	db_disconnect();
	exit(0);
}

if ($out_fmt == "xml")
{
	print_xml($feed);
	db_disconnect(0);
	exit(0);
}

/* If we get this far, user has requested HTML output */
$skin = new Skin();

$skin->assign('feed', $feed);
if (isset($feeds))
	$skin->assign('feeds', $feeds);
$skin->assign('items', $feed['items']);
$skin->assign('prev_link', $prev_link);
$skin->assign('prev_link_text', $prev_link_text);
$skin->assign('next_link', $next_link);
$skin->assign('next_link_text', $next_link_text);
$skin->assign('mobile', $mobile);
$skin->display("view.tpl");

db_disconnect();

/* defeedburn
 * Remove bugs and links that FeedBurner adds to articles.
 * Not only does this help prevent FeedBurner from following your
 * reading habits, it also speeds up loading the page, since the
 * browser doesn't have to load a jillion external images.
 */
// XXX - This belongs in a FeedBurner plugin.
// XXX - Is this duplicated someplace in a lib/*.inc file?
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
