<?php
/* feedburner_origlink_hook
 * Feedburner replaces the URL to a story with its own URL, presumably
 * in order to collect statistics, or something. Thankfully, it leaves
 * a "<feedburner:origLink>" element. This allows us to replace the
 * story's URL with the original, so we don't have to leapfrog through
 * feedproxy.google.com.
 */
function feedburner_origlink_hook($nodename, &$retval, &$context)
{
	if (isset($retval['feedburner:origLink']))
	{
		if (isset($retval['url']))
			$retval['oldurl'] = $retval['url'];
		$retval['url'] = $retval['feedburner:origLink'];
	}
}

add_hook("item", "feedburner_origlink_hook");

/* pheedo_origlink_hook
 * Same as above, with a different name.
 */
function pheedo_origlink_hook($nodename, &$retval, &$context)
{
	if (isset($retval['pheedo:origLink']))
	{
		if (isset($retval['url']))
			$retval['oldurl'] = $retval['url'];
		$retval['url'] = $retval['pheedo:origLink'];
	}
}

add_hook("item", "pheedo_origlink_hook");
?>
