<?php
/* editfeed.php
 * Edit the user-settable parameters on a feed.
 */
require_once("config.inc");
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
else {
	// XXX - Abort more gracefully
	echo "<p>Error: invalid feed ID.</p>\n";
	exit(0);
}

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
	// XXX - Better error-reporting
	echo "<p>Error: invalid action [$cmd]</p>\n";
	exit(0);
}
exit(0);

/* show_form
 * Display the form for updating a feed.
 */
function show_form($feed_id)
{
	// We've already established above that $feed_id is numeric
	$feed_info = db_get_feed($feed_id);
		// XXX - Abort if no such feed

	$skin = new Skin();

	$skin->assign('feed', $feed_info);
	$skin->assign('command', "update");
	$skin->display("editfeed.tpl");
}

function update_feed_info($feed_id)
{
	// XXX - Check values

	/* Build an assoc of new values */
	$new = array();
	// I'm not sure why or how $_REQUEST values acquire
	// backslashes in front of quotation marks, but they do.
	$new['nickname'] = stripcslashes($_REQUEST['nickname']);
	$new['url']      = stripcslashes($_REQUEST['url']);
	$new['feed_url'] = stripcslashes($_REQUEST['feed_url']);
	$new['username'] = stripcslashes($_REQUEST['username']);
	$new['passwd']   = stripcslashes($_REQUEST['password']);

	// XXX - Perhaps try to fetch the feed if the feed URL,
	// username, or password has changed?

	$ok = true;
	$errors = array();
/*
	// XXX - For testing, don't allow "xxx" in any of these
	// values.
	if (strstr($new['nickname'], "xxx"))
	{
		$ok = false;
		$errors['nickname'] = "Can't have xxx in nickname.";
	}
	if (strstr($new['url'], "xxx"))
	{
		$ok = false;
		$errors['url'] = "Can't have xxx in url.";
	}
	if (strstr($new['feed_url'], "xxx"))
	{
		$ok = false;
		$errors['feed_url'] = "Can't have xxx in feed url.";
	}
	if (strstr($new['username'], "xxx"))
	{
		$ok = false;
		$errors['username'] = "Can't have xxx in username.";
	}
	if (strstr($new['passwd'], "xxx"))
	{
		$ok = false;
		$errors['passwd'] = "Can't have xxx in password.";
	}
*/

	if (!$ok)
	{
		$feed_info = db_get_feed($feed_id);
			// XXX - Abort if no such feed

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
		$skin->display("editfeed.tpl");
		return;
	}

	/* No errors. Update the database. */
	db_update_feed_info($feed_id, $new);
		// XXX - Error-checking

	// XXX - Redirect to someplace interesting. Like maybe
	// view.php?id=$feed_id, or index.php
	redirect_to("view.php?id=$feed_id");
}
?>
