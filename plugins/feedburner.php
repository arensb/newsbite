<?php
/* feedburner_hook
 * Remove Feedburner link and bug at bottom of postings.
 */
function feedburner_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
	/* There are two separate lines that can appear: an ad/survey
	 * link, and a web bug.
	 */
	$retval = preg_replace('{<div class="feedflare">.*?</div>}s',
			       '',
			       $retval);
	$retval = preg_replace('{<p><a href="http://feedads.g.doubleclick.net.*?</p>}',
			       '',
			       $retval);
	$retval = preg_replace('{<img src="http://feeds\d*.feedburner.com/[^\"]*" height="1" width="1"/>\r?\n?}',
			       '',
			       $retval);
}

add_hook("summary", "feedburner_hook");
add_hook("body", "feedburner_hook");
?>
