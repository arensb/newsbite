<?php
/* hooks.inc
 * Functions for dealing with hooks.
 */
require_once("lib/config.inc");

/* hooks
 * The entries in this array are hook names; the values are arrays of
 * function names to call for that hook. That is, if the hook is
 * called "parse-hook", and we want to call functions somefunc() and
 * otherfunc() on that hook, then $hooks will contain:
 *
 * $hooks == array("parse-hook" =>
 *		array("somefunc",
 *		      "otherfunc"));		
 */
$hooks = array();

/* url2handler
 * Maps the URL for a given namespace to its handler.
 * Elements of this array are of the form
 *	$URL => array(
 *			"init" => namespace_initialization,
 *			"element" => element_handler,
 *			"attribute" => attribute_handler,
 *			"text" => text_handler,
 *		),
 */
$url2handler = array();

/* load_hooks
 * Read all of the *.php files in a directory, and load them.
 */
function load_hooks($dir)
{
	if (!is_dir($dir))
		return FALSE;

	@$files = scandir($dir);
		// Get the list of files, so we don't feed a non-array
		// to foreach, below, if $dir doesn't exist or isn't
		// readable.
	if ($files === false)
		return FALSE;
	foreach ($files as $file)
	{
		// Ignore anything that isn't a file, or that doesn't
		// end in ".php".
		if (!is_file("$dir/$file") ||
		    strrchr($file, ".php") != ".php")
			continue;

		/* Include the file */
		$err = include_once("$dir/$file");
			// XXX - If the file fails to load,
			// include_once will return false. What do we
			// want to do about this?
	}

	return TRUE;
}

/* add_hook
 * Add a function to a hook.
 */
function add_hook($hook, $func)
{
	global $hooks;

	$hooks[$hook][] = $func;	// Append the function
}

/* add_xml_handler
 * Very similar to add_hook, but for XML namespaces. Takes a namespace
 * identifier, like http://purl.org/rss/1.0/modules/content/ , and
 * sets its handler, an array of the form:
 *	array("init"	=> init_function,
 *	      "element"	=> element_handler_function,
 *	      "attribute"	=> attribute_handler_function,
 *	      "text"	=> text_handler_function);
 */
/* XXX - This is only used in lib/parse-feed.inc, to deal with XML
 * elements in feeds. It lives here because there are functions in
 * plugins/* that use it, but there are other scripts that want to use
 * plugins.

 * There probably should be a distinction between plain hooks and
 * functions that deal with XML elements.
 */
function add_xml_handler($namespace, $handler)
{
	global $url2handler;

	$url2handler[$namespace] = $handler;
}

/* run_hooks
 * Run all the functions on a given hook.
 * The first argument is name of the function to run; the second is an
 * array of arguments to pass to the function.
 *
 * Note on references: if a hook function expects an argument to be
 * passed by reference, $args must be explicitly called with a
 * reference to a variable:
 *	function hook_func(&$ref_arg) { ... };
 *	add_hook("myhook", "hook_func");
 *	run_hooks("myhook",
 *		array($&some_var));
 */
function run_hooks($hook, $args)
{
	global $hooks;

	if (!isset($hooks[$hook]) ||
	    !is_array($hooks[$hook]))
		// No such hook
		return NULL;

	// Call each hook in turn
	foreach ($hooks[$hook] as $h)
		call_user_func_array($h, $args);
			// XXX - Perhaps ought to define an error
			// value for the hook to return; and if it
			// does, do something intelligent.

	return true;
}
?>
