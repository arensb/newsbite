<?php
/* opml.php
 * Dump subsriptions in OPML format.
 */
// OPML validator: http://validator.opml.org/
require_once("common.inc");
require_once("database.inc");

/* Write the subscription list in OPML 2.0 format.
 * See http://www.opml.org/spec2
 */
$feeds = db_get_feeds();
	// XXX - Error-checking
header("Content-type: text/xml");
header("Content-Disposition: attachment; filename=\"newsbite-subscriptions.xml\"");
echo "<", '?xml version="1.0" encoding="UTF-8"?', ">\n";

// The spec says the <dateCreated> must be in RFC822 format. RFC822
// says to use a two-digit year abbreviation, but the examples at the
// OPML site use four-digit years, which I think is wise.
$date_created = strftime("%a, %d %b %Y %H:%M:%S %Z")
?>
<opml version="2.0">
  <head>
    <title>Newsbites subscription list</title>
    <dateCreated><?=$date_created?></dateCreated>
    <ownerId>http://www.ooblick.com/</ownerId>
    <docs>http://www.opml.org/spec2</docs>
  </head>
  <body>
<?php

foreach ($feeds as $f)
{
	 /* Print the <outline> entry for this feed */
	 // XXX - For htmlspecialchars(), there's probably a special
	 // set of arguments for properly escaping XML, but I guess
	 // the ones for HTML work fine, at least for now.
	 echo "    <outline",
	 	' text="',
		 htmlspecialchars($f['nickname'] == "" ?
				  $f['title'] :
				  $f['nickname']), '"',
		 ' type="rss"',
		 ' xmlUrl="', htmlspecialchars($f['feed_url']), '"',
		 ($f['description'] == "" ? "" :
		  ' description="' . htmlspecialchars($f['description']) . '"'),
		 ' htmlUrl="', htmlspecialchars($f['url']), '"',
		 ' title="', htmlspecialchars($f['title']), '"',
		"/>\n";
}
?>
  </body>
</opml>
