<?php
/* editfeed.php
 * Edit the user-settable parameters on a feed.
 */
require_once("common.inc");
require_once("database.inc");
require_once("group.inc");
require_once("skin.inc");
require_once("hooks.inc");

load_hooks(PLUGIN_DIR);

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

/* mark_groups
 * Internal helper function: Given a feed ID and a group tree (as
 * returned by group_tree(), mark the groups that contain the feed:
 * set the 'marked' field to TRUE.
 */
function mark_groups($feed_id, &$group)
{
	$retval = 0;		// Return the number of groups this
				// feed is in. Really, that's just to
				// let us know whether we need to add
				// it to -1 or not.

	// If this group has members, iterate over them to see whether
	// $feed_id is one of them, and recurse as necessary.
	if (isset($group['members']) &&
	    count($group['members']) > 0)
	{
		foreach ($group['members'] as &$member)
		{
			/* $member is either an integer, if it's a
			 * feed ID, or a data structure, if it's
			 * another group.
			 */
			if ($member == $feed_id)
			{
				$group['marked'] = TRUE;
				$retval++;
				next;
			}

			if ($member['id'] < 0)
				$retval += mark_groups($feed_id, $member);
		}
	}

	// If a feed isn't in any other group, at least it's in -1.
	if ($retval == 0 && $group['id'] == -1)
	{
		$group['marked'] = TRUE;
		$retval++;
	}
	return $retval;
}

/* show_form
 * Display the form for updating a feed.
 */
function show_form($feed_id)
{
	// We've already established above that $feed_id is numeric
	$feed_info = db_get_feed($feed_id);
	if ($feed_info === NULL)
		abort("No such feed: $feed_id");

	// Figure out which groups this feed is in.
	$groups = group_tree(TRUE);
	mark_groups($feed_id, $groups);

	$skin = new Skin();

	$skin->assign('feed', $feed_info);
	$skin->assign('groups', $groups);
	$skin->assign('command', "update");
	$skin->display("editfeed");
}

function update_feed_info($feed_id)
{
	$feed_info = db_get_feed($feed_id);
	if ($feed_info === NULL)
		abort("No such feed: $feed_id");

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

	// XXX - Check parameters.

	if (!$ok)
	{
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

	/* Update the list of groups that the feed is in:
	 * $old_groups is the groups the feed is currently in.
	 * $new_groups is the ones it should be in, as gleaned from
	 * $_REQUEST.
	 * Diff the two to see whichg groups to remove the feed from,
	 * and which ones to add it to.
	 */
	$old_groups = $feed_info['groups'];
	$new_groups = array();
	foreach ($_REQUEST as $key => $value)
	{
		if (preg_match('/^group_(-?\d+)$/', $key, $match))
			$new_groups[] = $match[1];
	}

	// First diff: see which groups to remove the feed from.
	$diffs = array_diff($old_groups, $new_groups);
	if (count($diffs) != 0)
	{
		foreach ($diffs as $g)
		{
			// Remove feed $feed_id from group $g
			db_group_remove_member($g, $feed_id);
		}
	}

	// Second diff: see which groups to add the feed to.
	$diffs = array_diff($new_groups, $old_groups);
	if (count($diffs) != 0)
	{
		foreach ($diffs as $g)
		{
			echo "Add feed $feed_id to group $g<br/>\n";
			db_group_add_member($g, $feed_id);
		}
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
