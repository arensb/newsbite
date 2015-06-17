<?php
/* Strip a bunch of ads from feeds */

/* googleads_hook
 * Remove Google Ads ads at bottom of postings.
 */
function googleads_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
	$retval = preg_replace('{\r?\n<p>(<a href="http://feedads.g.doubleclick.net/[^\"]*"><img src="http://feedads.g.doubleclick.net/[^\"]*" border="0" ismap="true"></img></a>(<br/>\n\r?)?)*<a href="http://feedads.g.doubleclick.net/[^\"]*"><img src="http://feedads.g.doubleclick.net/[^\"]*" border="0" ismap="true"></img></a></p>}',
					'',
					$retval);
}

add_hook("summary", "googleads_hook");
add_hook("body", "googleads_hook");

/* more_ads_hook
 * Remove yet more ads at bottom of RSS items.
 */
function more_ads_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
	# Visible ad
#echo "more_ads before: <tt>"; print_r($retval);
	$retval = preg_replace('{(<br */>\r?\n?)*<a href="http://(\w+)\.feedsportal.com/.*?</a>}',
			       '',
			       $retval);
#echo "more_ads after: <tt>"; print_r($retval);
}

add_hook("summary", "more_ads_hook");

?>
