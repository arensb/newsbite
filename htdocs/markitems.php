<?php
require_once("config.inc");
require_once("database.inc");

$ok = true;	// Error status. If $ok, then we can just redirect to
		// wherever we came from when we're done. Otherwise,
		// need to display an error message.

$mark_new    = array();
$mark_unread = array();
$mark_read   = array();
foreach ($_REQUEST as $k => $v)
{
	/* Look for POST variables of the form "state-{id}", where
	 * {id} is the ID of the item to be updated.
	 */
	if (!preg_match('/^state-(\d+)$/', $k, $match))
		continue;
	$id = $match[1];
	switch ($v[0])
	{
	    case "n":		// Mark as new
		$mark_new[] = $id;
		break;
	    case "u":		// Mark as unread
		$mark_unread[] = $id;
		break;
	    case "r":		// Mark as read
		$mark_read[] = $id;
		break;
	    default:
		echo "Unknown update status: [$v]<br/>\n";
		break;
	}
}

db_mark_items("new",    $mark_new);
db_mark_items("read",   $mark_read);
db_mark_items("unread", $mark_unread);

if ($ok)
{
	/* Redirect back to where we came from */
	header('Location: ' . $_SERVER['HTTP_REFERER']);
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
