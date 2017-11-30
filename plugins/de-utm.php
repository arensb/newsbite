<?php
/* Remove utm_* trackers in URLs.
 */

/* deutmify_link
 * Try to remove all of the "utm_<foo>=<value>" parameters in a URL.
 * These are related to Google AdSense.
 */
function deutmify_link(&$link)
{
#error_log("deutmify: old [$link]");
	$newlink = preg_replace('{\b((?:[\?\&]|\&amp;)utm_\w+=([^\&]*|\"[^\"]*\"))}',
			       '',
			       $link);
#echo "deutmify_link([$link] [$newlink])<br/>\n";
	$link = $newlink;
#error_log("deutmify: new [$link]");
}

add_hook("post_link", "deutmify_link");
?>
