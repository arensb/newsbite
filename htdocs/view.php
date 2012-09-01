<?
/* view.php
 * Display a feed.
 */
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");
require_once("hooks.inc");

load_hooks(PLUGIN_DIR);

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
elseif ($feed_id == "all")
	;
else
	abort("Invalid feed ID: \"$feed_id\".");

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

$skin->assign('feed', $feed);
## XXX - Debugging
$skin->assign('auth_user', $auth_user);
$skin->assign('auth_expiration', strftime("%c", $auth_expiration));
## XXX - end debugging
$skin->display("view");

db_disconnect();

?>
