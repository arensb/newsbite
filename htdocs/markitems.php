<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: {$feed.title}</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
</head>
<body>

<?php
require_once("config.inc");
require_once("database.inc");

/* XXX - All this stuff really needs to go above the doctype, so that
 * we can redirect to the REFERER once we're done if all goes well.
 */
$mark_new    = array();
$mark_unread = array();
$mark_read   = array();
foreach ($_REQUEST as $k => $v)
{
	/* Look for POST variables of the form "state_{id}", where
	 * {id} is the ID of the item to be updated.
	 */
	if (!preg_match('/^state_(\d+)$/', $k, $match))
		continue;
	$id = $match[1];
	echo "Need to update id=[$id]: [$v]<br/>\n";
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
?>

<a href="<?=$_SERVER['HTTP_REFERER']?>">Back</a>.

</body>
</html>
