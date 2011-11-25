<?
/* items.php
 * Send a list of items, selected by various criteria.
 */
/* XXX - Criteria:
 * - feed: ID of feed to include, or "all".
 * - (group: ID of group to include, or "all")
 * - n: max no. of articles to include (top out at 25 or something)
 * - b: Batch mode? Might want to allow caller to specify whether all the
 *      data should be sent in one efficient packet, or as it comes in.
 *      That way, can start displaying stuff before the transaction is finished.
 *      Then again, there might not be enough savings to matter.
 */
require_once("common.inc");
require_once("database.inc");
//require_once("hooks.inc");	// XXX - Ought to have hooks for
				// converting normal URLs to mobile
				// URLs.

//load_hooks(PLUGIN_DIR);

/* Make sure output format is sane */
switch ($out_fmt)
{
    case "xml":
    case "json":
	break;
    default:
	# 400 == generic bad request
	header("HTTP/1.0 400 Invalid output format.");
	exit(1);
}

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
elseif ($feed_id == "all")
	;
else
	header("HTTP/1.0 400 Invalid feed ID.");

if (isset($_REQUEST['s']))
{
	$start = $_REQUEST['s'];		// Skip first $start items

	/* Make sure $feed_id is an integer */
	if (!is_numeric($start) || !is_integer($start+0))
		/* Ignore illegal values. */
		$start = 0;
} else
	$start = 0;

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

/* XXX - Clean HTML in fixed-width fields:
 * feed:
 * - title
 * - subtitle
 * - nickname
 * - url (escape URL entities)
 * - feed_url (escape URL entities)
 * item:
 * - url (escape URL entities like '?' and '&')
 * - title
 * - author
 * - comment_url (escape URL entities like '?' and '&')
 * - comment_rss (escape URL entities like '?' and '&')
 */
/* XXX - If id == "all", need to include feed_url, feed_title, etc. in
 * each item.
 */

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
	# XXX - In batch mode, would want to send $feed first, then
	# each element of $items by itself.
	echo jsonify(
		array("feed"	=> $feed,
		      "items"	=> $items,
		      ));

	db_disconnect();
	exit(0);
}

if ($out_fmt == "xml")
{
	require_once("xml-output.inc");
				// Get print_xml() only when necessary

	print_xml(
		array("feed"	=> $feed,
		      "items"	=> $items));
	db_disconnect(0);
	exit(0);
}

?>
