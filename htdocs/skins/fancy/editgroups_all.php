<?php
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
global $skin_dir;
$skin_dir = $skin_vars['skin'];
global $groups;
$groups = &$skin_vars['groups'];

function delete_group_form($gid)
{
	$retval = "<form name=\"delete-group\" method=\"post\" action=\"groups.php\">";
	$retval .= "<input name=\"command\" type=\"hidden\" value=\"delete\"/>";
	$retval .= "<input name=\"id\" type=\"hidden\" value=\"$gid\"/>";
	$retval .= "<input name=\"delete\" type=\"submit\" value=\"Delete group\"/>";
	$retval .= "</form>";
	return $retval;
}

function group_select_list(&$group, $prefix = "")
{
#echo "Inside group_select_list(<pre>", print_r($group, TRUE), "</pre>)";
echo "<pre>";
debug_print_backtrace();
echo "</pre>";
	$gid = $group['id'];

	echo "<option value=\"$gid\">",
		$prefix, " ",
		htmlspecialchars($group['name']),
		"</option>";
	if (isset($group['members']) &&  count($group['members']) > 0)
	{
		foreach ($group['members'] as $g)
		{
			if ($g['id'] < 0)
				group_select_list($g, "- " . $prefix);
		}
	}
}

/* group_list
 * Print a tree of groups, each with a checkbox.
 */
function group_list($group)
{
	global $groups;
echo "group_list global groups:(<pre>", print_r($groups, TRUE), "</pre>)\n";
#return;

	$gid = $group['id'];

	// Display an <li> for this group.
	// The name is "group_<gid>" (e.g., "group_-19").
	// If the current feed is a member of this group
	// ($group['marked'] is set), then the checkbox is marked.
	echo "<li>",
		"<input",
		" name=\"name_$gid\"",
		" type=\"text\"",
		" size=\"20\"",
		" value=\"", htmlspecialchars($group['name']), "\"",
		">",
		"</input>";
#	echo "[reparent]";
/*
	echo "<form name=\"reparent-group\" method=\"post\" action=\"groups.php\">";
	echo "<input name=\"command\" type=\"hidden\" value=\"reparent\"/>";
	echo "<input name=\"id\" type=\"hidden\" value=\"$gid\"/>";
*/
	echo "<select name=\"parent\">";
echo "Calling group_select_list(<pre>", print_r($groups, TRUE), "</pre>)\n";
	group_select_list($groups, "foo");
	echo "</select>";
/*
	echo "<input name=\"reparent\" type=\"submit\" value=\"Reparent group\"/>";
	echo "</form>";
*/

#	echo "[delete]";
	echo delete_group_form($gid);

	// If this group has children, recursively call group_list()
	// to display their tree.
	echo "<ul>";
	if (isset($group['members']) && count($group['members']) > 0)
	{
		foreach ($group['members'] as $g)
			if ($g['id'] < 0)
				group_list($g);
	}
	echo "<li>[add new child]</li>";
	echo "</ul>";

	echo "</li>\n";
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Editing groups</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/editgroups.css" media="all" />
</head>
<body id="edit-group">

<? /* XXX - Links to get back to interesting places, like feed list */ ?>
<h1>Groups</h1>

<form name="edit-groups" method="post" action="groups.php">
<input type="hidden" name="command" value="<?=$skin_vars['command']?>"/>

<!-- XXX - Tree of groups. -->
<ul>
<?
#group_select_list($groups, "top");
/* $groups is the tree for -1 == All.
 * Display a tree of its children.
 */
foreach ($groups['members'] as $group)
{
#echo "In loop global groups:((<pre>", print_r($groups, TRUE), "</pre>))";
	group_list($group);
}
echo "<li>[Add another top-level group]</li>";
?>
</ul>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>

<h2>Add a group</h2>
<!-- Yeah, maybe it's just easier to have a separate form for adding groups. -->
<form name="add-group" method="post" action="groups.php">
  <input name="command" type="hidden" value="add"/>
<!-- XXX - name -->
  Group name: <input name="name" type="text" size="20"/><br/>
<!-- XXX - parent -->
  <input name="parent" type="hidden" value="-1"/>
<!-- XXX - submit button -->
  <input name="add" type="submit" value="Add group"/>
</form>

</body>
</html>
