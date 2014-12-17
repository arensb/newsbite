/* group.jsh					-*- JavaScript -*-
 * JavaScript functions for the group-editing page.
 */
#include "js/guess-mobile.js"
// #include "js/defer.js"
#include "js/xhr.js"
/*#include "js/keybindings.js"*/
/*#include "js/PatEvent.js"*/
/*#include "js/types.js"*/
#include "js/Template.js"
/*#include "js/CacheManager.js"*/
//#include "js/load_module.js"
// XXX - Should block multiple updates from occurring in parallel.
/*#include "js/status-msg.js"*/

document.addEventListener("DOMContentLoaded", init, false);

var group_tmpl;		// Template for an entry in the group tree
var group_tree;		// Tree of groups

function init()
{
	group_tmpl = new Template($("#groupentry").html());

	refresh_group_tree();
	$("#add-group-form").submit(add_group);
}

/* refresh_group_tree
 * Fetch the current list of groups from the server, and redraw the tree.
 */
// XXX - The list of groups should be in localStorage.
function refresh_group_tree()
{
	/* _draw_group_tree
	 * Find the #group-tree div, and populate it with a tree of groups.
	 */
	function _draw_group_tree(tree)
	{
		/* Find #group-tree */
		var group_list = $("#group-tree");

		// Remove any children it might have from a previous
		// iteration.
		$(group_list).empty();

		// Append the current list of children (recursively).
		$(group_list)
			.append($.map(tree.members,
				      _draw_members));
		// Defer to draw_members() to generate a <li> element
		// for each group (and its children).
	}


	/* _draw_members
	 * Helper function for draw_group_tree(): given a group, create a <li>
	 * element for it (possibly recursively, if it has children), and
	 * return that.
	 */
	function _draw_members(val, key)
	{
console.log("_draw_members()");
		var li = $(group_tmpl.expand({
			GID:		val.id,
			GROUPNAME:	val.name,
		}));
//var del_button = $(".delete-group-button", li);
//console.log("Delete button: ", del_button);
		if (val.members != undefined)
		{
			var ul = $(".child-groups", li)
				.append("<ul/>");
			ul.children().append($.map(
				$.grep(val.members,
				       function(a) {
					       var retval = a instanceof Object;
					       return retval;
				       }),
				_draw_members));
		}
		return li;
	}

	// refresh_group_tree main:
	get_json_data("group.php",
		      { command:	'tree',
		      },
		      _draw_group_tree,
		      null,
		      false);
}

/* add_group
 * Handler for #add-group-form, the form for adding a group.
 */
function add_group(ev)
{
	ev.preventDefault();		// Don't propagate the event and
					// submit the form.

	/* Get the form fields */
	var name  = this.elements["name"].value;
	var parent = this.elements["parent"].value;
		// XXX - Do we care that 'parent' is a string, not an int?

	// Make AJAX call to create group
	get_json_data("group.php",
		      { command:	"add",
			name:		name,
			parent:		parent
		      },
		      // Handler
		      function(value)
		      {
			      // Update the group tree, above.
			      refresh_group_tree();
		      },
		      // Error handler
		      function(status, msg)
		      {
			      console.error("Failed to create group: error "+
					    status+
					    ", error "+err);
		      },
		      true);
}

/* delete_group
 * Handler for .delete-group-button buttons.
 */
function delete_group(ev)
{
	ev.preventDefault();		// Don't propagate the event and
					// submit the form.
	// XXX
}
