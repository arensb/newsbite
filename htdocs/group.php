<?
/* group.php
 * Edit groups and whatnot.
 */
require_once("common.inc");
#require_once("database.inc");
require_once("group.inc");
require_once("skin.inc");

/* XXX - Should take a command and do something intelligent.
 * - create new group
 * - delete existing group
 * - rename group
 * - move a group to a different parent
 */

$groups = group_tree(TRUE);
print_r($groups);
?>
