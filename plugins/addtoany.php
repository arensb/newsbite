<?php
/* addtoany.php
 * Remove addtoany.com social links
 */

function strip_addtoany($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
#echo "a2a before: <tt>"; print_r($retval);
	$retval = preg_replace('{^<p><a class="a2a_dd.*}m',
			       '',
			       $retval);
#echo "a2a after: <tt>"; print_r($retval);
}

#add_hook("summary", "strip_addtoany");
add_hook("body", "strip_addtoany");
?>
