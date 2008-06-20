<?
/* rss.inc
 * Functions for parsing RSS (and similar) XML files.
 */
// XXX - Should probably inspect HTML in summary and content: if any
// links or images are relative, try to make them absolute. Or maybe
// there's a way in HTML/XHTML to specify a base URL.

# Make sure the fields set by parse_{rss,atom}{,_item} are the same:
# RSS:			Atom:			field
# <item>		<entry>			items[]
# <rss>			<feed>			feed_type
# <rss version>		-			rss_version
# <title>		<title>			title
# -			<subtitle>		subtitle
# <link>		<link rel=alternate>	url
# -			<link rel=replies type=html>	comment_url
# -			<link rel=replies type=xml>	comment_feed
# <description>					description
# <language>					language
# <copyright>					-
# <managingEditor>	<author><email>		author_email
#			<author><name>		author_name
#			<author><uri>		author_url
# <webMaster>					webmaster
#			<contributor>		feed_contributors[]
# <pubDate>					pub_time
# <lastBuildDate>	<updated>		build_time
# <category>		<category><term>	categories[][term]
#			<category><scheme>	categories[][scheme]
#			<category><label>	categories[][label]
# <generator>		<generator>text		generator
#			<generator uri>		generator_url
#			<generator version>	generator_version
# <docs>					-
# <cloud>					-
# <ttl>						ttl (secs)
# <image><url>		<icon>			image[url]
# <image><title>				image[title]
# <image><link>					image[link]
# <rating>					-
# <textInput>					-
# <skipHours>					-
# <skipDays>					-
#			<guid>			guid
#			<rights>		rights

# RSS item:		Atom entry		field
# <title>		<title>			title
# <link>		<link rel=alternate>	url
# <description>		<summary>		description
#			<content>		content
# <author>		<author><email>		author_email
#			<author><name>		author_name
#			<author><url>		author_url
# <category>		<category>		categories[]
# <comments>		<link rel=replies type=html>	comment_url
#			<link rel=replies type=xml>	comment_feed
# <enclosure>		<link {other}>?		enclosure{}
# <guid>		<guid>			guid
# <pubDate>		<published>		pub_time
#			<updated>		build_time
# <source>		<source>		-
#			<rights>		-

/* parse_feed
 * Given the text of a feed, parse it into a reasonable data structure.
 */
function parse_feed($rss_text)
{
	$rss = @simplexml_load_string($rss_text);
		// XXX - The '@' is to hide parsing error messages
		// from the user. In some cases at least, ought to
		// show them to the user.

	$rss_name = $rss->getName();	// Name of top-level element

	// XXX - Try to figure out what kind of feed this is.
	// RSS 0.91:
	// http://www.scripting.com/netscapeDocs/RSS%200_91%20Spec,%20revision%203.html
	//	RSS 0.91 should be upward-compatible with 2.0
	// RSS 0.92: http://backend.userland.com/rss092
	//	upward-compatible with 0.91 (and 0.92 is
	//	upward-compatible with 2.0)
	// RSS 2.0: http://cyber.law.harvard.edu/rss/rss.html
	// Atom: XXX


	// XXX - What follows is a rather naive implementation: each
	// element, whether with or without a prefix, is inside some
	// namespace. In the case of simple formats, this can be
	// implied.

	// XXX - RDF Site Summary: typepad uses this, so can't duck
	// out of supporting it. I think the Right Thing to do is to
	// preprocess the XML tree to resolve internal RDF links,
	// which (hopefully) will turn the tree into an ordinary RSS
	// or Atom document. RDF doco:
	// http://web.resource.org/rss/1.0/spec

	// Pick a function to parse the XML, based on the name of the
	// root element.
	switch ($rss_name) {
	    case "rss":
	    case "RDF":		// XXX - Need separate preprocessing
		return parse_rss($rss);

	    case "feed":
		return parse_atom($rss);

	    default:
		echo "Unrecognized feed element: [$rss_name]\n";
		return FALSE;
	}
}

/* parse_rss
 * Parse an RSS 0.91, 0.92, or 2.0 feed.
 */
function parse_rss($rss)
{
	$retval = array();		// A description of the feed
	$feed_categories = array();	// Categories a feed belongs
					// in (as far as it's
					// concerned).
	$retval['items'] = array();	// Items in this feed

	/* Set the type of feed */
	$rss_name = $rss->getName();	// Name of top-level element
	if ($rss_name == "rss")
	{
		$retval['feed_type'] = "RSS";

		// Top-level element (which we know to be <rss>) must have a
		// "version" attribute.
		$retval['rss_version'] = (string) $rss['version'];
	} else {
		// Assume that if we're in parse_rss but the first
		// element isn't <rss>, then it's an RDF feed.
		$retval['feed_type'] = "RDF";
	}

	// Top-level element must contain a single <channel>.
	// XXX - We don't actually check that there's only one. We
	// just take whatever SimpleXML gave us.
	$channel = $rss->channel[0];

	/* Parse channel elements */
	// A lot of these elements are currently ignored, but may be
	// of some use to plugins. E.g. someone may find it useful to
	// add color to a feed depending on its language.

	// XXX - Apparently $channel->children() gives only the
	// children in the default (no prefix) namespace. This is good
	// for a first pass, but for the more general case, we want to
	// be able to look through the list of declared namespaces
	// (with prefixes) and parse them as well, possibly with
	// plugins.
	foreach ($channel->children() as $elt => $value)
	{
		switch ($elt) {
		    // Mandatory elements
		    case "title":
		    	// title: name of this feed
			$retval['title'] = (string) $value;
			// XXX - Make sure this exists
			break;

		    case "link":
			// link: link to the feed's site
			$retval['url'] = (string) $value;
			// XXX - Make sure this exists
			break;

		    case "description":
			// description: description of the feed/site
			// XXX - This seems to be plain text in every
			// case. Is this a safe assumption?
			$retval['description'] = (string) $value;
			// XXX - Make sure this exists
			break;

		    // Optional channel elements
		    case "language":
			// language: the language the feed is written
			// in. Must be a two-letter abbreviation with
			// optional "-XX" country extension, e.g.,
			// "fr" or "en-us".
			// XXX - http://cyber.law.harvard.edu/rss/languages.html
			//	http://www.w3.org/TR/REC-html40/struct/dirlang.html#langcodes
			$retval['language'] = $value;
			break;

		    case "copyright":
			// copyright: copyright notice
			// Ignore.
			break;

		    case "managingEditor":
			// managingEditor: who's responsible for the
			// content: an email address (apparently with
			// optional comment in parens).
			// I think that in most cases, this is the
			// email address of the weblog author.
			$retval['author_email'] = $value;
			break;

		    case "webMaster":
			// webMaster: who's responsible for technical
			// issues: an email address
			$retval['webmaster'] = $value;
			break;

		    case "pubDate":
			// pubDate: publication time for the content,
			// in RFC 822 format, e.g.:
			//	Sat, 07 Sep 2002 00:00:01 GMT
			// (year may be 2 digits instead of 4).
			$retval['pub_time'] = strtotime($value);
			break;

		    case "lastBuildDate":
			// lastBuildDate: the last time the contents
			// changed (also apparently in RFC 822 format)
			// XXX - Perhaps use this as shortcut to
			// decide whether it's worth doing anything
			// else with the feed: cache the pubDate, and
			// if it hasn't changed, don't do anything.
			// XXX - Likewise with posts: if a post hasn't
			// changed since we last looked at it, don't
			// bother parsing it.
			// XXX - In LJ feeds, apparently lastBuildDate
			// refers to the time when the RSS feed was
			// updated. Since it's generated on the fly,
			// that's pretty useless.
			$retval['build_time'] = strtotime($value);
			break;

		    case "category":
			// category: zero or more categories in which
			// the channel/item belongs.
			// The assoc is for compatibility with Atom
			// XXX - Find a test case that has categories
			$feed_categories[] = array(
				"term"	=> (string) $value
				);
			break;

		    case "generator":
			// generator: name of program that generated
			// the RSS. Not terribly useful for now, but
			// may be of value if we ever need to work
			// around some version-specific quirks or
			// something.
			$retval['generator'] = (string) $value;
			break;

		    case "docs":
			// docs: URL pointing to documentation for
			// format used in this RSS file.
			// Ignore.
			break;

		    case "cloud":
			// cloud: not sure what this is
			// XXX - Deal with this if it becomes desirable
			// Ignore for now.
			break;

		    case "ttl":
			// ttl: time to live: how long a channel
			// should be cached before refreshing, in
			// minutes.
			// We convert to seconds, just cos.
			// XXX - Use this if set. Don't refresh feeds
			// younger than this.
			// XXX - Perhaps allow overriding?
			$retval['ttl'] = $value * 60;
			break;

		    case "image":
			// image: image to display with the channel.
			// <url>: URL to image
			// <title>: image title. Use this for "alt" in HTML.
			// <link>: link to the site
			$retval['image'] = array(
				"url"	=> (string) $value->url,
				"title"	=> (string) $value->title,
				"link"	=> (string) $value->link
				);
			break;

		    case "rating":
			// rating: PICS rating for this channel.
			// XXX - Deal with this if it becomes desirable.
			break;

		    case "textInput":
			// textInput: text input box that goes to an
			// arbitrary CGI script. dKos uses this for a
			// dKos search function. Most aggregators
			// ignore this.
			// We'll ignore it too.
			# $feed_textinput_title = $value->title;	// Label
			# $feed_textinput_desc = $value->description;
			# $feed_textinput_name = $value->name;	// Name of input field
			# $feed_textinput_link = $value->link;	// CGI script
			break;

		    case "skipHours":
			// Hint to aggregators: don't update during
			// these hours. An array of <hour>s.
			// 0 = midnight GMT.
			// Ignore.
			break;

		    case "skipDays":
			// Hint to aggregators: don't update during
			// these days. An array of <day> elements:
			// "<day>Monday</day>", ... , "<day>Sunday</day>".
			// Ignore.
			break;

			// XXX - If we get this far, and the value has
			// a prefix (e.g., "lj:mood"), see if there's
			// a handler for the namespace, and pass it
			// along to it.
		    case "item":
			$item = parse_rss_item($value);
			if ($item !== NULL)
				$retval['items'][] = $item;
			break;

		    default:
			# XXX - Abort somehow
			echo "Unrecognized channel element: [[$elt]]\n";
#			break;
		}
	}

	// XXX - Should we make sure mandatory channel elements exist?

	// XXX - Do something smart.
	$retval['categories'] = $feed_categories;

	// Stupid hack for RDF: yes, it can pass through an RSS 1.0
	// parser that doesn't look too closely. But while an RSS
	// document has
	//	<channel>
	//	  <item>...
	//	  <item>...
	// RDF has
	//	<channel>
	//	  <items>...</items>
	//	</channel>
	//	<item>...
	//	<item>...
	// So we need to look outside the <channel> to find the actual
	// items.
	if ($rss->getName() == "RDF")
	{
		foreach ($rss->children() as $elt => $value)
		{
			if ($elt != "item")
				continue;
			$item = parse_rss_item($value);
			if ($item !== NULL)
				$retval['items'][] = $item;
		}
	}

	return $retval;
}

// parse_rss_item
// Parse an RSS (2.0) <item> element.
function parse_rss_item($item)
{
	$retval = array();		// Return value
	$item_categories = array();	// Categories item belongs to

	// XXX - The same namespace considerations that apply to <channel>
	// also apply here.
	foreach ($item->children() as $elt => $value)
	{
		switch ($elt) {
		    case "title":
			// title: title of the item
			$retval['title'] = (string) $value;
			break;

		    case "link":
			// link: URL of the item
			$retval['url'] = (string) $value;
			break;

		    case "description":
			// description: synopsis of the item
			$retval['description'] = (string) $value;
			break;

		    case "author":
			// author: email address of the author
			$retval['author_email'] = (string) $value;
			break;

		    case "category":
			// category: category in which the item goes.
			// There can be several of these, so put them
			// in an array.
			$retval_categories[] = (string) $value;
					// Cast to string: we don't want
					// the full XML object
			break;

		    case "comments":
			// comments: URL of page where comments can be
			// read. This is a regular page, not an RSS
			// feed.
			$retval['comment_url'] = (string) $value;
			break;

		    case "enclosure":
			// enclosure: a media item attached to the item.
			// required attributes:
			//	url: link to the item
			//	length: size in bytes
			//	type: MIME type of attachment
			// XXX - If we deal with this, we should
			// probably handle the case where there are
			// more than one enclosures.
			$tmp = $value->attributes();
			$retval['enclosure'] = array(
				"url"		=>
					// XXX - Not sure whether this
					// is always needed.
					// Apparently atheistmedia
					// double-encodes the
					// enclosure URL. It's decoded
					// once by simpleXML, but we
					// need to do it again.
					htmlspecialchars_decode($tmp['url']),
				"length"	=> $tmp['length'],
				"type"		=> $tmp['type']
				);
			break;

		    case "guid":
			// guid: globally unique identifier. Often
			// looks like a URL, but it's really just a
			// string.
			// XXX - Perhaps should use this as database key.
			$retval['guid'] = (string) $value;
			break;

		    case "pubDate":
			// pubDate: date when item was published, as
			// an RFC 822 string.
			$retval['pub_time'] = strtotime($value);
			// XXX - If we've cached this, we can use this
			// as a heuristic, and not update the database
			// if the item hasn't changed since we last
			// saw it.
			break;

		    case "source":
			// source: the URL of the channel that this
			// item came from. Doesn't seem to be used.
			// XXX - Deal with this when it becomes
			// desirable. Ignore for now.
			break;

		    default:
			# XXX - Abort cleanly
			echo "Unrecognized item element: [[$elt]]\n";
			break;
		}
	}

	return $retval;
}

/* parse_atom
 * Parse an Atom feed.
 */
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
function parse_atom($rss)
{
	$retval = array();
	$feed_categories = array();	// Categories the feed claims
					// to belong to
	$feed_contributors = array();	// People who have contributed
					// to this feed
	$retval['items'] = array();	// Entries in this feed

	// Theoretically, an Atom feed can be just a single item. In
	// practice, we should probably assume it's a feed.

	$retval['feed_type'] = "Atom";
	// XXX - Perhaps abort if $name != "feed".

	foreach ($rss->children() as $elt => $value)
	{
		switch ($elt) {
		    case "author":
			// atomAuthor: author of the feed
			$person = atom_person($value);
			$retval['author_name']  = $person['name'];
			$retval['author_url']   = $person['url'];
			$retval['author_email'] = $person['email'];
			break;

		    case "category":
			// atomCategory: category, and optional scheme
			// and label for this feed.
			$attrs = $value->attributes();
			$feed_categories[] = array(
				"term"		=> $attrs['term'],
				"scheme"	=> $attrs['scheme'],
				"label"		=> $attrs['label']
				);
			break;

		    case "contributor":
			// atomContributor: someone who contributed to
			// this feed.
			// XXX - Not used in practice, apparently
			$person = atom_person($value);
			$feed_contributors[] = $person;
			break;

		    case "generator":
			// atomGenerator: the software that wrote this feed
			$attrs = $value->attributes();
			$retval['generator'] = (string) $value[0];
			if (isset($attrs['uri']))
				$retval['generator_url'] = $attrs['uri'];
			if (isset($attrs['version']))
			$retval['generator_version'] = $attrs['version'];
			break;

		    case "icon":
			// atomIcon: an icon that goes with this feed.
			// The assoc is for compatibility with RSS's
			// <image>.
			$retval['image'] = array(
				"url"	=> (string) $value[0],
				);
			break;

		    case "id":
			// atomId: GUID for this feed
			$retval['guid'] = (string) $value[0];
			break;

		    case "link":
			// atomLink: links to various documents
			// pertaining to this feed.
			$attrs = $value->attributes();

			// There's an infinite number of possible
			// '<link rel's, and we only care about a few
			// of them. Look for those and ignore the
			// others.
			$link_rel = $attrs['rel'];
			$link_type = $attrs['type'];
			$link_href = $attrs['href'];
			if ($link_rel == "alternate")
			{
				// Real page for this feed
				$retval['url'] = (string) $link_href;
			} elseif ($link_rel == "replies" &&
				  $link_type == "text/html")
			{
				// Page where comments can be found
				$retval['comment_url'] = $attrs['href'];
			} elseif ($link_rel == "replies" &&
				  $link_type == "application/atom+xml")
			{
				// Feed for comments on this post
				$retval['comment_feed'] = $attrs['href'];
			}
			break;

		    case "logo":
			// atomLogo: a logo that goes with this feed.
			// XXX - Not sure how it differs from an icon,
			// except that an icon should be small and
			// square, whereas a logo should have an
			// aspect ratio of 2:1.
			$retval['logo'] = (string) $value[0];
			break;

		    case "rights":
			// atomRights: copying rights and whatnot.
			// XXX - I don't have an example of this in use.
			$retval['rights'] = $value;
			break;

		    case "subtitle":
			// atomSubtitle: feed subtitle
			$retval['subtitle'] = atom_text_construct($value);
			break;

		    case "title":
			// atomTitle: feed title
			$retval['title'] = atom_text_construct($value);
			break;

		    case "updated":
			// atomUpdated: when the feed was last updated
			$retval['build_time'] = strtotime($value[0]);
			break;

		    case "entry":
			// atomEntry: an entry in this feed
			$entry = parse_atom_entry($value);
			if ($entry !== NULL)
				$retval['items'][] = $entry;
			break;

		    default:
			// XXX - Abort gracefully
			echo "Unrecognized feed element [$elt]\n";
			break;
		}
	}

	return $retval;
}

// parse_atom_entry
// Parse an Atom <entry> tag.
function parse_atom_entry($entry)
{
	$retval = array();		// Return value
	$entry_categories = array();	// Categories entry belongs in

	foreach ($entry->children() as $elt => $value)
	{
		switch ($elt) {
		    case "author":
			// atomAuthor: author of this entry
			$person = atom_person($value);
			$retval['author_name']  = (string) $person['name'];
			$retval['author_url']   = (string) $person['url'];
			$retval['author_email'] = (string) $person['email'];
			break;

		    case "category":
			// atomCategory: category, and optional scheme
			// and label, for this entry.
			$attrs = $value->attributes();
			$entry_categories[] = array(
				"term"		=> (string) $attrs['term'],
				"scheme"	=> (string) $attrs['scheme'],
				"label"		=> (string) $attrs['label']
				);
			break;

		    case "content":
			// atomContent: the actual content of the entry.
			$attrs = $value->attributes();
			switch ($attrs['type']) {
			    case "text":
			    case "html":
			    case "xhtml":
				$retval['content'] = atom_text_construct($value);
				break;

			    default:
				// XXX - Better error-reporting
				echo "Unrecognized content type: [$attrs[type]]\n";
$retval['content2'] = $value;
				// XXX - Actually, this could be link
				// to external content, and this is legal.
				// XXX - Find a test case.
				break;
			}
			break;

		    case "contributor":
			// atomContributor
			$person = atom_person($value);
			$retval['contributors'] = $person;
			break;

		    case "id":
			// atomId: GUID for this entry
			$retval['guid'] = (string) $value[0];
			break;

		    case "link":
			// atomLink: links to various documents
			// pertaining to this entry.
			$attrs = $value->attributes();

			// There's an infinite number of possible
			// '<link rel's, and we only care about a few
			// of them. Look for those and ignore the
			// others.
			$link_rel  = (string) $attrs['rel'];
			$link_type = (string) $attrs['type'];
			$link_href = (string) $attrs['href'];
			if ($link_rel == "alternate")
			{
				// Real page for this entry
				$retval['url'] = $link_href;
			} elseif ($link_rel == "replies" &&
				  $link_type == "text/html")
			{
				// Link to comments on this entry
				$retval['comment_url'] = $link_href;
			} elseif ($link_rel == "replies" &&
				  $link_type == "application/atom+xml")
			{
				// Link to feed for comments on this entry
				$retval['comment_feed'] = $link_href;
			}
			break;

		    case "published":
			// atomPublished: when the entry was first published.
			$retval['pub_time'] = strtotime($value[0]);
			break;

		    case "rights":
			// atomRights: copying rights and whatnot.
			// XXX - I don't have an example of this in use.
			break;

		    case "source":
			// atomSource: metadata from when this entry
			// was moved here from other feed.
			// XXX - Try this again when there's an
			// example to test with.
			break;

		    case "summary":
			// atomSummary: summary/abstract of the entry.
			$retval['description'] = atom_text_construct($value);
			break;

		    case "title":
			// atomTitle: entry title
			$retval['title'] = atom_text_construct($value);
			break;

		    case "updated":
			// atomUpdated: when the entry was last updated
			$retval['build_time'] = strtotime($value[0]);
			break;

		    default:
			// XXX - Better error-reporting
			echo "Unrecognized entry element: [$elt]\n";
			break;
		}
	}

	$retval['categories'] = $entry_categories;

	return $retval;
}

/* atom_text_construct
 * Parse an atomTextConstruct node, and return it as a string of HTML
 * that can be sent to a browser.
 */
function atom_text_construct($node)
{
	$attrs = $node->attributes();

	switch ($attrs['type']) {
	    case "text":
	    case "":		// No type -> assume text
		// This is a text node. Its value has already been
		// HTML-escaped for the XML feed. So leave it alone.
		return (string) $node[0];

	    case "html":
		// This is an HTML node. Its value has had the HTML
		// markup escaped so that it doesn't interfere with
		// the surrounding XML. Unescape one level
		return htmlspecialchars_decode($node[0]);

	    case "xhtml":
		// This is an XHTML node: it's inside a <div>, but is
		// otherwise fine the way it is. Strip off the <div>.
		// XXX - I don't have any examples of this to test with.
		return (string) $node[0]->children();

	    default:
		// XXX - Better error-reporting
		echo "Unrecognized atomTextConstruct type: [$attrs[type]]\n";
		break;
	}
}

/* atom_person
 * Parse an atomPersonConstruct node, and return it as an array with
 * keys "name", "url", and "email".
 */
function atom_person($node)
{
	// XXX - Error-checking?
	$retval = array(
		"name"	=> $node->name,
		"url"	=> $node->uri,
		"email"	=> $node->email
		);
	return $retval;
}
?>
