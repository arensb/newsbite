<?php
/* enclosure_hook
 * Look for enclosures attached to a post, and attach them to the
 * summary or body.
 */
function enclosure_hook($nodename, &$retval, &$context)
{

	if (!isset($retval['enclosure']))
		return;

	foreach ($retval['enclosure'] as $enc)
	{
		$before = FALSE;	# Should enclosure come before text?

#		if (preg_match(',^video/,', $enc['type']))
#			// XXX - Embedding video seems to slow things
#			// down. Turn it off for now.
#			continue;

		// XXX - Perhaps ought to have a drop-down widget for
		// audio and video, or something.
		if (preg_match(',image/,', $enc['type']))
		{
#			$newtext = "<hr/><img src=\"$enc[url]\"/>";
			$newtext = "<img src=\"$enc[url]\"/>\n";
			$before = TRUE;
		} else {
			$newtext = "<hr/>$enc[type]: <a href=\"$enc[url]\">$enc[url]</a>";
		}

		if (isset($retval['description']))
		{
			if ($before)
				$retval['description'] = $newtext . $retval['description'];
			else
				$retval['description'] .= $newtext;
		}
		if (isset($retval['content']))
		{
			if ($before)
				$retval['content'] = $newtext . $retval['content'];
			else
				$retval['content'] .= $newtext;
		}
	}
}

add_hook("item", "enclosure_hook");
?>
