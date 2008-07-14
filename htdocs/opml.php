<?php
/* opml.php
 * Functions to create and parse OPML files.
 */
/* OPML validator: http://validator.opml.org/ */
// XXX - Perhaps add an OPML logo somewhere:
// <a href="http://validator.opml.org/?url=http%3A%2F%2Fwww.ooblick.com%2F~arensb%2Fnewsbite.opml"><img src="http://images.scripting.com/archiveScriptingCom/2005/10/31/valid3.gif" width="114" height="20" border="0" alt="OPML checked by validator.opml.org."></a>
require_once("config.inc");
require_once("database.inc");

dump_opml();

/* dump_opml
 * Write the subscription list in OPML 2.0 format.
 * See http://www.opml.org/spec2
 */
function dump_opml()
{
	$feeds = db_get_feeds();
//	echo "feeds: [<pre>"; print_r($feeds); echo "</pre>]<br/>\n";
	header("Content-type: text/xml");
	echo "<", '?xml version="1.0" encoding="UTF-8"?', ">\n";

	// The spec says the <dateCreated> must be in RFC822 format.
	// RFC822 says to use a two-digit year abbreviation, but the
	// examples at the OPML site use four-digit years, which I
	// think is wise.
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
		 // XXX - For htmlspecialchars(), there's probably a
		 // special set of arguments for properly escaping
		 // XML, but I guess the ones for HTML work fine, at
		 // least for now.
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
<?php
}

/* XXX - Later ======================================== */
function parse_opml($str)
{
	/* Initialize parser */
	$xml_parser = xml_parser_create();
		// XXX - Error-checking
	xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, false);
	xml_set_element_handler($xml_parser,
				_opml_element_start,
				_opml_element_end);
	xml_set_character_data_handler($xml_parser, _opml_cdata);
}

function _opml_element_start($parser, $fullname, $attrs)
{
}

function _opml_element_end($parser, $fullname)
{
}

function _opml_cdata($parser, $data)
{
}
?>
