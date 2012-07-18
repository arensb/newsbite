<?php
/* pheedo-ads.php
 * Functions to remove pheedo.com ads from the bottom of articles.
 */

function pheedo_ad_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;

#echo "pheedo_ad_hook($nodename) before: [<pre>", htmlentities($retval), "</pre>]<br/>\n";
	// Row of pheedo ads at the bottom, optionally separated by
	// dashes.
	$retval = preg_replace(
		'{<a href=[\'"]http://ads\.pheedo\.com/.*?</a>( - )?}s',
		'',
		$retval);
#echo "text 2: [", $retval, "]\n";

	// Pheedo's Bing image
	$retval = preg_replace(
		'{<img src=[\'"]http://ads\.pheedo\.com/[^>]*/>}',
		'',
		$retval);
#echo "text 3: [", $retval, "]\n";

	// Social sites, I think.
	$retval = preg_replace(
		'{^\s*<a\b.*href=[\'"]http://www.pheedcontent.com/.*</a>\r?\n?}m',
		'',
		$retval);

	// Any number of web bugs
	$retval = preg_replace(
		'{<img[^>]*height="0" width="0"[^>]*/>}s',
		'',
		$retval);
#echo "text 4: [", $retval, "]\n";

	// Removing all of the above leaves a bunch of <br>s at the
	// end.
	$retval = preg_replace(
		'{(\s*<br\b[^>]*/>)+\s*$}s',
		'',
		$retval);
#echo "text 5: [<pre>", htmlentities($retval), "</pre>]<br/>\n";
}

// At least one feed (GraphJam) puts ads in both the summary and the
// content.
add_hook("summary", "pheedo_ad_hook");
add_hook("body", "pheedo_ad_hook");
?>
