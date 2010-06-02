<?php
/* wordpress_noise_hook
 * Like googleads_hook, removes the image links and web bug that
 * wordpress adds to the bottom of postings.
 */
function wordpress_noise_hook($nodename, &$retval, &$context)
{
#echo "Inside wordpress_noise_hook($nodename)<br/>\n";
	if (!is_string($retval))
		return;
#echo "wordpress before: <pre>", htmlentities($retval), "</pre>\n";
	# Image links
	$retval = preg_replace('{\s*<a rel="nofollow" href="http://feeds.wordpress.com/1.0/[^\"]*"><img alt="" border="0" src="http://feeds.wordpress.com/1.0/[^\"]*" /></a>}',
			       '',
			       $retval);
#echo "wordpress after 1: <pre>", htmlentities($retval), "</pre>\n";

	# Stats-tracking bug
	$retval = preg_replace('{\s*<img alt="" border="0" src="http://stats.wordpress.com/b.gif[^\"]*" />}',
			       '',
			       $retval);
#echo "wordpress after 2: <pre>", htmlentities($retval), "</pre>\n";
}

add_hook("body", "wordpress_noise_hook");
?>
