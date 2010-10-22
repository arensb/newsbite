<?
/* Hook for parsing the media XML namespace:
 * http://search.yahoo.com/mrss/
 */

function media_element_handler(
	$ns_url,	// URL of this element's namespace
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	&$context)	// Where we are at the moment
{
	$parent = $context[count($context)-1]['name'];

	switch ($elt_name)
	{
	    case "thumbnail":
		# Thumbnail for a post, presumably
		$retval['media:thumbnail'] = $attrs['url'];
		break;
		// XXX - Other elements seen in the wild:
		// category - Ignore
		// content - Attached pictures/audio/whatever
		// credit - Include in caption?
		// description - Hover text? Caption?
		// group - Group of related images
		// keywords - Ignore
		// player - Useful for video
		// thumbnail - Perhaps use for non-image media
		// title - Hover text? Caption?
		//
		// XXX - Watch out with media:player: the example at
		// http://video.search.yahoo.com/mrss uses it as a
		// singleton, but the only place I've seen it in the
		// wild is at blip.tv, which puts a CDATA section
		// inside (giving the <embed> element to use.
	    case "content":
		// XXX
		break;
	    default:
		// Ignore
		// XXX - Is there anything else we'd want?
		break;
	}

	return true;
}

/* Register this handler with the XML parser */
add_xml_handler("http://search.yahoo.com/mrss/",
		array(
		"element"	=> media_element_handler,
		)
	);
?>
