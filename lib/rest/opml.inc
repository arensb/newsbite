<?php
/* rest_opml
 * OPML-related REST stuff.
 */
require_once("common.inc");		// Mostly for authentication
require_once("database.inc");

class RESTController_opml {

	/* run
	 * Dispatcher for OPML-related requests.
	 *	- GET /opml - Get feeds as an OPML file
	 *	- PUT /opml - Replace feeds with feeds from OPML.
	 *	- POST /opml - Add feeds from OPML file.
	 * We don't really have a lot of commands. Basically, it's just a
	 * matter of manipulating the OPML list as a whole.
	 */
	function run($rreq)
	{
		$cmd = $rreq->shift_pathv();
		switch ($cmd)
		{
		    case NULL:
		    case "":
			switch ($rreq->verb())
			{
			    case "GET":
				return $this->get_opml($rreq);

			    case "POST":
				return $this->post_opml($rreq);

			    case "PUT":
				return $this->put_opml($rreq);

			    default:
				error_log("rest_info: bad verb " . $rreq->verb());
				throw new RESTException(NULL,
					"Bad verb");
			}

			$rreq->finish(200);

		    default:
			error_log("rest_info: bad command " .
				  ($cmd === NULL ? "NULL" :
				   $cmd));
			throw new RESTException(NULL, "Invalid command");
		}
	}

	function get_opml($rreq)
	{
		/* Write the subscription list in OPML 2.0 format.
		 * See http://www.opml.org/spec2
		 */
		$feeds = db_get_feeds();
			// XXX - Error-checking

		// Response is OPML, so it's always XML.

		// XXX - Do we want a special method or anything to
		// allow a REST class to ignore the user's language
		// request?
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

	/* POST_OPML
	 * POST /opml - Batch-subscribe to a bunch of feeds in OPML format.
	 */
	// XXX - The SQL part of this is much much faster if all the
	// subscriptions are batched together:
	//	INSERT INTO feeds (title, feed_url) VALUES
	//		(title1, feed1),
	//		(title2, feed2),
	//		...
	// instead of
	//	INSERT INTO feeds (title, feed_url) VALUES (title1, feed1);
	//	INSERT INTO feeds (title, feed_url) VALUES (title2, feed2);
	//	...
	// But for that we need a db_add_feed()-ish function that'll take
	// multiple feeds.
	function post_opml($rreq)
	{
		global $db_errno;
		global $db_errmsg;

		$err_level = 0;	// 0 = ok, 1 = warning, 2 = error. This is a
			// high-water mark for the code that follows:
			// if there's an error, raise it to 2. If
			// there's a warning, raise it to 1 unless
			// it's already at 2.

		$retval = array();
		$opml = $rreq->body();

		/* Look for the <opml><body><outline> elements.
		 * Get 'xmlurl' for the feed.
		 * Get 'text' for the title.
		 */
		// Find the <body>
		$opml_body = $opml->findChildrenByName("body");
		if (!isset($opml_body))
			// XXX - How to signal to the caller that
			// there was an error?
			return array("status" => "error",
				     "message" => "No <body> in OPML."
				);
		$opml_outlines = $opml_body[0]->findChildrenByName("outline");
		if (!isset($opml_body))
			// XXX - How to signal to the caller that
			// there was an error?
			return array("status" => "error",
				     "message" => "No <outline> elements in OPML."
				);

		// Get the list of feeds we're already subscribed to
		$feeds = db_get_feeds();
		$retval['statuses'] = array();

		// Loop through the <outline>s, looking for the stuff we want.
		foreach ($opml_outlines as $elt)
		{
			$feed_name = $elt->attributes['text'];
			$feed_url  = $elt->attributes['xmlUrl'];
			$elt_status = array(
				"name"	=> $feed_name,
				"url"	=> $feed_url,
				);
			// Check whether we're already subscribed to this feed.
			foreach ($feeds as $f)
			{
				if ($feed_url == $f['feed_url'])
				{
					error_log("We're already subscribed to $feed_url");
					$elt_status['status'] = "ok";
					$elt_status['message'] = "already subscribed";
					array_push($retval['statuses'], $elt_status);
					continue 2;
				}
			}
			/* Add the feed */
			error_log("Subscribing to feed \"$feed_name\" at $feed_url");
			$err = db_add_feed(array("title"	=> $feed_name,
						 "feed_url"	=> $feed_url));
			if ($err)
			{
				$elt_status['status'] = "ok";
			} else {
				$elt_status['status'] = "error";
				$elt_status['message'] = "DB error " .
					$db_errno . ": \"" .
					$db_errmsg . "\"";
				error_log("Database error $db_errno subscribing  to $feed_url: \"$db_errmsg\"");
				$err_level = 2;
			}
			array_push($retval['statuses'], $elt_status);
		}

		/* Record the overall error level for the POST OPML
		 * operation.
		 */
		switch ($err_level)
		{
		    case 0:
			$retval['status'] = "ok";
			break;
		    case 1:
			$retval['status'] = "warning";
			break;
		    case 2:
			$retval['status'] = "error";
			break;
		    default:		// This isn't even a valid error level!
			$retval['status'] = "error";
			$retval['message'] = "Unknown error level $err_level.";
			break;
		}

		return $retval;
	}

	function put_opml($rreq)
	{
		// XXX - Presumably, this should be like POST /opml, except
		// that the subscription list is overwritten rather than
		// edited. That is, make the 'feeds' table match the posted
		// OPML file, i.e., unsubscribe from anything that's not
		// on the list.
	}
}
?>
