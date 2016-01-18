<?php
$default_fmt = "json";
require_once("common.inc");
require_once("database.inc");

# Check parameters
if (!isset($_REQUEST['mark-read']) && !isset($_REQUEST['mark-unread']))
{
	# Need to provide 'mark-read' and/or 'mark-unread'.
	header("HTTP/1.0 400 Invalid parameters");
		# XXX - Is there a better status code?
	exit(1);
}

header("Content-type: text/plain; charset=utf-8");
	// Assume JSON

/* Check syntax of $_REQUEST[mark-read] and [mark-unread] */
if (!preg_match('/^(\d+,)*\d*$/', $_REQUEST['mark-read']))
	abort("mark-read: invalid value");
if (!preg_match('/^(\d+,)*\d*$/', $_REQUEST['mark-unread']))
	abort("mark-unread: invalid value");

/* Construct list of items to mark read and unread */
// XXX - There's some inefficiency here: $_REQUEST[mark-read] is a
// comma-separated string of numbers. We split it into an array. But
// then, db_mark_items is going to join them back together into a
// string. Is this worth fixing?
$mark_read = explode(",", $_REQUEST['mark-read']);
$mark_unread = explode(",", $_REQUEST['mark-unread']);

/* Mark items */
// The is_numeric() test is there because explode(",", "") gives
// array(0 => "") instead of an empty array. Feh.
if (is_numeric($mark_read[0]))
	db_mark_items(true, $mark_read);
	// XXX - Error-checking
if (is_numeric($mark_unread[0]))
	db_mark_items(false, $mark_unread);
	// XXX - Error-checking

// XXX - Ought to return status to caller. Ideally, should tell caller
// the status of each item marked: it's possible that something
// changed behind the scenes.
$status = array('state' => 'ok');
print_struct($status);
?>
