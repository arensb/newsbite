<?php
/* no_style_link_hook
 * At least one feed includes
 * <![CDATA[<link rel="stylesheet".../>]]>
 * This is pants-on-head retarded, so cut it out.
 */
function no_style_link_hook($nodename, &$retval, &$context)
{
	$retval = preg_replace('{<link rel=\"stylesheet\"(.*?)(/>|</link>)}',
			       '',
			       $retval);
}

add_hook("summary", "no_style_link_hook");
add_hook("body", "no_style_link_hook");
?>
