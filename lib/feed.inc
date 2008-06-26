<?
/* feed.inc
 * Functions for parsing RSS (and similar) XML feed files.
 */
// XXX - Should probably inspect HTML in summary and content: if any
// links or images are relative, try to make them absolute. Or maybe
// there's a way in HTML/XHTML to specify a base URL.

// Make sure the fields set by parse_{rss,atom}{,_item} are the same:
// RSS:			Atom:			field
// <item>		<entry>			items[]
// <rss>			<feed>			feed_type
// <rss version>		-			rss_version
// <title>		<title>			title
// -			<subtitle>		subtitle
// <link>		<link rel=alternate>	url
// -			<link rel=replies type=html>	comment_url
// -			<link rel=replies type=xml>	comment_feed
// <description>					description
// <language>					language
// <copyright>					-
// <managingEditor>	<author><email>		author_email
//			<author><name>		author_name
//			<author><uri>		author_url
// <webMaster>					webmaster
//			<contributor>		feed_contributors[]
// <pubDate>					pub_time
// <lastBuildDate>	<updated>		build_time
// <category>		<category><term>	categories[][term]
//			<category><scheme>	categories[][scheme]
//			<category><label>	categories[][label]
// <generator>		<generator>text		generator
//			<generator uri>		generator_url
//			<generator version>	generator_version
// <docs>					-
// <cloud>					-
// <ttl>						ttl (secs)
// <image><url>		<icon>			image[url]
// <image><title>				image[title]
// <image><link>					image[link]
// <rating>					-
// <textInput>					-
// <skipHours>					-
// <skipDays>					-
//			<guid>			guid
//			<rights>		rights

// RSS item:		Atom entry		field
// <title>		<title>			title
// <link>		<link rel=alternate>	url
// <description>		<summary>		description
//			<content>		content
// <author>		<author><email>		author_email
//			<author><name>		author_name
//			<author><url>		author_url
// <category>		<category>		categories[]
// <comments>		<link rel=replies type=html>	comment_url
//			<link rel=replies type=xml>	comment_feed
// <enclosure>		<link {other}>?		enclosure{}
// <guid>		<guid>			guid
// <pubDate>		<published>		pub_time
//			<updated>		build_time
// <source>		<source>		-
//			<rights>		-

/* $element_hooks
 * Lists hooks to be called after a particular element has been seen.
 * The entries are of the form
 *	""	=> array(			// Namespace URL
 *		"channel"	=> array(		// Element name
 *			rss_rss_cleanup
 *			),
 *		"item"	=> array(		// Element name
 *			rss_rss_cleanup
 *			)
 *		)
 */
$element_hooks = array();

/* url2handler
 * Maps the URL for a given namespace to its handler.
 * Elements of this array are of the form
 *	$URL => array(
 *			"element" => element_handler,
 *			"attribute" => attribute_handler,
 *			"text" => text_handler,
 *		),
 */
$url2handler = array(		// Handlers for specific URLs
	// Root element URLs

	// RSS 0.9x, 1.x, 2.0
	"" => array(
		"init"		=> rss_init,
		"element"	=> rss_element_handler,
		"attribute"	=> rss_attribute_handler,
//		"text"		=> rss_text_handler
		),

	// Atom
	"http://www.w3.org/2005/Atom"	=> array(
		"init"		=> atom_init,
		"element"	=> atom_element_handler,
		"attribute"	=> atom_attribute_handler,
		),

	// RDF
	"http://purl.org/rss/1.0/" => array(
		"element"	=> rdf_element_handler,
		),
	"http://www.w3.org/1999/02/22-rdf-syntax-ns#"	=> array(
		"element"	=> rdf_element_handler,
		),

// Other namespaces I've seen, for additional features inside a feed:
// admin	http://webns.net/mvcb/
// atom		http://www.w3.org/2005/Atom
// blogChannel	http://backend.userland.com/blogChannelModule
// cc		http://web.resource.org/cc/
// content	http://purl.org/rss/1.0/modules/content/
	"http://purl.org/rss/1.0/modules/content/"	=> array(
		"element"	=> content_element_handler,
		),
// dc		http://purl.org/dc/elements/1.1/
// feedburner	http://rssnamespace.org/feedburner/ext/1.0
// flickr	http://flickr.com/services/feeds/
// itunes	http://www.itunes.com/dtds/podcast-1.0.dtd
// lj		http://www.livejournal.org/rss/lj/1.0/
	"http://www.livejournal.org/rss/lj/1.0/" => array(
		"init"		=> lj_init,
		"element"	=> lj_element_handler,
		),
// media	http://search.yahoo.com/mrss/
// openSearch	http://a9.com/-/spec/opensearchrss/1.0/
// sy		http://purl.org/rss/1.0/modules/syndication/
// thr		http://purl.org/syndication/thread/1.0
// wfw		http://wellformedweb.org/CommentAPI/
	);

/* parse_feed
 * Take an XML string (read from a file or URL) and parse it down to a
 * PHP data structure of the form
 * $feed = array(			// Assoc describing the feed
 *	"title" => "Some Feed",
 *	"url" => "http://link.to/site/",
 *	...
 *	"items" => array(		// Array of posts in the feed
 *			array(		// First post
 *				"title" => "Title of post",
 *				"author" => "Author of post",
 *				"description" => "Summary of post",
 *				"content" => "Full body of post",
 *				...
 *			),
 *			array( ... ),	// Second post
 *			array( ... ),	// Thir post
 *			...
 *		),
 * );
 *
 * (See field list, above)
 *
 * The first thing we need to do is to find the root node, and decide
 * what to do with the rest of the XML file. In practice, this falls
 * into one of three cases:
 *	1) The root node has an 'xmlns=$URL' attribute, which tells us
 *	what kind of file this is, and therefore how to parse it.
 *	2) The root node is <rss> but has no 'xmlns=' attribute.
 *	Assume that it's an RSS file.
 *	3) None of the above. Throw an error.
 *	In particular, this can happen if you request a feed from
 *	LiveJournal and specify authentication, but don't supply a
 *	username or password. Then you get a fragment of HTML code
 *	with an error message.
 *
 *	After this initialization, the main loop walks through the XML
 * file, tag by tag and attribute by attribute. Whenever it sees a
 * closing tag (or an attribute) it calls a handler to Do the Right
 * Thing.
 */
function parse_feed($feed_text)
{
echo "Inside parse_feed(",
	htmlspecialchars(substr($feed_text, 0, 512)), ")<br/>\n";
	$ns2url = array();		// Maps namespace prefixes to their
					// URL identifiers
	$context = array();		// Context stack. Shows
					// handlers where they are

	// This preg_replace_callback is a butt-ugly hack to deal with
	// the top10 feed: it has constructs of the form
	//	<description>
	//	Some <b>unescaped</b> HTML text
	//	</description>
	// This is probably legal (<rdf:value> allows it as well), but
	// I don't have a good way to deal with it. So this hack looks
	// for
	// <description>...</description> pairs, and escapes the ones
	// that
	// a) don't begin with "<![CDATA[", and
	// b) contain a "<".
	// Oh, and it unescapes entities like "&quot;"

	// XXX - Come up with a better way of dealing with this.
	// Perhaps with $reader->expand?
	$feed_text = preg_replace_callback('{<description>(.*?)</description>}s',
					   create_function('$matches',
							   'if (substr($matches[1], 0, 9) == "<![CDATA[")
					// Properly escpaed
					return $matches[0];
			       if (strpbrk($matches[1], "<>"))
					// Contains <>. Must escape
					return "<description><![CDATA[" .
						htmlspecialchars_decode($matches[1]) .
						"]]></description>";
			       // No need to escape
			       return $matches[0];'),
					   $feed_text);

	/* Initialize the feed reader */
	$reader = new XMLReader();
	$reader->xml($feed_text);

	/* Find root node
	 * Although this code repeats a lot of code found further
	 * down, under "case ELEMENT:", it has to be separate: we want
	 * to do some top-level initialization and error-checking.
	 */
	// XXX - This can run into syntax errors. Shouldn't send
	// those to the user.
	while ($err = $reader->read())
	{
		if ($reader->nodeType != XMLReader::ELEMENT)
			// Skip past things like document type and
			// preprocessing instructions.
			continue;

		/* Create a new node for the root of the tree */
		$node = array(
			"ns"		=> $reader->prefix,
			"name"		=> $reader->localName,
			"ns_url"	=> $reader->namespaceURI,
			"is_empty"	=> $reader->isEmptyElement,
			// Can't just use
			// $reader->isEmptyElement at the end,
			// because the attribute parsing code
			// is going to change $parser's state.
			"retval"	=> NULL,
			);

		/* Find attributes */
		if ($reader->attributeCount > 0)
		{
			$attributes = array();

			$reader->moveToFirstAttribute();
			do {
				$attr_prefix     = $reader->prefix;
				$attr_name       = $reader->localName;
				$attr_fullname   = $reader->name;
				$attr_value      = $reader->value;

				// Namespaces look kinda like attributes, but
				// we handle them separately.
				// Namespace declarations come in two
				// varieties:
				//	xmlns="some/url" (default namespace)
				//	xmlns:prefix="some/url"
				if ($attr_prefix == "" &&
				    $attr_name == "xmlns")
				{
					// Default namespace
					$ns2url[""] = $attr_value;
					continue;
				}

				if ($attr_prefix == "xmlns")
				{
					// Non-default namespace
					$ns2url[$attr_name] = $attr_value;
					continue;
				}

				// Save all info about this attribute, so we
				// can call a handler for it below.
				$attributes[] = array(
					"prefix"	=> $attr_prefix,
					"name"		=> $attr_name,
					"value"		=> $attr_value,
					"url"		=> $reader->namespaceURI,
					);
			} while ($reader->moveToNextAttribute());
			// XXX - Ideally, ought to remember scope of
			// namespaces, so that you can have things
			// like
			//	<item>
			//	  <foo:elem1 xmlns:foo="abc">
			//	    <!-- "foo" -> "abc" -->
			//	  </foo:elem1>
			//	  <foo:elem2 xmlns:foo="xyz">
			//	    <!-- "foo" -> "xyz" -->
			//	  </foo:elem2>
			// but I suspect that's too rare to worry about until
			// it crops up in real life.
		}

		break;	// Found the root node.
	}
echo "\$err: ["; print_r($err); echo "]<br/>\n";

	/* Based on the default namespace in the root node, figure out which
	 * handler to use for the rest:
	 * If there's a default namespace (with URL)
	 *	If the URL has a handler in $known_handlers, use it
	 *	Else throw an error
	 * Else if the root node is <rss>
	 *	Use the RSS handler
	 * Else
	 *	Throw an error.
	 */
	if (isset($ns2url[""]))
	{
		// There's a default namespace for this XML file

		/* Do we have a handler for it? */
		if (isset($url2handler[$ns2url[""]]))
		{
			// Yes
		} else {
			// No
			// XXX - Abort
		}
	} elseif ($node['name'] == "rss")
	{
		// Use the default
	} else {
		return FALSE;	// Don't know how to handle this
	}

	/* Run the init hooks for any namespaces we've found */
	foreach ($ns2url as $ns => $url)
	{
		if (isset($url2handler[$url]['init']))
			$url2handler[$url]['init']($ns, $url);
	}

	/* Process handlers for any attributes we've seen. This is
	 * outside the loop above that records the attributes, because
	 * for all I know it's possible to have
	 *	<rss foo:attr="some value" xmlns:foo="http://some/url">
	 * If the attribute handler were inside the loop, we'd see
	 * "foo:attr" before the declaration of "xmlns:foo", and
	 * wouldn't know which handler to call.
	 */
	if (isset($attributes))
	{
		$node['@attrs'] = array();	// List of root attributes
		array_push($context, &$node);
				// Push $node onto context, for handler
		foreach ($attributes as $a)
		{
			/* If there's an attribute handler for this
			 * namespace, call it. Otherwise, use the
			 * default.
			 */
			if (isset($url2handler[$a['url']]['attribute']))
				$url2handler[$a['url']]['attribute'](
					$a['prefix'],
					$a['name'],
					$a['value'],
					$node['@attrs'],
					$context
					);
			else
				default_attribute_handler(
					$a['prefix'],
					$a['name'],
					$a['value'],
					$node['@attrs'],
					$context
					);
		}
		$node = array_pop($context);
				// We're no longer looking at attributes
	}

	/* Parse the rest of the file */
	while ($reader->read())
	{
		switch ($reader->nodeType)
		{
		    case XMLReader::ELEMENT:
			array_push($context, $node);
			// Save $node on context stack

			// $node describes the node we're currently looking
			// at. Be careful to separate the return value
			// ("retval") from the description of the node itself.
			$node = array(
				"ns"		=> $reader->prefix,
				"name"		=> $reader->localName,
				"ns_url"	=> $reader->namespaceURI,
				"is_empty"	=> $reader->isEmptyElement,
				// Can't just use
				// $reader->isEmptyElement at
				// the end, because the
				// attribute parsing code is
				// going to change $parser's
				// state.
				"retval"	=> NULL,
				);

			/* Check attributes */
			if ($reader->hasAttributes)
			{
				$attributes = array();

				$reader->moveToFirstAttribute();

				do {
					$attr_prefix     = $reader->prefix;
					$attr_name       = $reader->localName;
					$attr_fullname   = $reader->name;
					$attr_value      = $reader->value;

					/* Is this a namespace
					 * declaration? If so, it's of
					 * the form
					 * xmlns:foo="some url"
					 * or
					 * xmlns="some url"
					 */
					if ($attr_prefix == "" &&
					    $attr_name == "xmlns")
					{
						// Default namespace

						/* Run init hook if
						 * there is one
						 */
						if (!isset($ns2url[""]) &&
						    isset($url2handler[$attr_value]['init']))
							$url2handler[$attr_value]['init'](
								$attr_prefix,
								$attr_value);
						$ns2url[""] = $attr_value;
						continue;
					}

					if ($attr_prefix == "xmlns")
					{
						// Non-default namespace
						$ns2url[$attr_name] = $attr_value;
						if (!isset($ns2url[$attr_name]) &&
						    isset($url2handler[$attr_value]['init']))
							$url2handler[$attr_value]['init'](
								$attr_prefix,
								$attr_value);
						continue;
					}

					/* Call attribute handler */
					array_push($context, $node);
						// Push $node onto
						// context, for the
						// handler's benefit
					if (isset($url2handler[$node['ns_url']]["attribute"]))
						$url2handler[$node['ns_url']]["attribute"](
							$attr_prefix,
							$attr_name,
							$attr_value,
							$attributes,
							$context);
					default_attribute_handler(
						$attr_prefix,
						$attr_name,
						$attr_value,
						$attributes,
						$context);
					$node = array_pop($context);
						// Restore $node
				} while ($reader->moveToNextAttribute());
				$node['@attrs'] = $attributes;
			}

			if ($node['is_empty'])
				// This is an empty element (<foo/>), so we
				// need to close it right away, as if we'd
				// just seen <foo></foo>.
				_feed_close_element($node, $context);
			break;

		    case XMLReader::END_ELEMENT:
			_feed_close_element($node, $context);
			break;

		    case XMLReader::TEXT:
		    case XMLReader::CDATA:
			// TEXT and CDATA are combined because
			// XMLReader is nice enough to expand the HTML
			// entities in TEXT, so they're both presented
			// to us the same way.
			if (isset($url2handler[$node['ns_url']]["text"]))
				$url2handler[$node['ns_url']]["text"](
					$node,
					$reader->value,
					$node['retval'],
					$context);
			else
				default_text_handler(
					$node,
					$reader->value,
					$node['retval'],
					$context);
			break;

			/* The following tokens are either never used, or
			 * aren't useful.
			 */
		    case XMLReader::ATTRIBUTE:
			// Should never show up here. Look under
			// XMLReader::ELEMENT.
			break;
		    case XMLReader::NONE:
			// Apparently never used
			break;
		    case XMLReader::ENTITY_REF:
			// Apparently never used
			break;
		    case XMLReader::ENTITY:
			// Apparently never used
			break;
		    case XMLReader::PI:
			// Apparently name and value are the only fields with
			// any values. For
			// < ?some-thing blah='de' blah? >
			// 	name="some-thing"
			//	value="blah='de' blah"

			// Some of these are stylesheets, but it's not worth
			// applying them, because the ones I've seen are all
			// aimed at making XML look presentable in a browser.
			break;
		    case XMLReader::COMMENT:
			// This is just XML comments, not HTML comments.
			// Nothing interesting in here.
			break;
		    case XMLReader::DOC:
			// Apparently never used
			break;
		    case XMLReader::DOC_TYPE:
			// Apparently the only useful information is the name
			// of the root node. This can be used to guess the
			// correct handler, but in so many other cases we have
			// to look it up anyway, that it's not worth the
			// effort.
			break;
		    case XMLReader::DOC_FRAGMENT:
			// Apparently never used
			break;
		    case XMLReader::NOTATION:
			// Apparently never used
			break;
		    case XMLReader::WHITESPACE:
			// Apparently never used
			break;
		    case XMLReader::SIGNIFICANT_WHITESPACE:
			// Ignore this
			break;
		    case XMLReader::END_ENTITY:
			// Apparently never used
			break;
		    case XMLReader::XML_DECLARATION:
			// Apparently never used
			break;
		    default:
			// XXX - Better error-reporting
			echo "Unknown node type: [", $reader->nodeType, "], [",
				$reader->name, "]\n";
			break;
		}
	}

//	echo "context: ["; print_r($context); echo "]\n";
//	echo "node: ["; print_r($node); echo "]\n";

	return $node['retval'];
}

/* _feed_close_element
 * Private function, called when an element is closed, upon seeing
 * either </foo> or <foo/>.
 * Runs the appropriate hooks and adjusts the stack.
 */
function _feed_close_element(&$node, &$context)
{
	global $element_hooks;
	global $url2handler;

	/* Run any hooks associated with this element.
	 * Hooks are run before the handler: that way, a hook has a
	 * chance to clean up the element before the hander determines
	 * what its return value is.
	 */
	$hooks = $element_hooks[$node['ns_url']][$node['name']];
	if (isset($hooks))
	{
		foreach ($hooks as $h)
		{
			/* Call each hook in turn */
			$h($node['name'],
			   $node['retval'],
			   $context);
		}
	}

	/* Call element handler */
	$retval =& $context[count($context)-1]['retval'];
	if (isset($url2handler[$node['ns_url']]["element"]))
		$url2handler[$node['ns_url']]["element"](
			$node['ns'],
			$node['name'],
			$node['@attrs'],
			$node['retval'],
			$retval,
			$node['ns_url'],
			$context);
	else
		// XXX - Is this a good thing? If we don't have a
		// handler for this element, do we want to risk the
		// default one messing things up?
		default_element_handler($node['ns'],
					$node['name'],
					$node['@attrs'],
					$node['retval'],
					$retval,
					$node['ns_url'],
					$context);
	/* Pop parent element from context stack, and add
	 * current $node to it.
	 * (We didn't do this earlier, because if we had, then
	 * $context wouldn't have included $parent).
	 */
	$parent = array_pop($context);
	$parent['retval'] = $retval;
	$node = $parent;

	// XXX - Return something intelligent
}

/* default_element_handler
 * Function to do something sensible with an element, if no other
 * handler does. This function is called after the closing tag has
 * been seen, and any other hooks have run.
 *
 * XXX - Move the following description elsewhere:
 *
 * The job of this function is to look at the values returned by the
 * handlers that looked at the contents of the element, and fill in
 * $retval.
 *
 * $prefix - Prefix of the element: in <lj:mood>, $prefix is "lj"; in
 *	<title>, it's the empty string. The main purpose of this is to
 *	let the handler know whether it's in the default namespace or
 *	not.
 * $elt_name - Name of the element: in <lj:mood>, it's "mood"; in
 *	<title>, it's "title".
 * $attrs - the element's attributes
 * $retval - A value to return from this element
 * $ns_url - URL of this element's namespace. Useful mainly if there's
 *	a handler that deals with several related namespaces
 * $context - The context stack. Useful if the handler wants to treat
 *	a given element differently depending on where it is. For instance,
 * 	<channel><link/> might have to be treated differently from
 *	<channel><item><link/>
 */
function default_element_handler(
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	$ns_url,	// URL of this element's namespace
	&$context)	// Where we are at the moment
{
	$parent = $context[count($context)-1]['name'];

	// XXX - Should probably just pass stuff through:
	if ($prefix == "")
		$retval[$elt_name] = $children;
	else
		$retval[$prefix . ":" . $elt_name] = $children;
	$retval['@attrs'] = $attrs;

	return;		// Do nothing.

	if (isset($retval[$elt_name]))
	{
		// There's already one or more $elt_names in $parent
		if (is_array($retval[$elt_name]) &&
		    array_key_exists(0, $retval[$elt_name]))
		{
			// Append $children to existing array
			array_push($retval[$elt_name], $children);
		} else {
			// Create a new array with the old and new values
			$retval[$elt_name] = array(
				$retval[$elt_name],
				$children);
		}
	} else {
		// This is the first $elt_name child
		switch ($elt_name)
		{
//		    case 'item':
//			// XXX - This is RSS-specific
//			$retval['items'] = $children;
//			break;
		    default:
			$retval[$elt_name] = $children;
		}
	}
}

function default_text_handler(
	&$parent,
	$text,		// The #text element, as a string
	&$retval,	// Fill in the blanks
	&$context)	// Where we are at the moment
{
	$retval .= $text;
}

function default_attribute_handler(
	$prefix,	// Prefix of the attribute
	$name,		// Name of the attribute
	$value,		// Value of the attribute
	&$retval,	// Fill in the blanks
	&$context	// Where we are at the moment
	)
{
	if ($prefix == "")
		$retval[$name] = $value;
	else
		$retval[$prefix . ":" . $name] = $value;
}

/********** RSS ************************************************/
function rss_init($ns, $url)
{
//	echo "Inside rss_init($ns, $url)\n";
}

function rss_element_handler(
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	$ns_url,	// URL of this element's namespace
	&$context)	// Where we are at the moment
{
	$parent = $context[count($context)-1]['name'];
			// Get parent node's name
	switch ($elt_name)
	{
	    case "title":
		// This might be:
		// - <channel><title>
		// - <image><title>
		// - <item><title>
		// - <textInput><title>
		switch ($parent)
		{
		    case "channel":
		    case "image":
		    case "item":
		    case "textInput":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "link":
		// Might be:
		// - <channel><link>
		// - <channel><image><link>
		// - <channel><textInput><link>
		// - <item><link>
		// - <textInput><link>
		switch ($parent)
		{
		    case "image":
			$retval['link'] = $children;
			break;
		    case "channel":
		    case "item":
		    case "textInput":
			$retval['url'] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "description":
		switch ($parent)
		{
		    case "channel":
		    case "item":
		    case "image":
		    case "textInput":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "pubDate":
		switch ($parent)
		{
		    case "channel":
		    case "item":
			// XXX - Make sure $children is a string.
			$retval['pub_time'] = strtotime($children);
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "lastBuildDate":
		switch ($parent)
		{
		    case "channel":
			// XXX - Make sure $children is a string.
			$retval['build_time'] = strtotime($children);
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "generator":
		switch ($parent)
		{
		    case "channel":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "generator":
		switch ($parent)
		{
		    case "image":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "url":
		switch ($parent)
		{
		    case "image":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "image":
		switch ($parent)
		{
		    case "channel":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "author":
		switch ($parent)
		{
		    case "item":
			// XXX - Make sure $children is a string
			$retval['author_email'] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "managingEditor":
		switch ($parent)
		{
		    case "channel":
			// XXX - Make sure $children is a string
			$retval['author_email'] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "guid":
		switch ($parent)
		{
		    case "item":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "width":
	    case "height":
		switch ($parent)
		{
		    case "image":
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "copyright":
	    case "docs":
		switch ($parent)
		{
		    case "channel":
			// Ignore copyright.
			// XXX - Should this be tacked onto the
			// channel description?
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "comments":
		switch ($parent)
		{
		    case "item":
			$retval['comment_url'] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "language":
		switch ($parent)
		{
		    case "channel":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "webMaster":
		switch ($parent)
		{
		    case "channel":
			$retval['webmaster'] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "category":
		switch ($parent)
		{
		    case "item":
			$retval['categories'][] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "enclosure":
		switch ($parent)
		{
		    case "item":
			if (!isset($retval['enclosure']))
				$retval['enclosure'] = array();
			$retval['enclosure'][] = array(
				"url"		=>
					htmlspecialchars_decode($attrs['url']),
					// XXX - This might need a
					// second round of
					// htmlspecialchars_decode().
					// Only atheistmedia has this
					// problem, so I don't know
					// whether it's a problem with
					// the blogger.com generator,
					// or part of the spec.
				"length"	=> $attrs['length'],
				"type"		=> $attrs['type'],
				);
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "name":
		switch ($parent)
		{
		    case "textInput":
			$retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "textInput":
		switch ($parent)
		{
		    case "channel":
			// Just ignore it.
			// $retval[$elt_name] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "ttl":
		switch ($parent)
		{
		    case "channel":
			// XXX - Make sure $children is a string
			$retval[$elt_name] = $children * 60;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "item":
		switch ($parent)
		{
		    case "channel":
			$retval['items'][] = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "channel":
		switch ($parent)
		{
		    case "rss":
			$retval = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "rss":
		switch ($parent)
		{
		    case "":
			$retval = $children;
			break;
		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    default:
		echo "Warning: unknown RSS element: [$elt_name]\n";
		break;
	}

	// XXX - Return something sensible
}

function rss_attribute_handler(
	$prefix,	// Prefix of the attribute
	$name,		// Name of the attribute
	$value,		// Value of the attribute
	&$retval,	// Fill in the blanks
	&$context	// Where we are at the moment
	)
{
	// XXX
	// <rss version=[...]>		Version of RSS spec
	// <source url=[...]>		XML version of source
	// <enclosure url=[...]>	Where to find enclosure
	// <enclosure length=[...]>	Length of enclosure in bytes
	// <enclosure type=[...]>	MIME type of enclosure
	// <category domain=[...]>	URL of categorization taxonomy
	// <guid isPermaLink=[true|false]>	Whether GUID is URL to item

	// XXX - Return something sensible
}

/********** Atom ****** ****************************************/
/*
 * Summary of http://tools.ietf.org/html/rfc4287 :
 * start = atomFeed | atomEntry
 * atomCommonAttributes =
 *	attribute xml:base { atomUri }?,
 *	attribute xml:lang { atomLanguageTag }?,
 *	undefinedAttribute
 * Any element can have xml:base attribute, giving base for relative
 *	links
 * Any element can have xml:lang attribute
 *
 * atomPlainTextConstruct =
 *	atomCommonAttributes,
 *	attribute type { "text" | "html" }?,
 *	text
 * atomXHTMLTextConstruct =
 *	atomCommonAttributes,
 *	attribute type { "xhtml" },
 *	xhtmlDiv
 * atomTextConstruct = atomPlainTextConstruct | atomXHTMLTextConstruct
 * "type" attribute must be one of "text", "html", or "xhtml".
 * Defaults to "text". Text and HTML elements have no children.
 * HTML text can be stuck inside a <div>.
 * XHTML element must be a single <div>; strip off the <div> before
 * displaying.
 * Markup:
 *	text: string has been HTML-escaped for the feed. So leave it alone:
 *		it's already HTML-escaped.
 *	html: string has been HTML-escaped so that HTML sequences in the
 *		string don't interfere with XML markup. Unescape one level
 *		before displaying.
 *	xhtml: string is already XHTML. Leave it alone.
 *
 * atomPersonConstruct =
 *	atomCommonAttributes,
 *	(element atom:name { text }
 *	 & element atom:uri { atomUri }?
 *	 & element atom:email { atomEmailAddress }?
 *	 & extensionElement*)
 * Gives person's printable name, URL, and email address (RFC 822 format)
 *
 * atomDateConstruct =
 *	atomCommonAttributes,
 *	xsd:dateTime
 *
 * Containers:
 * atomFeed =
 *	element atom:feed {
 *		atomCommonAttributes,
 *		(atomAuthor*
 *		 & atomCategory*
 *		 & atomContributor*
 *		 & atomGenerator?
 *		 & atomIcon?
 *		 & atomId
 *		 & atomLink*
 *		 & atomLogo?
 *		 & atomRights?
 *		 & atomSubtitle?
 *		 & atomTitle
 *		 & atomUpdated
 *		 & extensionElement*),
 *		atomEntry*
 *	}
 *
 * atomEntry =
 *	element atom:entry {
 *		atomCommonAttributes,
 *		(atomAuthor*
 *		 & atomCategory*
 *		 & atomContent?
 *		 & atomContributor*
 *		 & atomId
 *		 & atomLink*
 *		 & atomPublished?
 *		 & atomRights?
 *		 & atomSource?
 *		 & atomSummary?
 *		 & atomTitle
 *		 & atomUpdated
 *		 & extensionElement*)
 *	}
 *
 * atom:content contains either content, or link to content
 * atomInlineTextContent =
 *	element atom:content {
 *		atomCommonAttributes,
 *		attribute type { "text" | "html" }?,
 *		(text)*
 *	}
 * atomInlineXHTMLContent =
 *	element atom:content {
 *		atomCommonAttributes,
 *		attribute type { "xhtml" },
 *		xhtmlDiv
 *	}
 * atomInlineOtherContent =
 *	element atom:content {
 *		atomCommonAttributes,
 *		attribute type { atomMediaType }?,
 *		(text|anyElement)*
 *	}
 * atomOutOfLineContent =
 *	element atom:content {
 *		atomCommonAttributes,
 *		attribute type { atomMediaType }?,
 *		attribute src { atomUri },
 *		empty
 *	}
 * atomContent = atomInlineTextContent
 *	| atomInlineXHTMLContent
 *	| atomInlineOtherContent
 *	| atomOutOfLineContent
 *
 * "src" attribute must be IRI (Internationalized Resource Identifier;
 * probably a URL in practice, albeit with i18n characters).
 *
 * Metadata elements:
 * atomAuthor = element atom:author { atomPersonConstruct }
 * atomCategory =
 *	element atom:category {
 *		atomCommonAttributes
 *		attribute term { text }
 *		attribute scheme { atomUri }?,
 *		attribute label { text }?,
 *		undefinedContent
 *	}
 *	"term" is a category name, and "scheme" identifies a
 *	categorization scheme. Thus, presumably a bunch of political
 *	sites could get together and decide that category "MT" in the
 *	scheme "http://politics.com/" refers to Montana.
 *	"label" is a human-readable label. Not used in practice.
 *
 * atomContributor = element atom:contributor { atomPersonConstruct }
 *	Person who contributed to this feed/entry
 *
 * atomGenerator = element atom:generator {
 *	atomCommonAttributes,
 *	attribute uri { atomUri }?,
 *	attribute version { text }?,
 *	text
 *	The software that generated this feed. XXX - Might be useful for
 *	hints on working around bugs and such.
 *
 * atomIcon = element atom:icon {
 *	atomCommonAttributes,
 *	(atomUri)
 *	An icon that goes with the feed.
 *
 * atomId = element atom:id {
 *	atomCommonAttributes,
 *	(atomUri)
 *	}
 *	GUID for this feed/entry
 *
 * atomLink =
 *	element atom:link {
 *		atomCommonAttributes,
 *		attribute href { atomUri },
 *		attribute rel { atomNCName | atomUri }?,
 *		attribute type { atomMediaType }?,
 *		attribute hreflang { atomLanguageTag }?,
 *		attribute title { text }?
 *		attribute length { text }?,
 *		undefinedContent
 *	}
 *	Link to something on the web. "href" attribute is mandatory.
 *	XXX - "title" can be used for "alt", I guess
 *	Apparently:
 *	- "link rel=alternate" points to real page for this item.
 *	- "link rel=replies type=text/html" points to comments page
 *	- "link rel=replies type=application/atom+xml" points to comments feed
 *
 * atomLogo = element atom:logo {
 *	atomCommonAttributes,
 *	(atomUri)
 *	}
 *	Link to an image that identifies the feed.
 *
 * atomPublished = element atom:published { atomDateConstruct }
 *	When item was first published (not when it was revised/edited).
 *
 * atomRights = element atom:rights { atomTextConstruct }
 *	Information about rights.
 *
 * atomSource
 *	Used to preserve metadata when copying items from one feed to
 *	another. Doesn't seem to be used in practice.
 *	XXX - Use this when it becomes desirable.
 *
 * atomSubtitle = element atom:subtitle { atomTextConstruct }
 *	Feed's subtitle, human-readable.
 *
 * atomSummary = element atom:summary { atomTextConstruct }
 *	Summary/abstract of an entry.
 *
 * atomTitle = element atom:title { atomTextConstruct }
 *	Title of the entry/feed, human-readable.
 *
 * atomUpdated = element atom:updated { atomDateConstruct }
 *	Time when entry/feed was last updated.
 */

function atom_init($ns, $url)
{
//	echo "Inside atom_init($ns, $url)\n";
}

function atom_element_handler(
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	$ns_url,	// URL of this element's namespace
	&$context)	// Where we are at the moment
{
	if ($prefix != "")
	{
		// If we're a guest, just pass this element along.
		// XXX - Should put this in the other
		// *_element_handler()s.
		$retval["$prefix:$elt_name"] = $children;
		$retval["$prefix:$elt_name"]['@attrs'] = $attrs;
		return;
	}

	$parent = $context[count($context)-1]['name'];
	switch ($elt_name)
	{
	    case "id":
		switch ($parent)
		{
		    case "feed":
		    case "entry":
			// Entry ID, in a not-terribly-useful format.
			// Ignore it.
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "updated":
		switch ($parent)
		{
		    case "feed":
		    case "entry":
			// XXX - Make sure $children is a string.
			$retval['build_time'] = strtotime($children);
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "title":
		switch ($parent)
		{
		    case "feed":
		    case "entry":
			// XXX - Make sure $children is a string.
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "subtitle":
		switch ($parent)
		{
		    case "feed":
			// XXX - Make sure $children is a string.
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "link":
		switch ($parent)
		{
		    case "feed":
			switch ($attrs['rel'])
			{
			    case "alternate":
				// Real page for this feed
				$retval['url'] = $attrs['href'];
				break;

			    case "replies":
				if ($attrs['type'] == "text/html")
				{
					// Page where comments can be found
					$retval['comment_url'] = $attrs['href'];
					break;
				} elseif ($attrs['type'] == "application/atom+xml")
				{
					// Feed for comments on this post
					$retval['comment_feed'] = $attrs['href'];
					break;
				} else {
					// XXX - Better error-reporting
					echo "Warning: Unexpected replies type [$attrs[type]]\n";
					break;
				}

			    case "next":
				// Apparently this is nonstandard. In
				// blogspot.com feeds, this points to
				// the next 25 items. Ignore it.
				break;

			    case "self":
				// Link to the Atom feed itself.
				// Ignore it.

				// XXX - Or perhaps ought to save it,
				// in case the feed changes, and
				// update the database automatically.
				break;

			    case "license":
				// Apparently nonstandard. License
				// under which this content is
				// released. Ignore it.
				break;

			    default:
				if (preg_match(',^http://,',
					       $attrs['rel']))
				{
					// Not sure what this is, but
					// blogspot.com has <link
					// rel="http://schemas.google.com/g/2005#feed" type="application/atom+xml"
					// that points to the feed URL.
					break;
				}
				// XXX - Better error-reporting
				echo "Warning: Unexpected link rel: [$attrs[rel]]\n";
				break;
			}
			break;

		    case "entry":
			switch ($attrs['rel'])
			{
			    case "alternate":
				// Real page for this entry
				$retval['url'] = $attrs['href'];
				break;

			    case "replies":
				if ($attrs['type'] == "text/html")
				{
					// Page where comments can be found
					$retval['comment_url'] = $attrs['href'];
					break;
				} elseif ($attrs['type'] == "application/atom+xml")
				{
					// Feed for comments on this post
					$retval['comment_feed'] = $attrs['href'];
					break;
				} else {
					// XXX - Better error-reporting
					echo "Warning: Unexpected replies type [$attrs[type]]\n";
					break;
				}

			    case "self":
				// Feed for this entry. Seems a bit
				// pointless, if you ask me, but okay.
				// Ignore it.
				break;

			    case "edit":
				// Link for editing an entry?
				// Ignore it (for now?)
				break;

			    default:
				// XXX - Better error-reporting
				echo "Warning: Unexpected link rel: [$attrs[rel]]\n";
				break;
			}
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "name":
		switch ($parent)
		{
		    case "author":
			// XXX - Make sure $children is a string.
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "uri":
		switch ($parent)
		{
		    case "author":
			// XXX - Make sure $children is a string.
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "email":
		switch ($parent)
		{
		    case "author":
			// XXX - Make sure $children is a string.
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "author":
		switch ($parent)
		{
		    case "feed":
		    case "entry":
			$retval['author_email'] = $children['email'];
			$retval['author_name']  = $children['name'];
			$retval['author_url']   = $children['uri'];
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "generator":
		switch ($parent)
		{
		    case "feed":
			// XXX - Make sure $children is a string.
			$retval['generator'] = $children;
			$retval['generator_url'] = $attrs['uri'];
			$retval['generator_version'] = $attrs['version'];
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "published":
		switch ($parent)
		{
		    case "entry":
			// XXX - Make sure $children is a string.
			$retval['pub_time'] = strtotime($children);
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "category":
		switch ($parent)
		{
		    case "entry":
			// XXX - Make sure $children is a string?
			$retval['categories'][] = $attrs['term'];
			// XXX - Should perhaps do something smart with
			// $attrs['scheme']
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "summary":
		switch ($parent)
		{
		    case "entry":
			// XXX - Make sure $children is a string.
			$retval['description'] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "logo":
		switch ($parent)
		{
		    case "feed":
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "content":
		switch ($parent)
		{
		    case "entry":
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "entry":
		switch ($parent)
		{
		    case "feed":
			$retval['items'][] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "feed":
		switch ($parent)
		{
		    case "":
			$retval = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    default:
		// XXX - Better error-reporting
		echo "Warning: unknown Atom element: [$elt_name]\n";
		break;
	}
}

function atom_attribute_handler(
	$prefix,	// Prefix of the attribute
	$name,		// Name of the attribute
	$value,		// Value of the attribute
	&$retval,	// Fill in the blanks
	&$context	// Where we are at the moment
	)
{
	// XXX - What attributes does Atom provide that we might care
	// about?
//	echo "Inside atom_attribute_handler()\n";
}

/********** RDF ******* ****************************************/
function rdf_element_handler(
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	$ns_url,	// URL of this element's namespace
	&$context)	// Where we are at the moment
{
	$parent = $context[count($context)-1]['name'];

	switch ($elt_name)
	{
	    case "title":
		switch ($parent)
		{
		    case "channel":
			// XXX - Make sure $children is a string
			$retval[$elt_name] = $children;
			break;

		    case "item":
			// XXX - Make sure $children is a string
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "link":
		switch ($parent)
		{
		    case "channel":
		    case "item":
			// XXX - Make sure $children is a string
			$retval['url'] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "description":
		switch ($parent)
		{
		    case "channel":
			// XXX - Make sure $children is a string
			$retval[$elt_name] = $children;
			break;

		    case "item":
			// XXX - Make sure $children is a string
			$retval[$elt_name] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "li":
		switch ($parent)
		{
		    case "Seq":
			// XXX - Check attributes
			$retval['li'][] = $attrs['resource'];
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "Seq":
		switch ($parent)
		{
		    case "items":
			$retval['RDF:Seq'] = $children['li'];
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "items":
		switch ($parent)
		{
		    case "channel":
			$retval['RDF:Seq'] = $children['RDF:Seq'];
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "channel":
		switch ($parent)
		{
		    case "RDF":
			$retval = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "item":
		switch ($parent)
		{
		    case "RDF":
			$retval['items'][] = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    case "RDF":
		switch ($parent)
		{
		    case "":
			$retval = $children;
			break;

		    default:
			// XXX - Better error-reporting
			echo "Warning: Unexpected parent: [$parent]\n";
			break;
		}
		break;

	    default:
		// XXX - Better error-reporting
		echo "Warning: unknown RDF element: [$elt_name]\n";
		break;
	}

	// XXX - Return something sensible
}

/********** LiveJournal ****************************************/
function lj_init($ns, $url)
{
//	echo "Inside lj_init($ns, $url)\n";
}

function lj_element_handler(
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	$ns_url,	// URL of this element's namespace
	&$context)	// Where we are at the moment
{
	// XXX
	$retval["lj_" . $elt_name] = $children;
}

/********** content ****************************************/
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
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	$ns_url,	// URL of this element's namespace
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
		echo "Warning: unknown \"content:\" element: [$elt_name]\n";
		break;
		
	}
}
?>
