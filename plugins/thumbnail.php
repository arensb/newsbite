<?php
/* If an item has a thumbnail image attached to it, put it in the body */

function thumbnail_hook($nodename, &$retval, &$context)
{
#echo "Inside thumbnail_hook($nodename)<br/>\n";
#echo "<pre>"; print_r($retval); echo "</pre>\n";
	if (!isset($retval['media:thumbnail']))
		return;
#echo "There's a thumbnail: <img src=\"",
#$retval['media:thumbnail'], "\"/><br/>\n";
#echo "There's a thumbnail: [", $retval['media:thumbnail'], "]<br/>\n";

	$thumb = "<img class=\"thumbnail\" src=\"".
		$retval['media:thumbnail'] .
		"\"/>\n";

	if (isset($retval['content']))
		$retval['content'] = $thumb . $retval['content'];
	if (isset($retval['summary']))
		$retval['content'] = $thumb . $retval['summary'];
	if (isset($retval['description']))
		$retval['content'] = $thumb . $retval['description'];

	return true;
}

add_hook("item", "thumbnail_hook");

?>
