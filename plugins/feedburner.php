<?php
/* feedburner_hook
 * Remove Feedburner link and bug at bottom of postings.
 */
/* XXX - This should go in a separate file, as a plug-in */
function feedburner_hook($nodename, &$retval, &$context)
{
#echo "Inside feedburner_hook($nodename)<br/>\n";
	if (!is_string($retval))
		return;
	/* There are two separate lines that can appear: an ad/survey
	 * link, and a web bug.
	 */
#echo "(feedburner) before: <pre>[", htmlentities($retval), "]</pre>\n";
	$retval = preg_replace('{<div class="feedflare">.*?</div>}s',
			       '',
			       $retval);
	$retval = preg_replace('{<p><a href="http://feedads.g.doubleclick.net.*?</p>}',
			       '',
			       $retval);
#echo "(feedburner) after 1: <pre>[", htmlentities($retval), "]</pre>\n";
	$retval = preg_replace('{<img src="http://feeds\d*.feedburner.com/[^\"]*" height="1" width="1"/>\r?\n?}',
			       '',
			       $retval);
#echo "(feedburner) after 2: <pre>[", htmlentities($retval), "]</pre>\n";
}

add_hook("summary", "feedburner_hook");
add_hook("body", "feedburner_hook");
?>
