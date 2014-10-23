<?php
/* Remove utm_* trackers in URLs.
 */

/* deutmify_link
 * Try to remove all of the "utm_<foo>=<value>" parameters in a URL.
 * These are related to Google AdSense.
 */
function deutmify_link($link)
{
	$newlink = preg_replace('{\b([\?\&]utm_\w+=([^\&]*|\"[^\"]*\"))}',
			       '',
			       $link);
#echo "deutmify_link([$link] [$newlink])<br/>\n";
	$link = $newlink;
}

add_hook("post_link", "deutmify_link");
?>
