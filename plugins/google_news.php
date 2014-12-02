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

function ungooglify_link($link)
{
#echo "ungooglify before: [$link]<br/>\n";
	$retval = preg_replace('{http://news.google.com/.*url=(.*)}',
			       '\1',
			       $link);
# XXX - Was this urldecode() in here for a reason?
# It's commented out because if a URL has a + in it, it gets turned
# into a space. And Kinja uses "http://.../+authorname" in a lot of URLs.
#	$retval = urldecode($retval);
#echo "ungooglify after: [$retval]<br/>\n";
	return $retval;
}

/* google_news_origlink
 * Google News puts its own wrapper in front of story URLs.
 */
function google_news_origlink($nodename, &$retval, &$context)
{
#echo "google news origlink before: [$retval]<br/>\n";
	$retval = preg_replace('{<a href="http://news.google.com/[^"]*url=([^"&]*)[^"]*">}',
			       '<a href="\\1">',
			       $retval);
#echo "google news origlink after: [$retval]<br/>\n";
}

function google_news_link_hook(&$link)
{
	$link = ungooglify_link($link);
}

#add_hook("body", "google_news_origlink");
add_hook("summary", "google_news_origlink");
add_hook("post_link", "google_news_link_hook");
?>
