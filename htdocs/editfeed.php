<?php
/* editfeed.php
 * Edit the user-settable parameters on a feed.
 */
// XXX - Split this up into HTML vs. REST.
$out_fmt = "html";		// Mandatory output format
require_once("common.inc");
require_once("database.inc");
require_once("group.inc");

$feed_id = NULL;
if (isset($_REQUEST['id']))
	$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
else
	abort("Invalid feed ID: $feed_id.");

/* Get command. What are we supposed to do? */
$cmd = "";
if (isset($_REQUEST['command']))
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
				continue;
			}

			if (isset($member['id']) && intval($member['id']) < 0)
			{
				$retval += mark_groups($feed_id, $member);
			}
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
	$feed = db_get_feed($feed_id);
	if ($feed === NULL)
		abort("No such feed: $feed_id");

	// Figure out which groups this feed is in.
	$groups = group_tree(TRUE);
	mark_groups($feed_id, $groups);

	$feed_opts = db_get_feed_options($feed['id']);
########################################
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Editing <?=htmlspecialchars($feed['title'])?></title>
<link rel="stylesheet" type="text/css" href="css/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="css/editfeed.css" media="all" />
<meta name="theme-color" content="#8080c0" />
</head>
<body id="edit-feed">

<?php /* XXX - Links to get back to interesting places, like feed list */ ?>
<h1>Editing feed <?=htmlspecialchars($feed['title'])?></h1>

<form name="edit-feed" method="post" action="editfeed.php">
<?php /* Feed ID */ ?>
<input type="hidden" name="id" value="<?=$feed['id']?>"/>
<input type="hidden" name="command" value="update"/>

<table id="show-feed">
<?php /* XXX - Is it worth displaying the feed ID? */ ?>
  <tr>
    <th>ID</th>
    <td><?=$feed['id']?></td>
  </tr>

  <tr>
    <th>Title</th>
    <td><?=htmlspecialchars($feed['title'])?></td>
  </tr>

  <tr>
    <th>Subtitle</th>
    <td><?=$feed['subtitle'] ? htmlspecialchars($feed['subtitle']) : "&nbsp;"?></td>
  </tr>

<?php /* User-settable nickname */ ?>
  <tr>
    <th>Nickname</th>
    <td>
      <input type="text" name="nickname" value="<?=$feed['nickname']?>"/>
    </td>
  </tr>

<?php /* XXX - There should be a button or something to try to
   * auto-discover the feed URL from the site URL. Presumably the way
   * to do this is to fetch the site URL and check for "link
   * rel=alternate", where the MIME type is RSS or Atom.
   *
   * However, I'm not sure this can be done in JavaScript: we can't
   * just fetch an arbitrary URL.
   */
?>
  <tr>
    <th>Site URL</th>
    <td>
      <input type="text" name="url" value="<?=$feed['url']?>"/>
    </td>
  </tr>

  <tr>
    <th>Feed URL</th>
    <td>
      <input type="text" name="feed_url" value="<?=$feed['feed_url']?>"/>
    </td>
  </tr>

  <tr>
    <th>Description</th>
    <td>
      <div><?php
	# Sanitize description before displaying it.
	$description = $feed['description'];
	run_hooks("clean-html", array(&$description));
	echo $description;
?></div>
    </td>
  </tr>

<?php /* XXX - Probably not worth displaying this */ ?>
  <tr>
    <th>Last update</th>
    <td><?=$feed['last_update']?></td>
  </tr>

  <tr>
    <th>Image</th>
    <td>
<?php    if (isset($feed['image'])): ?>
        <img src="<?=$feed['image']?>"/>
<?php else: ?>
        No image.
<?php endif ?>
    </td>
  </tr>

  <tr>
    <th>Groups</th>
    <td>
<?php
      if (isset($groups['members'])  && count($groups['members']) > 0)
      {
		echo "<ul>";
		foreach ($groups['members'] as $g)
			if ($g['id'] < 0)
				html_group_list($g);
		echo "</ul>";
      }
?>
    </td>
  </tr>

  <tr>
    <th>Active</th>
    <td>
      <input type="checkbox" name="active"
	<?php if ($feed['active']) echo ' checked="checked"' ?>
      />
    </td>
  </tr>

  <tr>
    <th>Username</th>
    <td>
      <input type="text" name="username" value="<?=$feed['username']?>" autocomplete="off"/>
    </td>
  </tr>

  <tr>
    <th>Password</th>
    <td>
      <input type="password" name="password" value="<?=$feed['passwd']?>" autocomplete="off"/>
    </td>
  </tr>

  <tr>
    <th class="section-title" colspan="0">Options</th>
  </tr>
<?php
    if (count($feed_opts) > 0):
	foreach ($feed_opts as $opt => $value):
?>
    <tr>
      <th><?=$opt?></th>
      <td><input type="number"
		name="opt_<?=$opt?>"
		value="<?=$feed_opts[$opt]?>" /></td>
    </tr>
<?php
	  endforeach;
    endif;
?>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>

</body>
</html>
<?php
########################################
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
		// XXX - This is rather theoretical. We don't actually
		// check anything.
		abort("You supplied a bad value of some kind. Go back and fix it.");
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
		redirect_to("index.html");
}

/* html_group_list
 * Print a tree of groups, each with a checkbox.
 */
function html_group_list($group)
{
	// Display an <li> for this group.
	// The name is "group_<gid>" (e.g., "group_-19").
	// If the current feed is a member of this group
	// ($group['marked'] is set), then the checkbox is marked.
		$marked = array_key_exists("marked", $group) &&
			$group['marked'];
	echo "<li>",
		"<input type=\"checkbox\" name=\"group_",
		$group['id'],
		"\"",
		($marked ? " checked" : ""),
		"/>",
		 htmlspecialchars($group['name']);

	// If this group has children, recursively call html_group_list()
	// to display their tree.
	if (isset($group['members']) && count($group['members']) > 0)
	{
		echo "<ul>";
		foreach ($group['members'] as $g)
		{
			if (isset($g['id']) && $g['id'] < 0)
				html_group_list($g);
		}
		echo "</ul>";
	}
	echo "</li>\n";
}
?>
