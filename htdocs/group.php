<?
/* group.php
 * Edit groups and whatnot.
 */
$NO_AUTH_CHECK = TRUE;		# XXX - Just for testing
require_once("common.inc");
#require_once("database.inc");
require_once("group.inc");
require_once("skin.inc");

/* Get parameters */
$cmd = "";
if (isset($_REQUEST['command']))
	$cmd = $_REQUEST['command'];	// Command. What are we supposed to do?
$group_id = NULL;
if (isset($_REQUEST['id']))
	$group_id = $_REQUEST['id'];	// ID of group, if we're just changing one.
$name = "";
if (isset($_REQUEST['name']))
	$group_name = $_REQUEST['name'];	// Name of group
$parent = NULL;
if (isset($_REQUEST['parent']))
	$group_parent = $_REQUEST['parent'];	// Group's parent

switch ($cmd)
{
    case "":
	/* No command. Start a new page */
	if ($out_fmt == "json")
	{
		echo jsonify('state',	"error",
			     'error',	"No command specified."
			), "\n";
		exit(0);
	}
	if (isset($group_id))
	{
		// XXX - Form for just one group
		show_form_one($group_id);
	} else {
		// XXX - Form for all groups
		show_form_all();
	}
	break;

    case "tree":
	$tree = group_tree(TRUE);
	switch ($out_fmt)
	{
	    case "json":
		echo jsonify($tree);
		break;
	    case "console":
	    case "html":
	    default:
		// XXX - Do we even want to consider these?
		echo "<pre>", print_r($tree, TRUE), "</pre>\n";
	    break;
	}
	break;

    case "add":
	add_group($group_name, $group_parent);
	break;

    case "delete":
	delete_group($group_id);
	break;

    case "reparent":
	echo "<p>Ought to reparent something.</p>";
	break;

    case "update":
	if (isset($group_id))
	{
		/* Update an existing group */
		update_group_info($group_id);
	} else {
		update_group_info();
	}
	break;

    default:
	abort("Invalid action.");
}
exit(0);

function show_form_one($group_id)
{
	$groups = group_tree(FALSE);
	# XXX - Construct form for editing one feed.
}

function show_form_all()
{
	# XXX - Construct form for editing all feeds.
	$groups = group_tree();

	$skin = new Skin();

	$skin->assign('groups', $groups);
	$skin->assign('command', "update");
	$skin->display("editgroups_all");
	return;
}

function update_group_info($group_id = NULL)
{
	# XXX
	global $_REQUEST;

echo "update_group_info: \$_REQUEST:<pre>", print_r($_REQUEST, TRUE), "</pre>\n";
}

function add_group($name, $parent_id = -1)
{
	global $out_fmt;

	$parent = db_get_group($parent_id);
	if ($parent === NULL)
		abort("Invalid parent group");

	# XXX - Is there anything to check wrt the name?
	$err = db_add_group($name, $parent_id);
	if ($err === NULL)
	{
		global $db_errno;
		global $db_errmsg;

		switch ($out_fmt)
		{
		    case "json":
			echo jsonify('state',	"error",
				     'errno',	$db_errno,
				     'error',	$db_errmsg),
				"\n";
			break;
		    case "console":
			echo "Error $db_errno: $db_errmsg\n";
			break;
		    case "html":
		    default:
			echo "<b>Error</b> $db_errno: $db_errmsg<br/>";
			break;
		}
		return;
	}

	switch ($out_fmt)
	{
	    case "json":
		$err['state'] = 'ok';
		echo jsonify($err);
		break;
	    case "console":
		echo "Created group\n";
		break;
	    case "html":
	    default:
		# XXX - Redirect back to self, maybe?
		echo "Created group<br/>\n";
		break;
	}
}

function delete_group($group_id)
{
	global $out_fmt;

	// XXX - Sanity check? Make sure $group_id is an integer,
	// and negative, and a group that exists?

	if (db_delete_group($group_id))
	{
		// Group was deleted successfully
		switch ($out_fmt)
		{
		    case "json":
			echo jsonify('state',	'ok');
			break;
		    case "console":
			echo "Deleted\n";
			break;
		    case "html":
		    default:
			echo "Deleted<br/>\n";
			break;
		}
		return;
	} else {
		// Something went wrong
		switch ($out_fmt)
		{
		    case "json":
			echo jsonify('state',	"error",
				     'errno',	$db_errno,
				     'error',	$db_errmsg),
				"\n";
			break;
		    case "console":
			echo "Error $db_errno: $db_errmsg\n";
			break;
		    case "html":
		    default:
			echo "<p>Error $db_errno: $db_errmsg<p>\n";
			break;
		}
	}
}
?>
