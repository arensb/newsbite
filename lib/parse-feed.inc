<?
/* feed.inc
 * Functions for parsing RSS (and similar) XML feed files.
 */

/* Note on time:
 * One bit of odd behavior that might be reported as a bug is that the
 * displayed time of an item is several days in the future from what
 * it ought to be. This is not a bug in this code:
 *	At least one feed (The Unspeakable Vault of Doom) has a
 * problem in its RSS feed: its <pubDate>s are of the form
 * "thu, 11 July 2008 00:00:00 -0800".
 * Since I first noticed this bug on Jun. 29, 2008, the obvious
 * problem is that the time is in the future.
 *	But the real problem is that 11 July 2008 is not a Thursday,
 * but a Friday. Apparently this confuses strtotime() (or causes it to
 * be smart, or something): apparently it thinks that "thu" is
 * _relative_ to "11 July 2008 00:00:00 -0800". That is, if someone
 * says, on Jul 11, "let's meet on Thursday", they obviously don't
 * mean "let's meet today", they mean "let's meet in six days". So
 * strtotime() adds six days to the date.
 *
 * This is not a bug in this code, and therefore will not be fixed.
 */

/* Note on the term "handler":
 * The word "handler" is, confusingly, used in two different senses:
 * the XML text is processed by the XML Parser PHP module, which calls
 * functions (which it calls handlers) to deal with the various XML
 * tokens it encounters.
 *
 * In this module the XML Parser handlers call other functions, to
 * deal with the specific type of XML file we're parsing (e.g., RSS,
 * Atom, etc.). These are also called handlers.
 *
 * The functions
 *	_element_start_handler
 *	_element_end_handler
 *	_cdata_handler
 *	_namespace_handler
 *	_end_namespace_handler
 * are XML Parser handlers. All others are the other kind of handler.
 */

// XXX - Should probably inspect HTML in summary and content: if any
// links or images are relative, try to make them absolute. Or maybe
// there's a way in HTML/XHTML to specify a base URL.

// XXX - Functions to add handlers

// Element-to-field mapping:
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
//			<id>			guid
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
// <guid>		<id>			guid
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

// XXX - This should go in a separate plugin file.
$element_hooks[""]["channel"][] = gocomics_hook;
$element_hooks[""]["description"][] = googleads_hook;
$element_hooks[""]["description"][] = feedburner_hook;
$element_hooks[""]["item"][] = feedburner_origlink_hook;
$element_hooks["http://purl.org/rss/1.0/modules/content/"]["encoded"][] = googleads_hook;
$element_hooks["http://purl.org/rss/1.0/modules/content/"]["encoded"][] = wordpress_noise_hook;

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
// XXX - Used by atheistcartoons.com
"http://purl.org/atom/ns#" => array(
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
	"http://purl.org/dc/elements/1.1/"	=> array(
		"element"	=> dc_element_handler,
		),
// feedburner	http://rssnamespace.org/feedburner/ext/1.0
// flickr	http://flickr.com/services/feeds/
// feedburner	http://rssnamespace.org/feedburner/ext/1.0
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

$url2ns = array();	// Map URLs to their namespace prefixes
$ns2url = array();	// Map namespace prefixes to their URLs

/* $_context
 * Context stack. Tells handlers where they are within the tree.
 */
$_context = array();

$feed_err = 0;		// Error code.
			// 0 - No error
			// 1 - Some error
			// XXX - Is it worth expanding this?
$feed_errmsg = "";	// Error message

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
 *			array( ... ),	// Third post
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
//echo "<hr/>About to parse [<pre>$feed_text</pre>]\n\n<hr/>\n";
	global $feed_err, $feed_errmsg;
	global $_context;

	$feed_err = 0;		// No error yet

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
	$feed_text = preg_replace_callback('{<description>(.*?)</description>}s',
					   create_function('$matches',
							   'if (preg_match("/^\s*<!\[CDATA\[.*\]\]>\s*$/s", $matches[1]))
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

	/* Initialize parser */
	$xml_parser = xml_parser_create_ns();
	xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, false);
	xml_parser_set_option($xml_parser, XML_OPTION_TARGET_ENCODING, "UTF-8");
			// I think UTF-8 might already be the default, but
			// let's set it explicitly anyway
	xml_set_element_handler($xml_parser,
				_element_start_handler,
				_element_end_handler);
	xml_set_start_namespace_decl_handler($xml_parser, _namespace_handler);
	// I'd set xml_set_end_namespace_decl_handler(), but it
	// doesn't seem to do anything, due to a bug in libxml2.
	xml_set_character_data_handler($xml_parser, _cdata_handler);

	/* Create a node to contain the value returned by the
	 * document, and push it onto the context stack.
	 */
	$node = array(
		"retval"	=> NULL
		);
	array_push($_context, $node);

	/* Start parsing */
	$err = xml_parse($xml_parser, $feed_text, true);

	if ($err == 0)
	{
		// XML Parser found an error.
		$feed_err = 1;
		$feed_errmsg = "XML Parser found an error: " .
			xml_error_string(xml_get_error_code($xml_parser));

		return FALSE;
	} else	if ($feed_err)
	{
		// XXX - Should just set error message, and allow
		// caller to do error-reporting.
//		echo "<b>Handler found an error</b><br/>\n";
		$feed_errmsg = "Handler error: $feed_errmsg";

		// The handlers have found an error.
		return FALSE;
	}

	/* Pop the node that we pushed earlier, and return its value */
	$node = array_pop($_context);
	return $node['retval'];
}

/* _feed_abort
 * Abort parsing the feed. Actually, we can't stop XML Parser from
 * running to completion, but this turns off any handlers, so that
 * they don't call any hooks that might have side effects.
 * Also, set the error code ($feed_err) and error message ($feed_errmsg).
 */
function _feed_abort($parser, $err, $msg)
{
	global $feed_err, $feed_errmsg;

	/* Disable all handlers */
	xml_set_element_handler($parser,
				FALSE, FALSE);
	xml_set_character_data_handler($parser, FALSE);

	/* Set the error code and message */
	$feed_err = $err;
	$feed_errmsg = $msg;
}

/* _element_start_handler
 * Handler for beginning of element. Private function.
 */
function _element_start_handler($parser, $fullname, $attrs)
{
	global $_context;
	global $url2handler;
	global $url2ns;

	/* Parse $fullname into URL and name */
	if (preg_match('/^(.*):(.*)$/', $fullname, $match))
	{
		$ns_url = $match[1];
		$name = $match[2];
	} else {
		$ns_url = "";
		$name = $fullname;
	}

	/* Create a new node */
	$node = array(
		"url"		=> $ns_url,
		"name"		=> $name,
		"retval"	=> NULL,
		);

	/* If this is the root node, decide how we're going to handle
	 * the rest of the file.
	 */
	if (count($_context) == 1)	// Only the topmost collector
					// on the context stack so far
	{
		/* Figure out what kind of file this is */

		if (isset($ns2url[""]))
		{
			// There's an explicit declaration of the
			// default namespace. (e.g., Atom, RDF, random
			// XML files)
			/* Do we have handler for default namespace? */
			if (!function_exists($url2handler[$ns2url[""]]))
			{
				// ...but we don't have a handler for
				// the default namespace that was
				// declared.
				_feed_abort($parser, 1,
					    "No handler for default namespace");
			} 
		} elseif ($ns_url == "")
		{
			// No explicit default namespace declaration,
			// and root has no prefix. This can still be
			// okay if the root node is "rss".
			if ($name != "rss")
				// This is probably HTML or something.
				_feed_abort($parser, 1,
					    "Unknown document type");
		}

		/* Can we process the root node?
		 * If it has no prefix, we've already established that
		 * it's "rss". But if the root node has a prefix, and
		 * we don't know how to handle it, abort.
		 */
		if (($ns_url != "") &&
		    (!isset($url2handler[$ns_url])))
			// We use isset() here instead of
			// function_exists() because we expect this to
			// be an assoc, and we don't want to have to
			// require any particular handler to exist.
		{
			// Error: no handler
			_feed_abort($parser, 1,
				    "No handler for this document type");
		}

		/* Call init handler, if any */
		if (function_exists($url2handler[$ns_url]["init"]))
		{
			$err = $url2handler[$ns_url]["init"](
				$ns_url,
				$url2ns[$ns_url]
				);
			/* Allow the handler to abort processing */
			if (!$err)
				_feed_abort($parser, 1,
					    "Aborted by init handler");
		}
	}

	/* Process attributes. Call handlers as necessary */
	if (count($attrs) > 0)
	{
		$node['@attrs'] = array();

		foreach ($attrs as $k => $v)
		{
			/* Parse $k into URL and name */
			if (preg_match('/^(.*):(.*)$/', $k, $match))
			{
				$attr_url = $match[1];
				$attr_name = $match[2];
			} else {
				$attr_url = "";
				$attr_name = $k;
			}

			/* Figure out which handler to call */
			// There's a potential problem here: let's say
			// we have
			// <feed xmlns="http://feed/type/"
			//	xmlns:ext="http://extension/type/">
			// ...
			//   <ext:element attr="value">
			//
			// Here, <element> is in the "ext" namespace,
			// and presumably the spec for "ext" says what
			// "attr" means. But since "attr" is in the
			// default namespace, it'll be processed by
			// some function that doesn't know about
			// "attr". It ought to pass "attr" through, so
			// that the function that'll handle
			// </ext:element> will see the attribute.
			//
			// This can cause some confusion, but
			// http://www.w3.org/TR/REC-xml-names/
			// explicitly says that attributes without a
			// prefix are in no namespace at all.
			//
			// So presumably best practice is for the
			// attribute handler to put unrecognized
			// attributes in $retval with their prefix.
			if (function_exists($url2handler[$attr_url]["attribute"]))				$handler = $url2handler[$attr_url]["attribute"];
			else
				$handler = default_attribute_handler;

			$err = $handler(
				$attr_url,
				$url2ns[$attr_url],
				$attr_name,
				$v,
				$node['@attrs'],
				$_context
				);
			/* Allow the handler to abort processing */
			if (!$err)
				_feed_abort($parser, 1,
					    "Aborted by attribute handler");
		}
	}

	/* Process this node: push it onto the context stack */
	array_push($_context, &$node);
}

/* _element_end_handler
 * Handler for end of element. Private function.
 */
function _element_end_handler($parser, $fullname)
{
	global $_context;
	global $url2handler;
	global $element_hooks;
	global $url2ns;

	/* Parse $fullname into URL and name */
	if (preg_match('/^(.*):(.*)$/', $fullname, $match))
	{
		$ns_url = $match[1];
		$name = $match[2];
	} else {
		$ns_url = "";
		$name = $fullname;
	}

	$node = array_pop($_context);	// Node that we're closing
	$retval =& $_context[count($_context)-1]['retval'];
			// Where to put return value from this element


	/* Run any hooks associated with this element.
	 * Hooks are run before the handler: that way, a hook has a
	 * chance to clean up the element before the hander determines
	 * what its return value is.
	 */
	// XXX - Test this code
	$hooks = $element_hooks[$ns_url][$name];
	if (isset($hooks))
	{
		foreach ($hooks as $h)
		{
			/* Call each hook in turn */
			$h($node['name'],
			   $node['retval'],
			   $context);
//echo "In _element_end_handler, now retval == [<pre>"; print_r($node['retval']); echo "</pre>]<br/>\n";
		}
	}

	/* Special case: if $node['retval'] is an array with only a
	 * "#text" entry, replace it with its string value.
	 */
	if (is_array($node['retval']) &&
	    count($node['retval']) == 1 &&
	    is_string($node['retval']['#text']))
		$node['retval'] = $node['retval']['#text'];

	/* Call element handler */
	if (function_exists($url2handler[$ns_url]["element"]))
		$handler = $url2handler[$ns_url]["element"];
	else
		$handler = default_element_handler;
	$err = $handler($ns_url,
			$url2ns[$ns_url],
			$name,
			$node['@attrs'],
			$node['retval'],
			$retval,
			$_context
		);
	/* Allow handler to abort processing */
	if (!$err)
		_feed_abort($parser, 1,
			    "Aborted by element handler");
}

/* _cdata_handler
 * Handle character data in the stream.
 * Note that this can be called several times for the same string: in
 *	<title>foo&amp;bar</title>
 * it'll be called for "foo", "&", and "bar".
 * It also gets called for whitespace between elements.
 */
function _cdata_handler($parser, $data)
{
	global $_context;

	// XXX - Call text handler, if one is defined?
	$parent = &$_context[count($_context)-1];
	$parent['retval']['#text'] .= $data;
}

// XXX - This version takes 3 arguments. The version documented at php.net
// takes four:
//	($parser, $user_data, $prefix, $uri)
// Fortunately, it's possible to define a function inside an
// if-statement, if I can find out how to tell which one is required:
//
// if (some condition)
// {
//	function foo($a, $b, $c) { ... }
// } else {
//	function foo($a, $b, $c, $d) { ... }
// }
// The simplest workaround might be to have the handler function
// defined this way, and have it call a helper function that does the
// real work.
function _namespace_handler($parser, $prefix, $url)
{
	global $url2handler;
	global $ns2url, $url2ns;

	/* Remember which URL goes with which prefix */
	// I suspect it's possible to have nested namespace
	// declarations with different prefixes:
	//
	// <foo xmlns:abc="http://some/url/">
	//   <abc:someElement/>
	//   <bar xmlns:def="http://some/url/">
	//     <def:someElement/>
	//   </bar>
	// </foo>
	//
	// Since we only look at when namespaces are declared, not
	// when they go out of scope, $ns2url and $url2ns will record
	// that "http://some/url/" corresponds to "def", and will
	// forget that it ever corresponded to "abc". This situation
	// is rare enough that we can probably ignore it until it
	// becomes a problem. Besides, handlers shouldn't need to know
	// which prefix the document uses.
	$ns2url[$prefix] = $url;
	$url2ns[$url] = $prefix;

	/* If there's an initialization hook for this URL, call it */
	if (function_exists($url2handler[$url]['init']))
		$url2handler[$url]['init']($url, $prefix);
}

// XXX - Apparently this is supposed to be called when leaving the
// scope of a namespace declaration. Unfortunately, there appears to
// be a bug in libxml that prevents it from working.
function _end_namespace_handler($parser, $prefix)
{
//	echo "End namespace:\n\tprefix: [$prefix]\n";
}

/* default_element_handler
 * Function to do something sensible with an element, if no other
 * handler does. This function is called after the closing tag has
 * been seen, and any other hooks have run.
 *
 * XXX - Move the following description to the formal documentation.
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
	$ns_url,	// URL of this element's namespace
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	&$context)	// Where we are at the moment
{
	/* Figure out this element's name, with prefix */
	if ($prefix == "")
		$my_name = $elt_name;
	else
		$my_name = "$prefix:$elt_name";

	if (is_array($children))
	{
		if (count($children) == 1 &&
		    array_key_exists('#text', $children))
			// $children is an array whose only element is a #text
			// string. Simplify and replace $children with that
			// string.
			$children = $children['#text'];
		else
			unset($children['#text']);
	}

	if (isset($retval[$my_name]))
	{
		// There's already one or more $my_names in $parent
		if (is_array($retval[$my_name]) &&
		    array_key_exists(0, $retval[$my_name]))
		{
			// Append $children to existing array
			array_push($retval[$my_name], $children);
		} else {
			// Create a new array with the old and new values
			$retval[$my_name] = array(
				$retval[$my_name],
				$children);
		}
	} else {
		// This is the first $my_name child
		$retval[$my_name] = $children;
	}

	return true;
}

// XXX - Should there even be a text/CDATA handler?
//function default_text_handler(
//	&$parent,
//	$text,		// The #text element, as a string
//	&$retval,	// Fill in the blanks
//	&$context)	// Where we are at the moment
//{
//	$retval .= $text;
//}

/* default_attribute_handler
 * Handles attributes that don't have a more specific handler: just
 * passes them through, with whichever prefix they have in the file.
 */
function default_attribute_handler(
	$ns_url,	// URL of this element's namespace
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

	return true;
}

/********** RSS ************************************************/
// RSS 2.0 spec at
// http://cyber.law.harvard.edu/rss/rss.html
function rss_init($url, $prefix)
{
	return true;
}

function rss_element_handler(
	$ns_url,	// URL of this element's namespace
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
				// See note on time at top of file
			break;
		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    case "lastBuildDate":
		switch ($parent)
		{
		    case "channel":
			// XXX - Make sure $children is a string.
			$retval['build_time'] = strtotime($children);
				// See note on time at top of file
			break;
		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    case "author":
		switch ($parent)
		{
		    case "item":
			// XXX - Make sure $children is a string
			// XXX - All we have is "author". We don't
			// know whether that's a name, address, or
			// URL. Assume it's a name and hope for the
			// best.
			$retval['author_name'] = $children;
			break;
		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    case "managingEditor":
		switch ($parent)
		{
		    case "channel":
			// XXX - Make sure $children is a string
			// XXX - All we have is "author". We don't
			// know whether that's a name, address, or
			// URL. Assume it's a name and hope for the
			// best.
			$retval['author_name'] = $children;
			break;
		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    case "category":
		switch ($parent)
		{
		    case "item":
		    case "channel":
			$retval['categories'][] = $children;
			break;
		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    case "cloud":
	    case "rating":
	    case "skipHours":
	    case "skipDays":
		switch ($parent)
		{
		    case "channel":
			// Not sure this is worth implementing.
			// Just ignore it.
			break;
		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    default:
//		echo "Warning: unknown RSS element: [$elt_name]\n";
		break;
	}

	return true;
}

/* rss_attribute_handler
 * Deal with RSS attributes.
 */
function rss_attribute_handler(
	$ns_url,	// URL of this element's namespace
	$prefix,	// Prefix of the attribute
	$name,		// Name of the attribute
	$value,		// Value of the attribute
	&$retval,	// Fill in the blanks
	&$context	// Where we are at the moment
	)
{
	// XXX - Check for prefix, and pass through.
	if ($prefix == "")
		$retval[$name] = $value;
	else
		$retval["$prefix:$name"] = $value;

	return true;
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

/* atom_init
 * Atom initialization hook. Do whatever is necessary to initialize
 * Atom.
 */
function atom_init($url, $prefix)
{
	return true;
}

function atom_element_handler(
	$ns_url,	// URL of this element's namespace
	$prefix,	// Namespace of this element
	$elt_name,	// Name of this element, without prefix
	&$attrs,	// Attributes
	&$children,	// Contents of this element
	&$retval,	// Fill in the blanks here
	&$context)	// Where we are at the moment
{
	if ($prefix != "")
	{
		/* If we're a guest (i.e., an Atom element inside a
		 * non-Atom XML file), just pass this element along.
		 */
		// XXX - Should put this in the other
		// *_element_handler()s.
		$retval["$prefix:$elt_name"] = $children;
		if ($children === NULL)
		{
			$retval["$prefix:$elt_name"] = array(
				"@attrs" => $attrs
				);
		} elseif (is_array($children))
		{
			$retval["$prefix:$elt_name"]['@attrs'] = $attrs;
		}
		return true;
	}

	$parent = $context[count($context)-1]['name'];
	switch ($elt_name)
	{
	    case "id":
		switch ($parent)
		{
		    case "feed":
		    case "entry":
			// GUID
			$retval['guid'] = $children;
			break;

		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
				// See note on time at top of file
			break;

		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//					echo "Warning: Unexpected replies type [$attrs[type]]\n";
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
//				echo "Warning: Unexpected link rel: [$attrs[rel]]\n";
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
//					echo "Warning: Unexpected replies type [$attrs[type]]\n";
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
//				echo "Warning: Unexpected link rel: [$attrs[rel]]\n";
				break;
			}
			break;

		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    case "published":
		switch ($parent)
		{
		    case "entry":
			// XXX - Make sure $children is a string.
			$retval['pub_time'] = strtotime($children);
				// See note on time at top of file
			break;

		    default:
			// XXX - Better error-reporting
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    default:
		// XXX - Better error-reporting
//		echo "Warning: unknown Atom element: [$elt_name]\n";
		break;
	}

	return true;
}

function atom_attribute_handler(
	$ns_url,	// URL of this element's namespace
	$prefix,	// Prefix of the attribute
	$name,		// Name of the attribute
	$value,		// Value of the attribute
	&$retval,	// Fill in the blanks
	&$context	// Where we are at the moment
	)
{
	// XXX - What attributes does Atom provide that we might care
	// about?
	return true;
}

/********** RDF ***********************************************/
function rdf_element_handler(
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
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
//			echo "Warning: Unexpected [$elt_name] parent: [$parent]\n";
			break;
		}
		break;

	    default:
		// XXX - Better error-reporting
//		echo "Warning: unknown RDF element: [$elt_name]\n";
		break;
	}

	return true;
}

/********** LiveJournal ****************************************/
// XXX - This belongs in a plugin
function lj_init($url, $prefix)
{
	return true;
}


/********** content ****************************************/
// XXX - This belongs in a plugin
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

/********** dc *********************************************/
// XXX - This belongs in a plugin
/* dc_element_handler
 * Handle the RSS Dublin Core extension:
 * http://purl.org/dc/elements/1.1/
 * http://dublincore.org/documents/dces/
 *
 * <dc:creator>name</dc:creator>
 *	The author of the post
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

	    default:
//		echo "Warning: unknown \"dc:\" element: [$elt_name]\n";
		break;
		
	}

	return true;
}

/********** Hooks *********************************************
 * These functions (and the bits that set them up as hooks, abuve)
 * should really go into separate functions.
 */

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

/* googleads_hook
 * Remove Google Ads ads at bottom of postings.
 */
/* XXX - This should go in a separate file, as a plug-in */
function googleads_hook($nodename, &$retval, &$context)
{
	if (!isset($retval['#text']))
		return;
	$retval['#text'] = preg_replace('{\r?\n<p><a href="http://feedads.googleadservices.com/[^\"]*"><img src="http://feedads.googleadservices.com/[^\"]*" border="0" ismap="true"></img></a></p>}',
					'',
					$retval['#text']);
}

/* feedburner_hook
 * Remove Feedburner link and bug at bottom of postings.
 */
/* XXX - This should go in a separate file, as a plug-in */
function feedburner_hook($nodename, &$retval, &$context)
{
	if (!isset($retval['#text']))
		return;
	/* There are two separate lines that can appear: an ad/survey
	 * link, and a web bug.
	 */
#echo "(feedburner) before: <pre>["; print_r($retval); echo "]</pre>\n";
	$retval['#text'] = preg_replace('{<div class="feedflare">\r?\n<a href="http://feeds2.feedburner.com/[^\"]*"><img src="http://feeds\d*.feedburner.com/[^\"]*" border="0"></img></a>\r?\n</div>}',
					'',
					$retval['#text']);
#echo "(feedburner) after 1: <pre>["; print_r($retval); echo "]</pre>\n";
	$retval['#text'] = preg_replace('{<img src="http://feeds\d*.feedburner.com/[^\"]*" height="1" width="1"/>\r?\n?}',
					'',
					$retval['#text']);
#echo "(feedburner) after 2: <pre>["; print_r($retval); echo "]</pre>\n";
}

/* feedburner_origlink_hook
 * Feedburner replaces the URL to a story with its own URL, presumably
 * in order to collect statistics, or something. Thankfully, it leaves
 * a "<feedburner:origLink>" element. This allows us to replace the
 * story's URL with the original, so we don't have to leapfrog through
 * feedproxy.google.com.
 */
/* XXX - This ought to be a separate plugin */
function feedburner_origlink_hook($nodename, &$retval, &$context)
{
	if (isset($retval['feedburner:origLink']))
	{
		if (isset($retval['url']))
			$retval['oldurl'] = $retval['url'];
		$retval['url'] = $retval['feedburner:origLink'];
	}
}

/* wordpress_noise_hook
 * Like googleads_hook, removes the image links and web bug that
 * wordpress adds to the bottom of postings.
 */
/* XXX - This ought to be a separate plugin */
function wordpress_noise_hook($nodename, &$retval, &$context)
{
	if (!isset($retval['#text']))
		return;
	# Image links
	$retval['#text'] = preg_replace('{\s*<a rel="nofollow" href="http://feeds.wordpress.com/1.0/[^\"]*"><img alt="" border="0" src="http://feeds.wordpress.com/1.0/[^\"]*" /></a>}',
					'',
					$retval['#text']);
	# Stats-tracking bug
	$retval['#text'] = preg_replace('{\s*<img alt="" border="0" src="http://stats.wordpress.com/b.gif[^\"]*" />}',
					'',
					$retval['#text']);
}
?>
