<?php
require_once("lib/common.inc");		// Mostly for authentication
require_once("lib/database.inc");
require_once("lib/net.inc");

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
		/* At this point, pathv might be:
		 *	1) ""
		 *	2) 123
		 *	3) update
		 *	4) update/123
		 * In cases 2 and 4, "123" is the feed ID.
		 * In cases 3 and 4, $cmd is "update";
		 * in cases 1 and 2, $cmd is "".
		 */
		$cmd = NULL;
		$feed_id = NULL;

		$arg = $rreq->pathv(0);
		if ($arg == "")
		{
			// GET /feed
			$cmd = NULL;
			$feed_id = NULL;
		} elseif (is_numeric($arg) && is_integer($arg+0))
		{
			// GET /feed/123
			$feed_id = $rreq->shift_pathv();
			$cmd = NULL;
		} else {
			// GET /feed/update
			// GET /feed/update/123
			$cmd = $rreq->shift_pathv();
			$arg = $rreq->pathv(0);
			if (is_numeric($arg) && is_integer($arg+0))
			{
				// GET /feed/update/123
				$feed_id = $rreq->shift_pathv();
			}
		}

		switch ($cmd)
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
				// Update information for a feed
				if (!isset($feed_id))
					// Error if $feed_id is not set.
					throw new RESTException(NULL, "ID not found");
				return $this->set_feed_info($feed_id, $rreq);
			}
			break;

		    case "update":
			switch ($rreq->verb())
			{
			    case "PUT":
				if (isset($feed_id))
					return $this->update_feed($feed_id, $rreq);
				else
					return $this->update_all_feeds($rreq);

			    default:
				throw new RESTException(NULL, "Invalid verb");
			}
			break;

		    default:
			throw new RESTException(NULL, "Bad command $cmd");
		}
	}

	/* add_feed
	 * Subscribe to a feed.
	 */
	function add_feed($rreq)
	{
		$feed_url = $rreq->body_param('url');
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

	/* set_feed_info
	 * Update a feed's information. The feed must already exist.
	 * Body parameters:
	 *	- nickname: feed's nickname (displayed instead of title)
	 *	- url: site URL
	 *	- feed_url: RSS feed URL
	 *	- active: boolean: should the feed be displayed/updated?
	 *	- username: username, for logging in
	 *	- passwd: password, for logging in
	 *	- groups: array of gids that the feed is in (explicitly)
	 */
	function set_feed_info($feed_id, $rreq)
	{
		// XXX - Blatantly stolen from editfeed.php get_feed_info().
		$feed_info = db_get_feed($feed_id);
		if ($feed_info === NULL)
			throw new RESTException(NULL, "Invalid feed ID: $feed_id");

		/* Build an assoc of new values */
		$new = array();
		// I'm not sure why or how $_REQUEST values acquire
		// backslashes in front of quotation marks, but they do.
		$new['nickname'] = $rreq->body_param('nickname');
		$new['url']      = $rreq->body_param('url');
		$new['feed_url'] = $rreq->body_param('feed_url');
		$new['active']   = $rreq->body_param('active') ? TRUE : FALSE;
		$new['username'] = $rreq->body_param('username');
		$new['passwd']   = $rreq->body_param('password');
error_log("new is now " . print_r($new, true));

		// XXX - Perhaps try to fetch the feed if the feed URL,
		// username, or password has changed?

		$ok = true;
		$errors = array();

		// XXX - Check parameters.

		if (!$ok)
		{
			// XXX - This is rather theoretical. We don't actually
			// check anything.
			throw new RESTException(NULL, "You supplied a bad value of some kind. Go back and fix it.");
		}

		/* Update the list of groups that the feed is in:
		 * $old_groups is the groups the feed is currently in.
		 * $new_groups is the ones it should be in, as gleaned from
		 * $_REQUEST.
		 * Diff the two to see whichg groups to remove the feed from,
		 * and which ones to add it to.
		 */
		// XXX - Should this go in a separate method? Like
		// PUT /feed/123/groups? For that matter, should all
		// of these be settable this way?
		$new_groups = $rreq->body_param('groups');
		if (isset($new_groups))
		{
			// Check $new_groups: it should be an array of
			// (negative) integers.
			if (!is_array($new_groups))
				throw new RESTException(NULL, "groups isn't an array");
			foreach ($new_groups as $gid)
			{
				if (!is_numeric($gid) || !is_integer($gid+0))
					throw new RESTException("non-integer GID");
				if ($gid > 0)
					throw new RESTExceptino("non-negative GID");
			}

			$old_groups = $feed_info['groups'];

			/* Compare $old_groups and $new_groups, and see what
			 * needs to be changed.
			 */
			// First diff: see which groups to remove the feed from.
			$diffs = array_diff($old_groups, $new_groups);
			if (count($diffs) != 0)
			{
				foreach ($diffs as $gid)
				{
					// Remove feed $feed_id from group $g
					db_group_remove_member($gid, $feed_id);
				}
			}

			// Second diff: see which groups to add the feed to.
			$diffs = array_diff($new_groups, $old_groups);
			if (count($diffs) != 0)
			{
				foreach ($diffs as $gid)
				{
					echo "Add feed $feed_id to group $g<br/>\n";
					db_group_add_member($gid, $feed_id);
				}
			}
		}

		/* No errors. Update the database. */
		db_update_feed_info($feed_id, $new);
			// XXX - Error-checking

		return array("status"	=> "ok");
	}

	/* update_feed
	 * PUT /feed/update/123
	 * Update a feed
	 */
	function update_feed($feed_id, $rreq)
	{
error_log("Updating feed $feed_id");

		$feed = db_get_feed($feed_id);
		if ($feed === NULL)
		{
			// Invalid feed ID, presumably..
			error_log("Invalid feed ID: $feed_id");
			throw new RESTException(NULL, "Invalid feed ID: $feed_id");
		}

		$err = update_feed($feed_id, $feed);
		if (!$err)
		{
			// This should never happen
			error_log("Unexpected error: " .
				  "update_feed() returned " .
				  ($err == "" ?
				   var_export($err) : $err));
			throw new RESTException(NULL,
						"update_feed() returned " .
						($err == "" ?
						 var_export($err) : $err))
				;
		}

		/* Error-checking */
		if (isset($err['status']) && $err['status'] != 0)
		{
			error_log("Error: returning { feed_id $feed_id, error " . $err['errmsg'] . "}");
			return array('state'	=> "error",
				     'feed_id'	=> $feed_id,
				     'error'	=> $err['errmsg'],
				);
		}

		// XXX - This "state: end" business is bogus. Replace
		// it with a proper state, maybe?
		return array('state'	=> "end",
			     'feed_id'	=> $err['id'],
			     'title'	=> $err['title'],
			     'counts'	=> $err['counts'],
			);
	}

	/* update_all_feeds
	 * PUT /feed/update
	 */
	function update_all_feeds($rreq)
	{
		$handler = new cumulative_handler();
		update_all_feeds($handler);
		return $handler->finish();
	}
}
