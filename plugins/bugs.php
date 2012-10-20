<?php
/* webbug_hook
 * Remove web bugs: images with width=1 and height=1 or less.
 */
function webbug_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
	$retval = preg_replace('{<img(\s+\w+=\"[^\"]*\")* (height="[01]" width="[01]"|width="[01]" height="[01]")(\s+\w+=\"[^\"]*\")*\s*/>\r?\n?}',
			       '',
			       $retval);

	# Same thing, but with single quotes instead of double
	$retval = preg_replace('{<img(\s+\w+=\'[^\']*\')* (height=\'[01]\' width=\'[01]\'|width=\'[01]\' height=\'[01]\').*?/>\r?\n?}',
			       '',
			       $retval);
}

add_hook("summary", "webbug_hook");
add_hook("body", "webbug_hook");
?>
