<?php
/* rest_opml
 * OPML-related REST stuff.
 */
require_once("common.inc");
require_once("database.inc");

/* opml_stuff
 * Dispatcher for OPML-related requests.
 *	- GET /opml - Get feeds as an OPML file
 *	- PUT /opml - Replace feeds with feeds from OPML.
 *	- POST /opml - Add feeds from OPML file.
 * We don't really have a lot of commands. Basically, it's just a
 * matter of manipulating the OPML list as a whole.
 */
function opml_stuff($rreq)
{
error_log("inside opml_stuff");
error_log("verb: " . $rreq->verb());
error_log("classname: " . $rreq->classname());
error_log("subpath: " . $rreq->subpath());
	switch ($rreq->subpath())
	{
	    case NULL:
	    case "":
		switch ($rreq->verb())
		{
		    case "GET":
			return GET_OPML($rreq);

		    case "POST":
			return POST_OPML($rreq);

		    case "PUT":
			return PUT_OPML($rreq);

		    default:
			error_log("rest_info: bad verb " . $rreq->verb());
			throw new RESTInvalidVerb();
		}

		$rreq->finish(200);

	    default:
		error_log("rest_info: bad subpath " .
			  ($rreq->subpath() === NULL ? "NULL" :
			   $rreq->subpath()));
		throw new RESTInvalidCommand();
	}
}

function GET_OPML($rreq)
{
	/* Write the subscription list in OPML 2.0 format.
	 * See http://www.opml.org/spec2
	 */
	$feeds = db_get_feeds();
			// XXX - Error-checking

	// Response is OPML, so it's always XML.

	// XXX - Do we want a special method or anything to allow a REST
	// class to ignore the user's language request?
	header("Content-type: text/xml");
	header("Content-Disposition: attachment; filename=\"newsbite-subscriptions.xml\"");
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

function POST_OPML($rreq)
{
error_log("Inside POST_OPML");
	$opml = $rreq->body();

	/* Look for the <opml><body><outline> elements.
	 * Get 'xmlurl' for the feed.
	 * Get 'text' for the title.
	 */
	// Find the <body>
	$opml_body = $opml->findChildrenByName("body");
	if (!isset($opml_body))
		return array("status" => "error",
			     "message" => "No <body> in OPML."
			);
	$opml_outlines = $opml_body[0]->findChildrenByName("outline");
	if (!isset($opml_body))
		return array("status" => "error",
			     "message" => "No <outline> elements in OPML."
			);

	// Get the list of feeds we're already subscribed to
	$feeds = db_get_feeds();

	// Loop through the <outline>s, looking for the stuff we want.
	foreach ($opml_outlines as $elt)
	{
error_log("Found element [" . $elt->attributes['text'] . "], url [" . $elt->attributes['xmlUrl'] . "]");
		// Check whether we're already subscribed to this feed.
		foreach ($feeds as $f)
		{
			if ($elt->attributes['xmlUrl'] == $f['feed_url'])
			{
				error_log("-- We're already subscribed to this feed.");
				continue 2;
			}
		}
		/* Add the feed */
error_log("++ Ought to subscribe to this feed");
//		$err = db_add_feed(array("title"	=> $o['text'],
//					 "feed_url"	=> $o['xmlurl']));
		// XXX - Error-checking
	}
return array("status" => "ok");
}

function PUT_OPML($rreq)
{
	// XXX - Presumably, this should be like POST /opml, except
	// that the subscription list is overwritten rather than
	// edited. That is, make the 'feeds' table match the posted
	// OPML file, i.e., unsubscribe from anything that's not
	// on the list.
}
?>
