<?
/* group.php
 * Edit groups and whatnot.
 */
require_once("common.inc");
require_once("database.inc");
require_once("skin.inc");

/* XXX - Should take a command and do something intelligent.
 * - create new group
 * - delete existing group
 * - rename group
 * - move a group to a different parent
 */

$groups = db_get_groups();

echo "groups: <pre>", htmlspecialchars(print_r($groups, TRUE)), "</pre>";

listgroup(-1);

function listgroup($gid)
{
	global $groups;

	$group = $groups[$gid];
	$children = array();
	foreach ($groups as $child)
	{
		if ($child['parent'] == $gid && $child['id'] != -1)
			$children[] = $child['id'];
	}
echo "children: ", print_r($children, TRUE), "<br/>\n";
	echo "<span class=\"groupname\">",
		htmlspecialchars($group['name']),
		"</span>";
	if (count($children) > 0)
	{
		echo "<ul>";
		foreach ($children as $c)
		{
			$child = $groups[$c];
			echo "<li>";
			listgroup($c);
			echo "</li>";
		}
		echo "</ul>";
	}
}
?>
