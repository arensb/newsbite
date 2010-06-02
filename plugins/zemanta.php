<?php
/* zemanta_hook
 * Remove zemanta web bug.
 */
function zemanta_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
#echo "(zemanta) before: <pre>["; print_r($retval); echo "]</pre>\n";
	$retval = preg_replace('{<div[\s\n]+class="zemanta-pixie".*?</div>}s',
			       '',
			       $retval);
#echo "(zemanta) after 2: <pre>["; print_r($retval); echo "]</pre>\n";
}

add_hook("summary", "zemanta_hook");
add_hook("body", "zemanta_hook");
?>
