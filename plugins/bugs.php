<?php
/* Remove web bugs (images of size 0x0 or 1x1) from feeds */

/* webbug_hook
 * Remove web bugs: images with width=1 and height=1
 */
/* XXX - This should go in a separate file, as a plug-in */
function webbug_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
#echo "Inside webbug_hook($nodename)<br/>\n";
#echo "(webbug) before: <pre>[", htmlentities($retval), "]</pre>\n";
	$retval = preg_replace('{<img(\s+\w+=\"[^\"]*\")* (height="[01]" width="[01]"|width="[01]" height="[01]")(\s+\w+=\"[^\"]*\")*\s*/>\r?\n?}',
			       '',
			       $retval);

	# Same thing, but with single quotes instead of double
	$retval = preg_replace('{<img(\s+\w+=\'[^\']*\')* (height=\'[01]\' width=\'[01]\'|width=\'[01]\' height=\'[01]\').*?/>\r?\n?}',
			       '',
			       $retval);
#echo "(webbug) after 2: <pre>[", htmlentities($retval), "]</pre>\n";
}

add_hook("summary", "webbug_hook");
add_hook("body", "webbug_hook");
?>
