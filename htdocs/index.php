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
} else if (preg_match(',Mozilla/\S+ \(Linux; U; Android,', $_SERVER['HTTP_USER_AGENT']))
{
	$mobile = "Android";
}

$dbh = db_connect();
$feeds = db_get_feeds(TRUE);
$counts = db_get_all_feed_counts();

// Normalize each feed's name, to make sorting easier.
foreach ($feeds as &$f)
	get_normal_name($f);
$err = usort($feeds, "byname");

$skin = new Skin();

$skin->assign('feeds', $feeds);
$skin->assign('counts', $counts);
$skin->assign('mobile', $mobile);
$skin->display("feeds");
db_disconnect();

// get_normal_name
// Normalize the feed's name, for sorting purposes
// XXX - Arguably, this should be cached somewhere. Like in a "sortname"
// field in the database.
function get_normal_name(&$feed)
{
	# Use the nickname if it's set, or the title otherwise.
	$newname = $feed['title'];
	if ($feed['nickname'] != "")
		$newname = $feed['nickname'];

	# Lower-case everything for case-insensitivity
	$newname = strtolower($newname);

	# Move "a", "an", "the", etc. to the end.
	$newname = preg_replace('/^(a|an|the)\s+(.*)/',
				'\2 \1',
				$newname);

	# And set it in the array.
	$feed['normal_name'] = $newname;
}

// byname
// Helper function to sort feeds by (normalized) name.
function byname($a, $b)
{
	if ($a['normal_name'] == $b['normal_name'])
		return 0;
	return $a['normal_name'] < $b['normal_name'] ? -1 : 1;
}
?>
