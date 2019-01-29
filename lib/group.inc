<?php
/* group.inc
 * Group-related functions.
 */
require_once("lib/common.inc");
require_once("lib/database.inc");

# XXX - A function to return groups, as a tree of groups and feeds.
/* group_tree
 * Return an object representing the feed groups, as a tree.
 *
 * If $feeds is TRUE, include feed IDs in the tree; otherwise, just give
 * the tree structure of groups. (This is like 'find /foo' vs
 * 'find /foo -type d'.)
 */
function group_tree($feeds = TRUE)
{
	# XXX

	// Get (raw-ish) list of groups from the database
	$raw_groups = db_get_groups();
	$feeds = db_get_feeds();	# XXX - Only if $feeds, I guess.

	foreach ($raw_groups as $g)
	{
		if ($g['id'] == -1)
			continue;
		$raw_groups[$g['parent']]['members'][] =
			&$raw_groups[$g['id']];
	}

	$retval = $raw_groups[-1];
	return $retval;
}
?>
