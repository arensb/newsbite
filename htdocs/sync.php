<?php
/* sync.php
 */
require_once("common.inc");
require_once("database.inc");

$retval = array();

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
elseif ($feed_id == "all")
	;
else
	abort("Invalid feed ID: \"$feed_id\".");

$ihave = json_decode($_REQUEST['ihave'], TRUE);
$json_err = json_last_error();

// XXX - Parse $ihave to make sure it's sane. In particular, 'id' must be an
// integer.
$ids = array();
// Make sure $ihave is an array.
if (!is_array($ihave))
	$ihave = array();

foreach ($ihave as $id => &$value)
{
	// Make sure the client-supplied id is an integer.
	if (is_integer($id))
		$ids[] = $id;
	else {
//		fwrite($fh, "[$id] is not an integer\n");
		continue;
	}

	$value['is_read'] = ($value['is_read'] ? TRUE : FALSE);
	$mtime = strtotime($value['mtime']);
		// XXX - Error-checking. What if mtime is an invalid
		// time?
	if ($mtime === FALSE)
		$mtime = 0;	// Go back to the epoch
	$value['mtime'] = $mtime;
}

// Look up the items from $ihave in the database
$db_items = db_get_items($ids);
$db_items_new = array();
foreach ($db_items as $item)
{
	$db_items_new[$item['id']] = $item;
}
$db_items = $db_items_new;
unset($db_items_new);

// Go through $ids and figure out what to do.
// is_read: keep the newest value. If $ihave is wrong, add a record
//	to $retval.
//	If $db_items is wrong, mark the item in the database.
foreach ($ids as $id)
{
	// We've done data sanitizing earlier, so we know that $id is an
	// integer.

	if (!isset($db_items[$id]))
	{
		// This item doesn't exist in the database.
		// Tell the client to delete it.
		$retval[] = array(
			"id"		=> $id,
			"action"	=> "delete",
			);
		continue;
	}

	$ihave_item = &$ihave[$id];
	$db_item = &$db_items[$id];

	// See whether $ihave and $db_items disagree as to whether an item is
	// read.
	if ($ihave_item['is_read'] xor $db_item['is_read'])
	{
		// See whether $ihave or $db_items has the newer version of
		// the item.
		if ($ihave_item['mtime'] > $db_item['mtime'])
		{
			// $ihave has the more recent version
			db_mark_items($ihave_item['is_read'], array($id));
				// XXX - Would it be more efficient to
				// collect the IDs and 'is_read's of all
				// the items to mark, and mark them all
				// at once?
			// Make sure the client hears about this.
			$retval[] = array("id"		=> $id,
					  "is_read"	=> $ihave_item['is_read'],
					  "mtime"	=> $ihave_item['mtime']
				);
		} else {
			// $db_items has the more recent version
			// In case of a tie, I guess the database wins.
			// Tell the client that its idea of the is_read
			// state is wrong.
			$retval[] = array("id"		=> $id,
					  "is_read"	=> $db_item['is_read'],
					  "mtime"	=> $db_item['mtime']
				);
		}
	}
}

// Get items with db_get_some_feed_items(), like items.php.
// If there are any that don't appear in $ihave, add them to $retval.
$get_feed_args = array(
	"read"		=> "unread",
	"max_items"	=> 100,	// XXX - What's the best value?
	);
if (is_integer($feed_id))
	$get_feed_args['feed_id'] = $feed_id;
$all_items = db_get_some_feed_items($get_feed_args);
foreach ($all_items as $item)
{
	// XXX - items.php converts URLs to mobile versions for some sites.
	// Need to do the same here.

	if (isset($ihave[$item['id']]))
		// The client already has this item.
		continue;

	array_push($retval, $item);
	// XXX - Do we need to run clean-html on anything?
	// Fixed-length fields in 'items'.
}

	print_struct($retval);		// Send result to the caller
?>
