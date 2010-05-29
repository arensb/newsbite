<?
/* Plugin for handling the RSS Content:
 * http://purl.org/rss/1.0/modules/content/
 *
 * <content:encoded><![CDATA[...]]></content:encoded>
 *	The contents of the post
 *
 * The following are also defined, but don't seem to be used:
 * <content:items>
 * <content:item>
 * <content:format>
 * <rdf:value>
 * <content:encoding>
 */

/* content_element_handler
 * Handle the RSS Content module:
 * http://purl.org/rss/1.0/modules/content/
 *
 * <content:encoded><![CDATA[...]]></content:encoded>
 *	The contents of the post
 *
 * The following are also defined, but don't seem to be used:
 * <content:items>
 * <content:item>
 * <content:format>
 * <rdf:value>
 * <content:encoding>
 */
function content_element_handler(
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
	    case "encoded":
		/* In practice, only RSS files[1] use the Content
		 * module (since Atom already has a <content>
		 * element), so we'll just set the parent's 'content'.
		 *
		 * [1] So does RDF, actually, but it's specially
		 * crafted to look like RSS.
		 */
		// XXX - Make sure $chldren is a string?
		$retval['content'] = $children;
		break;

	    default:
//		echo "Warning: unknown \"content:\" element: [$elt_name]\n";
		break;
		
	}

	return true;
}

/* Register this handler with the XML parser */
add_xml_handler("http://purl.org/rss/1.0/modules/content/",
		array(
			"element"	=> content_element_handler
			)
	);
?>
