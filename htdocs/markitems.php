<?php
require_once("config.inc");
require_once("common.inc");
require_once("database.inc");

$ok = true;	// Error status. If $ok, then we can just redirect to
		// wherever we came from when we're done. Otherwise,
		// need to display an error message.

/* Figure out what we're expected to do */
if (isset($_REQUEST['doit']))
	// Mark each item according to its radio buttons
	$cmd = "mark";
elseif (isset($_REQUEST['mark-all']))
	// Mark all items as read
	$cmd = "mark-all";
elseif (isset($_REQUEST['mark-read']) && isset($_REQUEST['mark-unread']))
	// Lists of items to mark.
	$cmd = "mark-lists";
elseif (count($_REQUEST) == 0)
	abort(<<<EOT
No command given. Perhaps the POST variables were dropped by your web proxy.
Please go back and resubmit.
EOT
		);
else
	abort("No command found; \$_REQUEST == [" .
	      var_export($_REQUEST) .
	      "]\n\$_POST == [" .
	      var_export($_POST) .
	      "]");

/* Ajax code submits lists of items */
if ($cmd == "mark-lists")
{
	header("Content-type: text/plain+xml; charset=utf-8");
		// Assume JSON

	/* Check syntax of $_REQUEST[mark-read] and [mark-unread] */
	if (!preg_match('/^(\d+,)*\d*$/', $_REQUEST['mark-read']))
		abort("mark-read: invalid value");
	if (!preg_match('/^(\d+,)*\d*$/', $_REQUEST['mark-unread']))
		abort("mark-unread: invalid value");

	/* Construct list of items to mark read and unread */
	// XXX - There's some inefficiency here: $_REQUEST[mark-read]
	// is a comma-separated string of numbers. We split it into an
	// array. But then, db_mark_items is going to join them back
	// together into a string. Is this worth fixing?
	$mark_read = explode(",", $_REQUEST['mark-read']);
	$mark_unread = explode(",", $_REQUEST['mark-unread']);

	/* Mark items */
	// The is_numeric() test is there because explode(",", "")
	// gives array(0 => "") instead of an empty array. Feh.
	if (is_numeric($mark_read[0]))
		db_mark_items(true, $mark_read);
		// XXX - Error-checking
	if (is_numeric($mark_unread[0]))
		db_mark_items(false, $mark_unread);
		// XXX - Error-checking

	// XXX - Ought to return status to caller. Ideally, should
	// tell caller the status of each item marked: it's possible
	// that 
	echo jsonify('state',	"ok");
	exit(0);
}

/* Make sure $mark_how has a legal value */
switch ($mark_how)
{
    case "read":
	$how = TRUE;
	break;

    case "unread":
	$how = FALSE;
	break;

    default:
	// This is the result either of programmer error, or illegal
	// input.
	abort("Don't know how to mark items: \"$mark_how\"");
}

$item_ids = array();		// The IDs of the items to mark

switch ($cmd)
{
    case "mark":
	foreach ($_REQUEST as $k => $v)
	{
		/* Look for POST variables of the form "state-{id}",
		 * where {id} is the ID of the item to be updated.
		 */
		if (!preg_match('/^cb[tb]-(\d+)$/', $k, $match))
			continue;
		$item_ids[] = $match[1];
	}
	break;

    case "mark-all":
	foreach ($_REQUEST as $k => $v)
	{
		/* Look for POST variables of the form "item-{id}",
		 * where {id} is the ID of the item to be updated.
		 */
		if (!preg_match('/^item-(\d+)$/', $k, $match))
			continue;
		$item_ids[] = $match[1];
	}
	break;

    default:
	// This should never happen.
	echo "Unknown command [$cmd]. This should never happen.\n";
	exit(1);
}

//echo "\$item_ids: [<pre>"; print_r($item_ids); echo "</pre>]<br/>\n";
//echo "Calling db_mark_items($how, $item_ids)<br/>\n";
db_mark_items($how, $item_ids);
	// XXX - Error-checking

if ($ok)
{
	/* Redirect back to where we came from */
	redirect_to($_SERVER['HTTP_REFERER']);
	exit(0);
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Update items</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
</head>
<body>

<p><b>Error:</b> Something went wrong, but I don't know what.</p>

<a href="<?=$_SERVER['HTTP_REFERER']?>">Back</a>.

</body>
</html>
