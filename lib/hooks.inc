<?php
/* hooks.inc
 * Functions for dealing with hooks.
 */
require_once("config.inc");

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

/* run_hooks
 * Run all the functions on a given hook.
 * The first argument is the hook to run; all subsequent arguments
 * will be passed to the hook functions.
 */
function run_hooks($hook)
{
	global $hooks;

	$args = func_get_args();
	array_shift($args);	// Remove $hook

	if (!isset($hooks[$hook]) ||
	    !is_array($hooks[$hook]))
		// No such hook
		return NULL;

	foreach ($hooks[$hook] as $h)
		call_user_func_array($h, $args);

	// XXX - This should probably return something
}
?>
