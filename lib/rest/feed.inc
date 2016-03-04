<?php
// XXX - Want common.inc in production. Actually, we just want authentication/authorization.
//require_once("common.inc");
require_once("database.inc");

/* feed REST controller
 *	GET /feed	Get information about all feeds
 *	GET /feed/123	Get information about feed id=123
 *	PUT /feed	Subscribe to a feed
 *	DELETE /feed	Unsubscribe from a feed
 *	POST /feed	Update a feed's information
 *
 *	PUT /feed/update	Update all feeds
 *	PUT /feed/update/123	Update feed 123
 */
class RESTController_feed {
	function run($rreq)
	{
		// If the subpath is of the form
		//	123/foo/bar
		// then "123" is taken as the feed ID.
		$feed_id = NULL;
		$subpath = $rreq->subpath();
		if (preg_match (',(\d+)(?:/(.*))?,', $subpath, $matches))
		{
			$feed_id = $matches[1];
			if (count($matches) > 2)
				$subpath = $matches[2];
			else
				$subpath = NULL;
		}

		switch ($subpath)
		{
		    case "":
		    case NULL:
			switch ($rreq->verb())
			{
			    case "GET":
				if (isset($feed_id))
					// Get information about one feed
					return $this->get_feed_info($feed_id, $rreq);
				else
					// Get information about all feeds
					return $this->get_all_feeds_info($rreq);
				break;

			    case "PUT":
				// Subscribe to a feed
				// XXX - Error if $id is set
				return $this->add_feed($rreq);

			    case "DELETE":
				// XXX - Unsubscribe from a feed
				// Error if $id is not set.
				return $this->unsubscribe($rreq);

			    case "POST":
				// XXX - Update information for a feed
				break;
			}
			break;

		    default:
			return array("error" => "What's up with subpath " . $rreq->subpath());
		}
	}

	/* add_feed
	 * Subscribe to a feed.
	 */
	// XXX - Tested with
	// curl -sSv -X PUT -H 'Content-Type: application/json' http://carrot.ooblick.com/newsbite/w1/feed -d '{"url":"http://www.foo.com/dummy/rss","username":"arensb"}'
	function add_feed($rreq)
	{
		$feed_url = $rreq->body()->{'url'};
				// XXX - Error-checking
		$params = array(
			"feed_url"	=> $feed_url,
			);

		// XXX - username
		$username = $rreq->body()->{'username'};
		if (isset($username))
			$params['username'] = $username;

		// XXX - password
		$passwd = $rreq->body()->{'password'};
		if (isset($passwd))
			$params['passwd'] = $passwd;
error_log("ought to db_add_feed(): " . print_r($params, true));

		// XXX - db_add_feed()
		// XXX - db_update_feed()
		return array("status"	=> "ok",
			     "feed"	=> $feed_url,
			     // XXX - Information about the feed?
			);
	}

	/* unsubscribe
	 * Unsubscribe from a feed
	 */
	function unsubscribe($rreq)
	{
		// XXX - Get feed ID
		// XXX - See unsubscribe.php, I think.
		error_log("Inside unsubscribe: " . $rreq->subpath());
		return array("status"	=> "ok");
	}

	/* get_feed_info
	 * Get information about one feed
	 */
	function get_feed_info($feed_id, $rreq)
	{
		// Collect the information to return
		$info = db_get_feed($feed_id);
		$counts = db_get_feed_counts($feed_id);

		// We build this by hand partly because we don't want
		// to send the username and password to the caller.
		$retval = array(
			'id'		=> $feed_id,
			'title'		=> $info['title'],
			'subtitle'	=> $info['subtitle'],
			'nickname'	=> $info['nickname'],
			'url'		=> $info['url'],
			'feed_url'	=> $info['feed_url'],
			'description'	=> $info['description'],
			'last_update'	=> $info['last_update'],
			'image'		=> $info['image'],
			'active'	=> $info['active'],
			'stale'		=> $info['stale'],

			'num_read'	=> $counts['num_read'],
			'num_unread'	=> $counts['num_unread'],
		);

		return $retval;
	}

	/* get_all_feeds_info
	 * Get information about all feeds.
	 */
	function get_all_feeds_info($rreq)
	{
		// Collect the information to return
		$feeds = db_get_feeds(TRUE);
		$counts = db_get_all_feed_counts();

		// Collect the gathered information into one array
		$retval = Array();

		foreach ($feeds as $id => $data)
		{
			$desc = Array();	// Feed description to
						// send to client

			// The fields are enumerated because we don't
			// want to send the username and password to
			// the client
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

			$retval[$id] = $desc;
		}

		return $retval;
	}
}
