<?
/* whatsread.php
 * Send the client a list of things that have been marked read since a
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
$updates = db_get_read_items($t, 100);
#print_r($updates);

/* Send the results to the user */
switch ($out_fmt)
{
    case "json":
	echo jsonify($updates);
	break;
    case "xml":
	require_once("xml-output.inc");
				// Get print_xml() only when necessary
	print_xml($updates);
}
db_disconnect();
?>
