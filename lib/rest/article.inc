<?php
require_once("common.inc");		// Mainly for authorization.
require_once("database.inc");

/* article REST controller
 *	GET /article/123
 *	POST /article/latest
 *		feed_id={123|all}	Defaults to "all"
 *		n=100			Defaults to 100
 *		group=$gid
 *	POST /article/read
 *		Mark article(s) as read/unread
 */
// XXX - Should
// DELETE /article/123
// mean "mark article 123 as read"?
class RESTController_article {
	function run($rreq)
	{
		$cmd = NULL;
		$art_id = NULL;

		// XXX - Is this structure,
		//	VERB /class(/subclass)?(/id)?
		// common enough that we want to put it in the parent?
		$arg = $rreq->pathv(0);
		if ($arg == "")
		{
			// VERB /class
			$cmd = NULL;
			$art_id = NULL;
		} elseif (is_numeric($arg) && is_integer($arg+0))
		{
			// VERB /class/ID
			$art_id = $rreq->shift_pathv();
			$cmd = NULL;
		} else {
			// VERB /class/subclass
			// VERB /class/subclass/ID
			$cmd = $rreq->shift_pathv();
			$arg = $rreq->pathv(0);
			if (is_numeric($arg) && is_integer($arg+0))
			{
				// VERB /class/subclass/ID
				$art_id = $rreq->shift_pathv();
			}
		}

		switch ($cmd)
		{
		    case "":
		    case NULL:
			switch ($rreq->verb())
			{
			    case "GET":
				if (!isset($art_id))
					throw new RESTException(NULL, "ID not found");
					// GET /article/123
					// Get one article.
					return $this->get_article($art_id, $rreq);
				break;

			    default:
				throw new RESTException(NULL, "Invalid verb");
			}
			break;

		    case "latest":
			switch ($rreq->verb())
			{
			    case "POST":
				if (isset($art_id))
					// Error if $art_id is set.
					throw new RESTException(NULL, "ID not found");
				return $this->get_articles($rreq);
				break;
			}
			break;

		    case "read":	// VERB /article/read
			switch ($rreq->verb())
			{
			    case "POST":
				if (isset($art_id))
					// Error if $art_id is set.

					// XXX - Perhaps eventually add
					// POST /article/read/123?read={true|false},
					// for marking individual articles?
					throw new RESTException(NULL, "ID not allowed");
				// POST /article/read
				// Mark articles as read/unread
				return $this->mark_articles($rreq);

			    default:
				throw new RESTException(NULL, "Invalid verb");
			}
			break;

		    default:
			throw new RESTException(NULL, "Bad command $cmd");
		}
	}

	/* get_article
	 * Get one article. This isn't actually used anywhere, but
	 * hey, it's easy to implement.
	 */
	function get_article($art_id, $rreq)
	{
		$ids = array($art_id);
		$articles = db_get_items($ids);
		if (count($articles) == 0)
			return NULL;
		return $articles[0];
	}

	/* get_articles
	 * Get the latest N articles, either from a specific feed, or
	 * from all feeds.
	 *	n	Max. number of articles to return (default: 100)
	 *	ihave	array of article IDs.
	 * If $ihave is given, return the latest N articles that
	 * aren't in $ihave.
	 */
	// XXX - base36-encoding in PHP:
	//	base_convert(1000000, 10, 36) == "lfls"
	//	base_convert("lfls", 36, 10) == 1000000
	// In JavaScript:
	//	(1000000).toString(36) == "lfls"
	//	parseInt("lfls", 36) == 1000000
	function get_articles($rreq)
	{
		// Get feed id (NULL->all)
		$feed_id = $rreq->body_param('feed_id');

		if (!isset($feed_id))
			// Defaults to "all"
			$feed_id = "all";
		elseif ($feed_id == "all")
			// Okay
			;
		else {
			// Make sure feed ID is sane.
			if (!is_numeric($feed_id) ||
			    !is_integer($feed_id+0))
				throw new RESTException(NULL, "Invalid feed ID");
			// Apparently is_numeric("123"), but not
			// is_integer("123"). But if we add 0, we turn
			// the string into an ingeger. Let's do that,
			// because that'll make life simpler.
			$feed_id += 0;
		}

		// Get max-articles
		$max_articles = $rreq->body_param('n');
		if (!isset($max_articles))
			// Defaults to 100
			$max_articles = 100;
		else
			// Make sure $max_articles is sane.
			if (!is_numeric($max_articles) ||
			    !is_integer($max_articles+0))
				throw new RESTException(NULL, "Invalid parameter n");

		// XXX - (Eventually: Get group ID)

		// Get ihave[]
		// This is expected to be an array of item IDs
		// (integers).
		$ihave = $rreq->body_param('ihave');
		if (!isset($ihave))
			$ihave = array();
		if (!is_array($ihave))
			throw new RESTException(NULL, "Not an array");

		$retval = array();

		// Get items with db_get_some_feed_items(). If there
		// are any that don't appear in $ihave, add them to
		// $retval.
		$get_feed_args = array(
			"read"		=> "unread",
			"max_items"	=> count($ihave) + $max_articles,
					// The +$max_articles is so
					// that even if the first 100
					// articles are in $ihave,
					// we'll get the next 100 to
					// return to the client.
			);
		if (is_integer($feed_id))
			$get_feed_args['feed_id'] = $feed_id;
		$all_items = db_get_some_feed_items($get_feed_args);

		$n = 0;
		foreach ($all_items as $item)
		{
			if (in_array($item['id'], $ihave))
			{
				// The client already has this item.

				// XXX - How fast is in_array()? Do we
				// want to do some kind of bulk
				// comparison, or something?
				continue;
			}


			array_push($retval, $item);
				// XXX - Do we need to run clean-html
				// on anything? Fixed-length fields in
				// 'items'.
			if (++$n > $max_articles)
				break;
		}
		return $retval;
	}

	/* mark_articles()
	 * Take an array of articles and statuses, update the database
	 * where necessary, and send back to the client a list of
	 * corrections.
	 */
	function mark_articles($rreq)
	{
error_log("Inside mark_articles()");
		// Get $ihave
		// This is a hash of the form $id => [is_read?, mtime]
		// $ihave == {
		//	123 => array(true, $mtime),
		//	456 => array(false, $mtime2),
		//	...
		// }

		$ihave = $rreq->body_param('ihave');
		if (!isset($ihave))
		{
			error_log("mark_articles() without an ihave");
			throw new RESTException(NULL, "Missing ihave parameter");
		}
		if (!is_object($ihave) && !is_array($ihave))
			throw new RESTException(NULL, "Not a hash");
		$client_ids = array();
		foreach ($ihave as $key => &$value)
			// In this foreach loop, note that &$value is
			// by-reference, because we might be changing
			// it.
		{
			// Make sure each ID is reasonable
			if (!is_numeric($key) || !is_integer($key+0))
				throw new RESTException(NULL, "Invalid ID");
			$client_ids[] = $key;

			// XXX - Check $value, perhaps, to make sure
			// it has a boolean and a non-negative
			// integer?

			// mtime is probably a date. If so, convert it
			// to a time_t.
			if (!is_numeric($value[1]) || !is_integer($value[1]+0))
				$value[1] = strtotime($value[1]);
		}

		// Get the articles in $ihave
		$db_items = db_get_items($client_ids);

		// Convert $db_items from an array of items, to a hash
		// indexed by item ID.
		$db_items_new = array();
		foreach ($db_items as $item)
		{
			$db_items_new[$item['id']] = $item;
		}
		$db_items = $db_items_new;
		unset($db_items_new);

		// Compare client's $ihave against the database
		$retval = array();
		foreach ($client_ids as $id)
		{
			// If $db_items doesn't have $id, the article has
			// been deleted. Tell the client that.
			if (!isset($db_items[$id]))
			{
				$retval[] = array(
					"id"		=> $id,
					"action"	=> "delete",
					);
				continue;
			}

			// If the caller and database agree on an
			// article's is_read status, all is well.
			// Send back the is_read status, and its mtime.
			if ((bool) $ihave[$id][0] ==
			    (bool) $db_items[$id]['is_read'])
			{
				# XXX - If $ihave[id][1] is false, as
				# has been known to happen, then this will
				# send back false/undefined as the mtime.
				$retval[] = array("id"		=> $id,
						  "feed_id"	=> (int) $db_items[$id]['feed_id'],
						  "is_read"	=> (bool) $ihave[$id][0],
						  "mtime"	=> $ihave[$id][1]
					);
				continue;
			}

			/* By this point, the caller and database disagree */
			if ($ihave[$id][1] > $db_items[$id]['mtime'])
			{
error_log("Update the database");
				// Update the database
				db_mark_items((bool) $ihave[$id][0],
					      array($id));
				// XXX - Would it be more efficient to
				// collect the IDs and 'is_read's of all
				// the items to mark, and mark them all
				// at once?

				// Make sure the client hears about this.
				$retval[] = array("id"		=> $id,
						  "is_read"	=> (bool) $ihave[$id][0],
						  "mtime"	=> $ihave[$id][1]
					);
			} else {
error_log("Update the client");
				// Database is more recent than the
				// client. In case of a tie, the
				// database wins.
				//
				// Tell the client that its idea of
				// the is_read state is wrong.
				$retval[] = array("id"		=> $id,
						  "is_read"	=> $db_items[$id]['is_read'],
						  "mtime"	=> $db_items[$id]['mtime']
					);
			}
		}

		// Send differences back to the client.
#error_log("Returning results: " . print_r($retval, true));
		return $retval;
	}
}
?>
