<?php
/* Hook to remove "Featured Advertiser" entries in WaPo.
 */
function wapo_ads($nodename, &$retval, &$context)
{
	# If the title is "Featured Advertiser", mark it as read.
	if (isset($retval['title']) && $retval['title'] == "Featured Advertiser")
		$retval['is_read'] = true;

	# Mark sports articles as read.
	if (isset($retval['wp:web-link']) &&
	    preg_match(',^http://www.washingtonpost.com/sports/,',
		       $retval['wp:web-link']))
		$retval['is_read'] = true;

	return true;
}

add_hook("item", "wapo_ads");	# Register hook
?>
