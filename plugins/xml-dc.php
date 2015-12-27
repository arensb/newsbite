<?php
/* Plugin for the RSS Dublin Core extension:
 * Handle the RSS Dublin Core extension:
 * http://purl.org/dc/elements/1.1/
 * http://dublincore.org/documents/dces/
 *
 * <dc:creator>name</dc:creator>
 *	The author of the post
 */

/* dc_element_handler
 */
function dc_element_handler(
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
	    case "creator":
		// XXX - Make sure $chldren is a string?
		$retval['author_name'] = $children;
		break;

	    case "date":
		switch ($parent)
		{
		    case "channel":
		    case "item":
			$retval['pub_time'] = strtotime($children);
			break;
		    default:
			// Dunno what to do with this
			break;
		}
		break;

	    default:
//		echo "Warning: unknown \"dc:\" element: [$elt_name]\n";
		break;
		
	}

	return true;
}

/* Register this handler with the XML parser */
add_xml_handler("http://purl.org/dc/elements/1.1/",
		array(
			"element"	=> 'dc_element_handler',
		)
	);
?>
