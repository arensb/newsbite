<?php
require_once("lib/common.inc");		// Mostly for authentication
require_once("lib/database.inc");
require_once("lib/group.inc");

/* group REST controller
 *	GET /group	Get all groups
 *	GET /group/-123	Get information about group -123
 *	PUT /group	Create a group
 *	DELETE /group/-123	Delete a group
 *	POST /group/-123	Rename a group
 *		name=$newname
 *	POST /group/-123	Reparent a group?
 *		parent=$newparent
 */
// XXX - Or do we want
//	POST /group/-123/name	Rename a group
//	POST /group/name/-123	Rename a group
//	POST /group/-123/parent	Reparent a group
//	POST /group/parent/-123	Reparent a group
class RESTController_group {
	function run($rreq)
	{
		$cmd = NULL;
		$group_id = NULL;

		// XXX - Is this structure,
		//	VERB /class(/subclass)?(/id)?
		// common enough that we want to put it in the parent?
		$arg = $rreq->pathv(0);
		if ($arg == "")
		{
			// VERB /class
			$cmd = NULL;
			$group_id = NULL;
		} elseif (is_numeric($arg) && is_integer($arg+0))
		{
			// VERB /class/ID
			$group_id = $rreq->shift_pathv();
			$cmd = NULL;
		} else {
			// VERB /class/subclass
			// VERB /class/subclass/ID
			$cmd = $rreq->shift_pathv();
			$arg = $rreq->pathv(0);
			if (is_numeric($arg) && is_integer($arg+0))
			{
				// VERB /class/subclass/ID
				$group_id = $rreq->shift_pathv();
			}
		}

		switch ($cmd)
		{
		    case "":
		    case NULL:
			switch ($rreq->verb())
			{
			    case "GET":
				if (isset($group_id))
					// GET /group/-123
					// Get information about one group
					return $this->get_group_info($group_id, $rreq);
				else
					// GET /group
					// Get information about all groups
					return $this->get_all_groups_info($rreq);
				break;

			    case "PUT":
				// Create a group
				if (isset($group_id))
					// Error if $id is set
					throw new RESTException(NULL, "ID not allowed");
				// PUT /group
				//	&parent_id=-123
				return $this->add_group($rreq);

			    case "DELETE":
				if (!isset($group_id))
					// Error if $group_id is not set.
					throw new RESTException(NULL, "ID not found");
				// DELETE /group/-123
				// Delete group -123. Reparent its
				// children.
				return $this->delete_group($group_id, $rreq);

			    case "POST":
				if (!isset($group_id))
					// Error if $group_id is not set.
					throw new RESTException(NULL, "ID not found");
				// POST /group/-123
				// Update information for a group
				return $this->set_group_info($group_id, $rreq);
			}
			break;

		    default:
			throw new RESTException(NULL, "Bad command $cmd");
		}
	}

	/* get_group_info
	 * Get information about a single group.
	 */
	function get_group_info($group_id, $rreq)
	{
		$info = db_get_group($group_id);
		if ($info === NULL)
			throw new RESTException("Bad group ID");
		return $info;
	}

	function get_all_groups_info($rreq)
	{
		return group_tree(TRUE);
	}

	function add_group($rreq)
	{
		$name = $rreq->body_param('name');
			// XXX - Error-checking
		$parent_id = $rreq->body_param('parent_id');
		if (!isset($parent_id))
			// Defaults to -1, the root group.
			$parent_id = -1;
		if (!is_numeric($parent_id) || !is_integer($parent_id+0))
			throw new RESTException(NULL, "Invalid parent group ID");

		$parent = db_get_group($parent_id);
		if ($parent === NULL)
			abort("Invalid parent group");

		// XXX - Is there anything to check wrt the name?
		$new_group = db_add_group($name, $parent_id);
		if ($new_group === NULL)
		{
			global $db_errno;
			global $db_errmsg;

			return array('state',	"error",
				     'errno',	$db_errno,
				     'error',	$db_errmsg);
		}

		$new_group['state'] = "ok";
		return $new_group;
	}

	function delete_group($group_id, $rreq)
	{
		/* Sanity check. Make sure the group ID is an integer.
		 * We don't care whether such a group exists, because
		 * we're about to delete in any case.
		 */
		if (!is_numeric($group_id) || !is_integer($group_id+0))
			throw new RESTException(NULL, "Invalid group ID");
		if (db_delete_group($group_id))
		{
			// Group was deleted successfully
			return array('state'	=> 'ok');
		} else {
			return array('state',	"error",
				     'errno',	$db_errno,
				     'error',	$db_errmsg);
		}
	}
}
?>
