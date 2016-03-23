/* group.jsh					-*- JavaScript -*-
 * JavaScript functions for the group-editing page.
 */
#include "guess-mobile.js"
// #include "defer.js"
#include "xhr.js"
#include "rest.js"
/*#include "keybindings.js"*/
/*#include "PatEvent.js"*/
/*#include "types.js"*/
#include "Template.js"
/*#include "CacheManager.js"*/
//#include "load_module.js"
// XXX - Should block multiple updates from occurring in parallel.
/*#include "status-msg.js"*/

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
	function _draw_group_tree(err, errmsg, tree)
	{
		// XXX - Check err to make sure the call was successful.

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
		var li = $(group_tmpl.expand({
			GID:		val.id,
			GROUPNAME:	val.name,
		}));

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

		/* Add a handler to the "Delete" button.
		 * The ">" in the selector matters: if 'li' is a group
		 * with children, then we don't want to attach this
		 * event handler to their buttons.
		 */
		$(li).on("click", ">.delete-group-button",
			 {gid: val.id},
			 delete_group);


		/* Add a handler to the "Edit" button.
		 * Again, the ">" in the selector prevents us from
		 * adding this event handler to buttons in children of
		 * 'li'.
		 */
		$(li).on("click", ">.edit-group-button",
			 {gid: val.id},
			 edit_group);

		// XXX - Make groups draggable, so they can be reparented.

		return li;
	}

	// refresh_group_tree main:
	REST.call("GET", "group", undefined,
		  _draw_group_tree,
		  function(err, errmsg){
			  console.log("GET /group error: "+err+": "+errmsg);
		  });
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

/* edit_group
 * Handler for .edit-group-button buttons.
 */
function edit_group(ev)
{
	ev.preventDefault();		// Don't propagate the event and
					// submit the form.
	// XXX
console.log("Inside edit_group, gid: ", ev.data.gid);

	var this_entry = $(ev.target).parent(".group-entry");
console.log("this entry: ", this_entry);
	var parent_entry = this_entry.parents(".group-entry:first");
console.log("parent entry: ", parent_entry);
parent_entry.css("outline", "1px solid red");
	var child_entries = this_entry.children(".child-groups");
console.log("child entries: ", child_entries);
child_entries.css("outline", "1px solid blue");

	// XXX - Update the tree to make name editable.
	// XXX - Make an AJAX call to edit the group.
	// XXX - If successful, update the group in DOM.
	// XXX - Update the copy of the group tree in local storage.
	// XXX - Update feeds in local storage?
}

/* delete_group
 * Handler for .delete-group-button buttons.
 */
function delete_group(ev)
{
	ev.preventDefault();		// Don't propagate the event and
					// submit the form.
	// XXX
	var gid = ev.data.gid;
console.log("Inside delete_group, gid: ", gid);

	var this_entry = $(ev.target).parent(".group-entry");
	var parent_entry = this_entry.parents(".group-entry:first");
	var child_entries = this_entry.children(".child-groups");

	// XXX - Make an AJAX call to delete the group.
	get_json_data("group.php",
		      { command:	"delete",
			id:		gid,
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
			      console.error("Failed to delete group: error "+
					    status+
					    ", error "+err);
		      },
		      true);

	// XXX - Update the copy of the group tree in local storage.
	// XXX - Update feeds in local storage?
}
