<?php
// XXX - Want common.inc in production. Actually, we just want authentication/authorization.
//require_once("common.inc");
require_once("database.inc");
require_once("net.inc");

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
				if (isset($feed_id))
					// Error if $id is set
					throw new RESTException(NULL, "ID not allowed");
				return $this->add_feed($rreq);

			    case "DELETE":
				if (!isset($feed_id))
					// Error if $feed_id is not set.
					throw new RESTException(NULL, "ID not found");
				return $this->unsubscribe($feed_id, $rreq);

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
	function add_feed($rreq)
	{
		$feed_url = $rreq->body()->{'url'};
				// XXX - Error-checking
		$params = array(
			"feed_url"	=> $feed_url,
			);

		// username
		$username = $rreq->body_param('username');
		if (isset($username))
			$params['username'] = $username;

		// password
		$passwd = $rreq->body_param('password');
		if (isset($passwd))
			$params['passwd'] = $passwd;

		// db_add_feed()
		$feed_id = db_add_feed($params);
		if ($feed_id === false)
			throw new RESTException(NULL, "Error adding feed");

		// XXX - In the REST world, do we really want to
		// update the feed now? Should we try to keep it small
		// and modular, and have "add feed" be separate from
		// "update feed"?
		// XXX - And for that matter, should we just call
		// $this->update_feed() once it's written?

		/* Refresh the new feed, to get info and new articles */
		$err = update_feed($feed_id);
		if (!$err)
			// XXX - Better error reporting: include error message
			throw new RESTException(NULL, "Error updating new feed.");
		if (isset($err['status']) && $err['status'] != 0)
			throw new RESTException($err['status'],
						$err['errmsg']);

		return array("id"	=> $feed_id);
	}

	/* unsubscribe
	 * Unsubscribe from a feed
	 */
	function unsubscribe($feed_id, $rreq)
	{
		error_log("Unsubscribing from feed " . $feed_id);
		/* Go ahead and unsubscribe */
		db_delete_feed($feed_id);
			// XXX - Error-checking?

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

			'num_read'	=> $counts['read'],
			'num_unread'	=> $counts['unread'],
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
