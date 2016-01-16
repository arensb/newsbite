<?php
/* feeds.php
 * Send list of feeds.
 */

#if ($_SERVER['REQUEST_METHOD'] == "PUT")
#{
#  echo "Hey, you tried to PUT something.\n";
#  exit(0);
#}

require_once("common.inc");
require_once("database.inc");

# Make sure the requested output format is sane
if ($out_fmt != 'json' && $out_fmt != "xml")
{
	# 400 == generic bad request
	header("HTTP/1.0 400 I hate you");
	exit(1);
}

# Gather information about the feeds
$feeds = db_get_feeds(TRUE);
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
	$desc['stale']           = $data['stale'];

	$desc['num_read']        = $counts[$id]['read'];
	$desc['num_unread']      = $counts[$id]['unread'];

	$output[$id] = $desc;
}

print_struct($output);		// Send result to the caller
?>
