<?php
// XXX - Want common.inc in production. Actually, we just want authentication/authorization.
//require_once("common.inc");
require_once("database.inc");


/* article REST controller
 *	GET /article
 *		&feed={123|all}
 *		&group=123
 *		&limit=100
 *		&read		Include read articles
 *		XXX - Should this be broken down into, say,
 *		GET /article/feed?feed_id=123
 *		GET /article/group?gid=-123
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
				if (isset($art_id))
					// GET /article/123
					// Get one article
					// XXX - This isn't actually used,
					// but it's easy to implement.
					return $this->get_article($art_id, $rreq);
				else
					// GET /article
					// Get information about all groups
					// XXX - Write this
					return $this->get_articles($rreq);
				break;

			    default:
				throw new RESTException(NULL, "Invalid verb");
			}
			break;

		    case "read":	// VERB /article/read
			switch ($rreq->verb())
			{
			    case "POST":
				if (isset($art_id))
					// Error if $art_id is not set.
					throw new RESTException(NULL, "ID not found");
				// XXX - POST /article/read
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
}
?>
