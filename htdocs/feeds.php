<?
/* feeds.php
 * Send list of feeds.
 */
require_once("config.inc");
require_once("common.inc");
require_once("database.inc");

$out_fmt = $_REQUEST['o'];	# Output format

# Make sure the requested output format is sane
if ($out_fmt != 'json')
{
	# 400 == generic bad request
	header("HTTP/1.0 400 I hate you");
	exit(1);
}

# Gather information about the feeds
$feeds = db_get_feeds();
$counts = db_get_all_feed_counts();

# Collect the gathered information into one array
$output = Array();

foreach ($feeds as $id => $data)
{
	$desc = Array();		# Feed description to send to client

	# The fields are enumerated because we don't want to send the
	# username and password to the client
	$desc['id']              = $id;
	$desc['title']           = $data['title'];
	$desc['subtitle']        = $data['subtitle'];
	$desc['nickname']        = $data['nickname'];
	$desc['url']             = $data['url'];
	$desc['feed_url']        = $data['feed_url'];
	$desc['description']     = $data['description'];
	$desc['last_update']     = $data['last_update'];
	$desc['image']           = $data['image'];
	$desc['active']          = $data['active'];
	$desc['num_read']        = $counts['num_read'];
	$desc['num_unread']      = $counts['num_unread'];
	$desc['latest_pub_date'] = $counts['latest_pub_date'];
	$desc['latest_update']   = $counts['latest_update'];

	$output[] = $desc;
}

// XXX - The following assumes that the user picked JSON output, above.
header("Content-type: text/plain+xml; charset=utf-8");
echo "<", '?xml version="1.0" encoding="UTF-8"?', ">\n";
echo "<![CDATA[\n";

echo jsonify($output);

/* Close the "<![CDATA[" from above */
echo "\n]]>\n";
?>
