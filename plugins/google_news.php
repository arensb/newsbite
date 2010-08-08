<?php
/* Fix Google News feed */

/* no_style_hook
 * Remove <font style=...> tags in Google News
 */
function google_news_hook($nodename, &$retval, &$context)
{
#echo "Inside no_style_hook($nodename)<br/>\n";
	if (!is_string($retval))
		return;
#echo "(no style) before: <pre>[", htmlentities($retval), "]</pre>\n";
	$retval = preg_replace('{<font[^>]*>}',
			       '',
			       $retval);
	$retval = preg_replace('{</font\s*>}',
			       '',
			       $retval);
#echo "(no style) after 1: <pre>[", htmlentities($retval), "]</pre>\n";
}

add_hook("summary", "google_news_hook");
#add_hook("body", "google_news_hook");
?>