<?
// XXX - This should go in a separate plugin
/* gocomics_hook
 * GoComics.com feeds don't include the comic image. This hook guesses
 * the image from the feed URL, and inserts it into the body of each
 * item.
 */
function gocomics_hook($nodename, &$retval, &$context)
{
	/* Make sure this is a GoComics.com feed:
	 * the site URL should be of the form
	 * http://www.gocomics.com/$comic
	 */
	if (!preg_match(',^http://www.gocomics.com/(.*)/?$,', $retval['url'],
			$matches))
		return;
	$comic = $matches[1];

	/* GoComics image URLs are of the form
	 * http://picayune.uclick.com/comics/lio/2008/lio080905.gif
	 * ^                                 ^        ^
	 * prefix                            suffix   suffix
	 * The prefix is constant. The suffix is guessed from $comic
	 * in the switch statement below.
	 */
	$base_url = "http://picayune.uclick.com/comics/";
	switch ($comic)
	{
	    case "lio":
		// Lio
		$suffix = "lio";
		break;
	    case "nonsequitur":
		// Non Sequitur
		$suffix = "nq";
		break;
	    case "tomthedancingbug":
		// Tom the Dancing Bug
		$suffix = "td";
		break;
	    default:
		/* Unknown comic. Abort */
		return;
	}

	/* Add the image to each item.
	 * The obvious way to do this would've been to make this an
	 * <item> hook rather than an <rss> hook, but then we would've
	 * had to reach down into the context stack to figure out
	 * which comic we're dealing with. This way, we can just look
	 * at the feed URL.
	 */
	foreach ($retval['items'] as &$item)
	{
		/* Construct the image URL */
		$img_url = $base_url . $suffix . "/" .
			strftime("%Y/{$suffix}%y%m%d.gif",
				 strtotime($item['title']));

		/* Add the image to the item content. Normally,
		 * there's no content, but we'll append to it anyway,
		 * just in case the format changes in the future.
		 */
		$item['content'] .= "<div><img src=\"$img_url\"/></div>";
	}
}

add_hook("feed", "gocomics_hook");
?>
