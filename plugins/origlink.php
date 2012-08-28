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

		# Apparently some feeds have multiple <feedburner:origLink>
		# elements (Pharyngula is the top offender, with 5). In the
		# cases I've seen, the first one links to the original post,
		# and the others are just links back to feedburner.
		#	At any rate, we need to check for this; otherwise,
		# the post's 'url' winds up being the string "Array".
		if (is_array($retval['feedburner:origLink']))
			$retval['url'] = $retval['feedburner:origLink'][0];
		else
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
