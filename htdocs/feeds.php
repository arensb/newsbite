<?
/* feeds.php
 * Send list of feeds.
 */
require_once("common.inc");
require_once("database.inc");
require_once("hooks.inc");

load_hooks(PLUGIN_DIR);

$want_stats = isset($_REQUEST['s']);
				# Add stats (# read/unread items)
				# if requested.
				# Generating stats is currently slow.

# Make sure the requested output format is sane
if ($out_fmt != 'json' && $out_fmt != "xml")
{
	# 400 == generic bad request
	header("HTTP/1.0 400 I hate you");
	exit(1);
}

# Gather information about the feeds
$feeds = db_get_feeds(TRUE);
if ($want_stats)
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

#echo "before hooks: <pre>", $desc['description'], "</pre>\n";
	run_hooks("clean-html", array(&$desc['title']));
	run_hooks("clean-html", array(&$desc['subtitle']));
	run_hooks("clean-html", array(&$desc['description']));
#echo "after hooks:  <pre>", $desc['description'], "</pre>\n";
	# XXX - What else needs to be cleaned up?

	if ($want_stats)
	{
		$desc['num_read']        = $counts[$id]['read'];
		$desc['num_unread']      = $counts[$id]['unread'];
	}

	$output[$id] = $desc;
}

if ($out_fmt == "json")
{
	header("Content-type: text/plain; charset=utf-8");

	echo jsonify($output);
} elseif ($out_fmt == "xml")
{
	require_once("xml-output.inc");
				// Get print_xml() only when necessary

	header("Content-type: text/xml; charset=utf-8");
	print_xml($output);
}
?>
