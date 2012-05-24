<?php
/* Strip a bunch of ads from feeds */

/* googleads_hook
 * Remove Google Ads ads at bottom of postings.
 */
/* XXX - This should go in a separate file, as a plug-in */
function googleads_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;
#echo "Inside googleads_hook($nodename)<br/>\n";
#echo "(googleads) before: <pre>[", htmlentities($retval), "]</pre>\n";
#	$retval = preg_replace('{\r?\n<p><a href="http://feedads.googleadservices.com/[^\"]*"><img src="http://feedads.googleadservices.com/[^\"]*" border="0" ismap="true"></img></a></p>}',
#					'',
#					$retval);
#echo "(googleads) after 1: <pre>[", htmlentities($retval), "]</pre>\n";
	$retval = preg_replace('{\r?\n<p>(<a href="http://feedads.g.doubleclick.net/[^\"]*"><img src="http://feedads.g.doubleclick.net/[^\"]*" border="0" ismap="true"></img></a>(<br/>\n\r?)?)*<a href="http://feedads.g.doubleclick.net/[^\"]*"><img src="http://feedads.g.doubleclick.net/[^\"]*" border="0" ismap="true"></img></a></p>}',
					'',
					$retval);
#echo "(googleads) after 2: <pre>[", htmlentities($retval), "]</pre>\n";
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
#echo "(more ads) before: <pre>[", htmlentities($retval), "]</pre>\n";
	# Visible ad
	$retval = preg_replace('{(<br */>\r?\n?)*<a href="http://da.feedsportal.com/.*?</a>}',
			       '',
			       $retval);

	# Ads at the bottom of Onion articles
	# Apparently the "mf-viral" class is sometimes in single quotes,
	# and sometimes in double quotes.
	$retval = preg_replace('{<div class=.mf-viral.>(.*?)</div>}',
			       '',
			       $retval);
#echo "(more ads) after 1: <pre>[", htmlentities($retval), "]</pre>\n";
}

add_hook("summary", "more_ads_hook");

?>
