<?php
/* editfeed.php
 * Edit the user-settable parameters on a feed.
 */
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
else
	abort("Invalid feed ID: $feed_id.");

/* Get command. What are we supposed to do? */
$cmd = $_REQUEST['command'];

switch ($cmd)
{
    case "":
	/* No command. Start a new page */
	show_form($feed_id);
	break;

    case "update":
	/* Update an existing feed */
	update_feed_info($feed_id);
	break;

    default:
	abort("Invalid action.");
}
exit(0);

/* show_form
 * Display the form for updating a feed.
 */
function show_form($feed_id)
{
	// We've already established above that $feed_id is numeric
	$feed_info = db_get_feed($feed_id);
	if ($feed_info === NULL)
		abort("No such feed: $feed_id");

	$skin = new Skin();

	$skin->assign('feed', $feed_info);
	$skin->assign('command', "update");
	$skin->display("editfeed");
}

function update_feed_info($feed_id)
{
	/* Build an assoc of new values */
	$new = array();
	// I'm not sure why or how $_REQUEST values acquire
	// backslashes in front of quotation marks, but they do.
	$new['nickname'] = stripcslashes($_REQUEST['nickname']);
	$new['url']      = stripcslashes($_REQUEST['url']);
	$new['feed_url'] = stripcslashes($_REQUEST['feed_url']);
	$new['active']   = $_REQUEST['active'] != "";	# Boolean
	$new['username'] = stripcslashes($_REQUEST['username']);
	$new['passwd']   = stripcslashes($_REQUEST['password']);

	// XXX - Perhaps try to fetch the feed if the feed URL,
	// username, or password has changed?

	$ok = true;
	$errors = array();

	if (!$ok)
	{
		$feed_info = db_get_feed($feed_id);
		if ($feed_info === NULL)
			abort("No such feed: $feed_id");

		/* Insert the supplied values into $feed_info, so
		 * they'll show up in the form.
		 */
		$feed_info['nickname'] = $new['nickname'];
		$feed_info['url']      = $new['url'];
		$feed_info['feed_url'] = $new['feed_url'];
		$feed_info['username'] = $new['username'];
		$feed_info['passwd']   = $new['passwd'];

		/* There were errors. Redisplay the form, with
		 * error messages.
		 */
		$skin = new Skin();

		$skin->assign('feed', $feed_info);
		$skin->assign('errors', $errors);
		$skin->assign('command', "update");
		$skin->display("editfeed");
		return;
	}

	/* No errors. Update the database. */
	db_update_feed_info($feed_id, $new);
		// XXX - Error-checking

	/* Redirect to the feed view page */
	if ($new['active'])
		redirect_to("view.php#id=$feed_id");
	else
		redirect_to("index.php");
}
?>
