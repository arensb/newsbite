<?php
/* loadopml.php
 * Load an OPML subscription list.
 */
// XXX - Perhaps add an OPML logo somewhere:
// <a href="http://validator.opml.org/?url=http%3A%2F%2Fwww.ooblick.com%2F~arensb%2Fnewsbite.opml"><img src="http://images.scripting.com/archiveScriptingCom/2005/10/31/valid3.gif" width="114" height="20" border="0" alt="OPML checked by validator.opml.org."></a>

// XXX - Perhaps add a OPML_MAX_SIZE option to config.inc?

// XXX - Should this be split into HTML and REST, or something?

$out_fmt = "html";
require_once("common.inc");
require_once("database.inc");

if (!isset($_FILES['opml']) || $_FILES['opml'] == ""):
	// Prompt for an OPML file
########################################
	echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Load OPML</title>
<link rel="stylesheet" type="text/css" href="css/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="css/opml.css" media="all" />
<script type="text/javascript" src="js/jquery.js"></script>
<meta name="theme-color" content="#8080c0" />
</head>
<body>

<form enctype="multipart/form-data" action="loadopml.php" method="post">
  <!-- 100,000 chars should be enough for ~500 feeds -->
  <input type="hidden" name="max_file_size" value="100000" />
  OPML file:
  <input type="file" name="opml"/>
  <br/>
  <input type="submit" name="doit" value="Upload file"/>
</form>

<button id="opml-button">Upload OPML</button>

<script type="text/javascript">
$(document).ready(function(){
	$("#opml-button").on("click", function(ev) {
		console.log("Clicked on button.");
		var req = new XMLHttpRequest();
		req.open("POST", "w1/opml", true);
		// XXX - Why is "PUT" not allowed?
		req.setRequestHeader("Content-Type",
				     "application/json; charset=UTF-8");
		req.send(JSON.stringify({
				  foo: "bar",
				}));
	});
});
</script>
</body>
</html>
<?php
########################################
	exit(0);
endif;

$load = $_FILES['opml'];	// Name of OPML file to load

/* Load the file */
$text = file_get_contents($load['tmp_name']);

/* Parse the file using XML Parser */

/* Initialize parser */
$xml_parser = xml_parser_create();
	// XXX - Error-checking
xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, false);
xml_set_element_handler($xml_parser,
			_opml_element_start,
			FALSE);

$opml = array();	// Will hold list of subscriptions
$opml_urls = array();	// URLs found in the subscription list

/* Parse the OPML file. _opml_element_start() adds each <outline>
 * entry to the $opml array.
 */
$err = xml_parse($xml_parser, $text, true);
	// XXX - Error-checking

$feeds = db_get_feeds();	// Get a list of all feeds

/* Find the feeds we're not already subscribed to, i.e., the ones we
 * want to subscribe to now.
 */
foreach ($opml as $o)
{
	// XXX - Prettier output
	echo "Checking <tt>$o[xmlurl]</tt><br/>\n";

	/* See whether we're already subscribed to this feed */
	foreach ($feeds as $f)
	{
		if ($o['xmlurl'] == $f['feed_url'])
		{
			// XXX - Prettier output
			echo "    <b>Already subscribed</b><br/>\n";
			continue 2;
		}
	}
	// XXX - Prettier output
	echo "  <i>Subscribing</i><br/>\n";

	/* Add the feed */
	$err = db_add_feed(array("title"	=> $o['text'],
				 "feed_url"	=> $o['xmlurl']));
		// XXX - Error-checking
}

function _opml_element_start($parser, $fullname, $attrs)
{
	global $opml;
	global $opml_urls;

	// XXX - Would be nice to handle categories at some point.
	if ($fullname != "outline")
		// We only care about <outline> elements.
		return;
	// Can't skip entries that don't have a type="rss" attribute,
	// because LJ exports its OPML files with only 'text' and
	// 'xmlURL'.

	// $entry will be appended to $opml. It's basically $attrs,
	// but with every attribute normalilzed to lower case, since
	// different OPML generators use different capitalization
	$entry = array();

	/* Lowercase all of the attributes */
	foreach ($attrs as $k => $v)
	{
		$entry[strtolower($k)] = $v;
	}

	/* Make sure there's a feed URL */
	if ($entry['xmlurl'] == "")
		// This entry doesn't have a URL. Ignore it.
		return;

	/* Append the entry and the URL to our lists */
	$opml[] = $entry;
	$opml_urls[] = $entry['xmlurl'];
}
?>
