<?
/* updates.php
 * Send the client a list of things that have been updated since a
 * given time.
 */
require_once("common.inc");
require_once("database.inc");

/* Check arguments */

/* Make sure output format is sane */
switch ($out_fmt)
{
    case "xml":
    case "json":
	break;
    default:
	# 400 == generic bad request
	header("HTTP/1.0 400 Invalid output format.");
	exit(1);
}

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
elseif ($feed_id == "all")
	;
else {
	header("HTTP/1.0 400 Invalid feed ID.");
	exit(0);
}

/* Make sure t exists and is an integer */
if (!isset($_REQUEST['t']))
{
	header("HTTP/1.0 400 No time specified");
	exit(0);
}
$t = $_REQUEST['t'];
if (is_numeric($t) && is_integer($t+0))
	$t = (int) $t;
else {
	header("HTTP/1.0 400 Invalid time");
	exit(0);
}

/* Get the updates since that time. Limit it to 100 items */
$updates = db_get_item_updates($t, $feed_id, 100);
#print_r($updates);

$retval = array();
while (count($updates) > 0)
{
	// Trim the stuff we don't need: if an article is read, just
	// send the ID, is_read, and mtime. Don't resend the whole
	// article.
	$item = array_shift($updates);
	if ($item['is_read'])
		$item = array(
			"id"		=> $item['id'],
			"is_read"	=> $item['is_read'],
			"mtime"		=> $item['mtime'],
			);
	$retval[] = $item;
}

/* Send the results to the user */
switch ($out_fmt)
{
    case "json":
	echo jsonify($retval);
	break;
    case "xml":
	require_once("xml-output.inc");
				// Get print_xml() only when necessary
	print_xml($retval);
}
db_disconnect();
?>
