<?
// XXX - Read a feed (retrieved by getfeed.php, or otherwise), parse
// it, and return a structure containing the interesting bits.

// XXX - Look up the specs for different feed types (RSS 0.91, 0.92,
// 2.0, Atom) and figure them out.

if (!file_exists($argv[1]))
{
	echo "Usage: php parsefeed.php somefile\n";
	exit(1);
}
$rss_text = file_get_contents($argv[1]);

$rss = @simplexml_load_string($rss_text);
		// XXX - We use simplexml_load_string() here instead
		// of the more obvious simplexml_load_file() because
		// in the production version, we'll get the raw XML as
		// a string from Curl.
		// XXX - The '@' is to hide pars error messages from
		// user. Perhaps these should be captured and
		// displayed separately.
// XXX - Error-checking: make sure it actually loaded.
#echo "rss: [["; print_r($rss); echo "]]\n";
if (isset($rss->channel->items))
{
#	foreach ($rss->channel->items as $i)
#	{
	$i = $rss->channel->items[0];
		echo "Channel item: [["; print_r($i); echo "]]\n";
#	}
}

// XXX - AIUI, getDocNamespaces() returns the namespaces that are
// declared, whereas getNamespaces() returns the ones that are
// actually used.
// Experimentation shows that if there's an element with an undeclared
// namespace, it won't show up in either list. So don't use those. But
// if a declaration is buried deep within the document, it'll show up
// in getDocNamespaces(true). So use that.
// Also, while getDocNamespaces() returns the list of declared
// namespaces, there doesn't seem to be a good way of culling the ones
// that are never used. ICBW.
$rss_name = $rss->getName();	// Name of top-level element
echo "name: [[$rss_name]]\n";
$rss_doc_namespaces = $rss->getDocNamespaces(false);
echo "rss doc namespaces (root): [["; print_r($rss_doc_namespaces); echo "]]\n";
$rss_doc_namespaces = $rss->getDocNamespaces(true);
echo "rss doc namespaces (all):  [["; print_r($rss_doc_namespaces); echo "]]\n";

$rss_namespaces = $rss->getNamespaces(false);
echo "rss namespaces (root): [["; print_r($rss_namespaces); echo "]]\n";
$rss_namespaces = $rss->getNamespaces(true);
echo "rss namespaces(all):  [["; print_r($rss_namespaces); echo "]]\n";
//echo "rss: [["; print_r($rss); echo "]]\n";
echo "Attributes: [["; print_r($rss->attributes); "]]\n";

// XXX - Try to figure out what kind of feed this is.
// RSS 0.91: http://www.scripting.com/netscapeDocs/RSS%200_91%20Spec,%20revision%203.html
//	RSS 0.91 should be upward-compatible with 2.0
// RSS 0.92: http://backend.userland.com/rss092
//	upward-compatible with 0.91 (and 0.92 is upward-compatible with
//	2.0
// RSS 2.0: http://cyber.law.harvard.edu/rss/rss.html
// Atom: XXX

// XXX - Some namespaces seem to be fairly common, e.g.
//	xmlns:content="http://purl.org/rss/1.0/modules/content/"
// Should have built-in handlers for these. Also ought to have plugins to
// add one's own handlers (hooks).

// XXX - Actually, what follows is a rather naive implementation: each
// element, whether with or without a prefix, is inside some
// namespace. In the case of simple formats, this can be implied.
//
// At the top level, try to figure out which namespaces are in use
// (from $rss->getDocNamespaces(true) if possible, or from guesswork
// otherwise), and register appropriate handlers.
//
// Leave this for the next version, though.

// XXX - Presumably there are two functions we should use: one for
// parsing the RSS feed, and another for parsing the individual
// elements. Anything more complex than that can be handled by
// specialized functions.

// XXX - See whether there's a default namespace (one without a
// prefix; shows up with key "" in the list of namespaces), and try to
// use that first. If there isn't one, try to guess by looking at the
// topmost element.

// XXX - RDF Site Summary: typepad uses this, so can't duck out of
// supporting it. I think the Right Thing to do is to preprocess the
// XML tree to resolve internal RDF links, which (hopefully) will turn
// the tree into an ordinary RSS or Atom document.
// RDF doco: http://web.resource.org/rss/1.0/spec

// Pick a function to parse the XML, based on the name of the root element.
switch ($rss_name) {
    case "rss":
    case "RDF":		// XXX - Need separate preprocessing
	parse_rss($rss);
	break;
    case "feed":
	parse_atom($rss);
	break;
    default:
	echo "Unrecognized feed element: [$rss_name]\n";
	exit(1);
}

// parse_rss
// Parse an RSS 0.91, 0.92, or 2.0 feed.
function parse_rss($rss)
{
echo "Inside parse_rss()\n";
#	global $rss_doc_namespaces;

#	echo "rss: [["; print_r($rss); echo "]]\n";

	// Top-level element (which we know to be <rss>) must have a
	// "version" attribute.
	$rss_version = $rss['version'];
	echo "RSS version: [[$rss_version]]\n";

	// Top-level element must contain a single <channel>.
	// XXX - We don't actually check that there's only one. We
	// just take whatever SimpleXML gave us.
	$channel = $rss->channel[0];
#	echo "channel: [["; print_r($channel); echo "]]\n";

	// Parse channel elements
	echo "Channel elements:\n";
	// XXX - Apparently $channel->children() gives only the
	// children in the default (no prefix) namespace. This is good
	// for a first pass, but for the more general case, we want to
	// be able to look through the list of declared namespaces
	// (with prefixes) and parse them as well, possibly with
	// plugins.
$ns = $channel->items->getNamespaces(true);
echo "Namespaces in <items>: [["; print_r($ns); echo "]]\n";

echo "Looking for RDF children:\n";
#foreach ($channel->items->children("http://www.w3.org/1999/02/22-rdf-syntax-ns#", false) as $elt => $value)
foreach ($channel->items->children("rdf", true) as $elt => $value)
{
	echo "\tRDF elt [$elt] => [$value]\n";
}

#	foreach ($channel->children("http://www.livejournal.org/rss/lj/1.0/", false) as $elt => $value)
#	foreach ($channel->children("", false) as $elt => $value)
#	foreach ($rss_doc_namespaces as $nsprefix => $ns)
#	{
#		echo "Using namespace [$nsprefix] => [$ns]\n";
#	foreach ($channel->children($ns, false) as $elt => $value)
	foreach ($channel->children() as $elt => $value)
	{
		echo "Element [[$elt]] => [$value]\n";
		switch ($elt) {
		    // Mandatory elements
		    case "title":
		    	// title: name of this feed
			$feed_title = $value;
			// XXX - Make sure this exists
			echo "feed title: [[$feed_title]]\n";
			break;

		    case "link":
			// link: link to the feed's site
			$feed_link = $value;
			// XXX - Make sure this exists
			echo "feed link: [[$feed_link]]\n";
			break;

		    case "description":
			// description: description of the feed/site
			$feed_desc = $value;
			// XXX - Make sure this exists
			echo "feed description: [[$feed_desc]]\n";
			break;

		    // Optional channel elements
		    case "language":
			// language: the language the feed is written
			// in. Must be a two-letter abbreviation with
			// optional "-XX" country extension, e.g.,
			// "fr" or "en-us".
			// XXX - http://cyber.law.harvard.edu/rss/languages.html
			//	http://www.w3.org/TR/REC-html40/struct/dirlang.html#langcodes

			$feed_lang = $value;
			echo "Language: [[$feed_lang]]\n";
			break;

		    case "copyright":
			// copyright: copyright notice
			$feed_copyright = $value;
			echo "Copyright: [[$feed_copyright]]\n";
			break;

		    case "managingEditor":
			// managingEditor: who's responsible for the
			// content: an email address (apparently with
			// optional comment in parens).
			$feed_editor = $value;
			echo "Managing editor: [[$feed_editor]]\n";
			break;

		    case "webMaster":
			// webMaster: who's responsible for technical
			// issues: an email address
			$feed_webmaster = $value;
			echo "Webmaster: [[$feed_webmaster]]\n";
			break;

		    case "pubDate":
			// pubDate: publication time for the content,
			// in RFC 822 format, e.g.:
			//	Sat, 07 Sep 2002 00:00:01 GMT
			// (year may be 2 digits instead of 4).
			// XXX - Use strtotime($date) to convert to time_t.
			$feed_pubdate = $value;
			echo "Publication date: [[$feed_pubdate]]\n";

		    case "lastBuildDate":
			// lastBuildDate: the last time the contents
			// changed (also apparently in RFC 822 format)
			// XXX - Use strtotime($date) to convert to time_t.
			// XXX - Perhaps use this as shortcut to
			// decide whether it's worth doing anything
			// else with the feed: cache the pubDate, and
			// if it hasn't changed, don't do anything.
			// XXX - Likewise with posts: if a post hasn't
			// changed since we last looked at it, don't
			// bother parsing it.
			$feed_lastbuild = $value;
			echo "Last build: [[$feed_lastbuild]]\n";
			break;

		    case "category":
			// category: zero or more categories in which
			// the channel/item belongs.
			// XXX - Find a test case that has categories
			echo "Feed categories: [[", $value, "]]\n";
			break;

		    case "generator":
			// generator: name of program that generated the RSS
			$feed_generator = $value;
			echo "Generator: [[$feed_generator]]\n";
			break;

		    case "docs":
			// docs: URL pointing to documentation for
			// format used in this RSS file.
			$feed_docs = $value;
			echo "Docs: [[$feed_docs]]\n";
			break;

		    case "cloud":
			// cloud: not sure what this is
			// XXX - Deal with this if it becomes desirable
			break;

		    case "ttl":
			// ttl: time to live: how long a channel
			// should be cached before refreshing, in
			// minutes.
			// XXX - Convert to seconds, for consistency?
			// XXX - Use this if set. Don't refresh feeds
			// younger than this.
			// XXX - Perhaps allow overriding?
			$feed_ttl = $value;
			echo "TTL: [[$feed_ttl]]\n";
			break;

		    case "image":
			// image: image to display with the channel.
			// <url>: URL to image
			// <title>: image title. Use this for "alt" in HTML.
			// <link>: link to the site
			$feed_image_url = $value->url;
			$feed_image_title = $value->title;
			$feed_image_link = $value->link;
			echo "Image:\n",
				"\turl: [[$feed_image_url]]\n",
				"\ttitle: [[$feed_image_title]]\n",
				"\tlink: [[$feed_image_link]]\n";
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
			$feed_textinput_title = $value->title;	// Label
			$feed_textinput_desc = $value->description;
			$feed_textinput_name = $value->name;	// Name of input field
			$feed_textinput_link = $value->link;	// CGI script
			echo "Text input:\n",
				"\ttitle: [[$feed_textinput_title]]\n",
				"\tdesc: [[$feed_textinput_desc]]\n",
				"\tname: [[$feed_textinput_name]]\n",
				"\tlink: [[$feed_textinput_link]]\n";
			break;

		    case "skipHours":
			// Hint to aggregators: don't update during
			// these hours. An array of <hour>s.
			// 0 = midnight GMT.
		    case "skipDays":
			// Hint to aggregators: don't update during
			// these days. An array of <day> elements:
			// "<day>Monday</day>", ... , "<day>Sunday</day>".

			// XXX - If we get this far, and the value has
			// a prefix (e.g., "lj:mood"), see if there's
			// a handler for the namespace, and pass it
			// along to it.
			break;

		    case "item":
			parse_rss_item($value);
			break;

		    default:
			echo "Unrecognized channel element: [[$elt]]\n";
#			break;
		}
	}

	// XXX - Make sure mandatory channel elements exist.

	// XXX - Do something smart.

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
			echo "Found an RDF <item>\n";
			parse_rss_item($value);
		}
	}
}

// parse_rss_item
// Parse an RSS (2.0) <item> element.
function parse_rss_item($item)
{
echo "Inside parse_rss_item()\n";
#echo "\titem [["; print_r($item); echo "]]\n";
	// XXX - The same namespace considerations that apply to <channel>
	// also apply here.
	foreach ($item->children() as $elt => $value)
	{
		echo "\titem element: [[$elt]] => [[$value]]\n";
		switch ($elt) {
		    case "title":
			// title: title of the item
			$item_title = $value;
			echo "Item title: [[$item_title]]\n";
			break;

		    case "link":
			// link: URL of the item
			$item_url = $value;
			echo "Item URL: [[$item_url]]\n";
			break;

		    case "description":
			// description: synopsis of the item
			$item_desc = $value;
			echo "Item description: [[$item_desc]]\n";
			break;

		    case "author":
			// author: email address of the author
			$item_author = $value;
			echo "Item author: [[$item_author]]\n";
			break;

		    case "category":
			// category: category in which the item goes.
			// There can be several of these, so put them
			// in an array.
			$item_categories[] = (string) $value;
					// Cast to string: we don't want
					// the full XML object
			echo "Categories: "; print_r($item_categories); echo "\n";
			break;

		    case "comments":
			// comments: URL of page where comments can be
			// read. This is a regular page, not an RSS
			// feed.
			$item_comment_link = $value;
			echo "Comments at [[$item_comment_link]]\n";
			break;

		    case "enclosure":
			// enclosure: a media item attached to the item.
			// required attributes:
			//	url: link to the item
			//	length: size in bytes
			//	type: MIME type of attachment
			// XXX - The URL
			$tmp = $value->attributes();
			$item_encl_url = $tmp['url'];
			$item_encl_len = $tmp['length'];
			$item_encl_type = $tmp['type'];
			echo "Enclosure:\n",
				"\tURL: [[$item_encl_url]]\n",
				"\tlength: [[$item_encl_len]]\n",
				"\ttype: [[$item_encl_type]]\n";
			// XXX - Not sure whether this is always
			// needed. Apparently atheistmedia
			// double-encodes the enclosure URL. It's
			// decoded once by simpleXML, but we need to
			// do it again.
			$url2 = htmlspecialchars_decode($item_encl_url);
			echo "decoded URL: [[$url2]]\n";
			break;

		    case "guid":
			// guid: globally unique identifier. Often
			// looks like a URL, but it's really just a
			// string.
			// XXX - Perhaps should use this as database key.
			$item_guid = $value;
			echo "GUID: [[$item_guid]]\n";
			break;

		    case "pubDate":
			// pubDate: date when item was published, as
			// an RFC 822 string.
			$item_pubdate = $value;
			// XXX - Use strtotime() to convert to time_t.
			// XXX - If we've cached this, we can use this
			// as a heuristic, and not update the database
			// if the item hasn't changed since we last
			// saw it.
			echo "pubDate: [[$item_pubdate]]\n";
			break;

		    case "source":
			// source: the URL of the channel that this
			// item came from. Doesn't seem to be used.
			// XXX - Deal with this when it becomes
			// desirable.
			break;

		    default:
			echo "Unrecognized item element: [[$elt]]\n";
			break;
		}
	}
}

function parse_atom($rss)
{
echo "Inside parse_atom()\n";
// Summary of http://tools.ietf.org/html/rfc4287 :
// start = atomFeed | atomEntry
// atomCommonAttributes =
//	attribute xml:base { atomUri }?,
//	attribute xml:lang { atomLanguageTag }?,
//	undefinedAttribute
// Any element can have xml:base attribute, giving base for relative
//	links
// Any element can have xml:lang attribute
//
// atomPlainTextConstruct =
//	atomCommonAttributes,
//	attribute type { "text" | "html" }?,
//	text
// atomXHTMLTextConstruct =
//	atomCommonAttributes,
//	attribute type { "xhtml" },
//	xhtmlDiv
// atomTextConstruct = atomPlainTextConstruct | atomXHTMLTextConstruct
// "type" attribute must be one of "text", "html", or "xhtml".
// Defaults to "text". Text and HTML elements have no children.
// HTML text can be stuck inside a <div>.
// XHTML element must be a single <div>; strip off the <div> before
// displaying.
// Markup:
//	text: string has been HTML-escaped for the feed. So leave it alone:
//		it's already HTML-escaped.
//	html: string has been HTML-escaped so that HTML sequences in the
//		string don't interfere with XML markup. Unescape one level
//		before displaying.
//	xhtml: string is already XHTML. Leave it alone.
//
// atomPersonConstruct =
//	atomCommonAttributes,
//	(element atom:name { text }
//	 & element atom:uri { atomUri }?
//	 & element atom:email { atomEmailAddress }?
//	 & extensionElement*)
// Gives person's printable name, URL, and email address (RFC 822 format)
//
// atomDateConstruct =
//	atomCommonAttributes,
//	xsd:dateTime
//
// Containers:
// atomFeed =
//	element atom:feed {
//		atomCommonAttributes,
//		(atomAuthor*
//		 & atomCategory*
//		 & atomContributor*
//		 & atomGenerator?
//		 & atomIcon?
//		 & atomId
//		 & atomLink*
//		 & atomLogo?
//		 & atomRights?
//		 & atomSubtitle?
//		 & atomTitle
//		 & atomUpdated
//		 & extensionElement*),
//		atomEntry*
//	}
//
// atomEntry =
//	element atom:entry {
//		atomCommonAttributes,
//		(atomAuthor*
//		 & atomCategory*
//		 & atomContent?
//		 & atomContributor*
//		 & atomId
//		 & atomLink*
//		 & atomPublished?
//		 & atomRights?
//		 & atomSource?
//		 & atomSummary?
//		 & atomTitle
//		 & atomUpdated
//		 & extensionElement*)
//	}
//
// atom:content contains either content, or link to content
// atomInlineTextContent =
//	element atom:content {
//		atomCommonAttributes,
//		attribute type { "text" | "html" }?,
//		(text)*
//	}
// atomInlineXHTMLContent =
//	element atom:content {
//		atomCommonAttributes,
//		attribute type { "xhtml" },
//		xhtmlDiv
//	}
// atomInlineOtherContent =
//	element atom:content {
//		atomCommonAttributes,
//		attribute type { atomMediaType }?,
//		(text|anyElement)*
//	}
// atomOutOfLineContent =
//	element atom:content {
//		atomCommonAttributes,
//		attribute type { atomMediaType }?,
//		attribute src { atomUri },
//		empty
//	}
// atomContent = atomInlineTextContent
//	| atomInlineXHTMLContent
//	| atomInlineOtherContent
//	| atomOutOfLineContent
//
// "src" attribute must be IRI (Internationalized Resource Identifier;
// probably a URL in practice, albeit with i18n characters).
//
// Metadata elements:
// atomAuthor = element atom:author { atomPersonConstruct }
// atomCategory =
//	element atom:category {
//		atomCommonAttributes
//		attribute term { text }
//		attribute scheme { atomUri }?,
//		attribute label { text }?,
//		undefinedContent
//	}
//	"term" is a category name, and "scheme" identifies a
//	categorization scheme. Thus, presumably a bunch of political
//	sites could get together and decide that category "MT" in the
//	scheme "http://politics.com/" refers to Montana.
//	"label" is a human-readable label. Not used in practice.
//
// atomContributor = element atom:contributor { atomPersonConstruct }
//	Person who contributed to this feed/entry
//
// atomGenerator = element atom:generator {
//	atomCommonAttributes,
//	attribute uri { atomUri }?,
//	attribute version { text }?,
//	text
//	The software that generated this feed. XXX - Might be useful for
//	hints on working around bugs and such.
//
// atomIcon = element atom:icon {
//	atomCommonAttributes,
//	(atomUri)
//	An icon that goes with the feed.
//
// atomId = element atom:id {
//	atomCommonAttributes,
//	(atomUri)
//	}
//	GUID for this feed/entry
//
// atomLink =
//	element atom:link {
//		atomCommonAttributes,
//		attribute href { atomUri },
//		attribute rel { atomNCName | atomUri }?,
//		attribute type { atomMediaType }?,
//		attribute hreflang { atomLanguageTag }?,
//		attribute title { text }?
//		attribute length { text }?,
//		undefinedContent
//	}
//	Link to something on the web. "href" attribute is mandatory.
//	XXX - "title" can be used for "alt", I guess
//	Apparently:
//	- "link rel=alternate" points to real page for this item.
//	- "link rel=replies type=text/html" points to comments page
//	- "link rel=replies type=application/atom+xml" points to comments feed
//
// atomLogo = element atom:logo {
//	atomCommonAttributes,
//	(atomUri)
//	}
//	Link to an image that identifies the feed.
//
// atomPublished = element atom:published { atomDateConstruct }
//	When item was first published (not when it was revised/edited).
//
// atomRights = element atom:rights { atomTextConstruct }
//	Information about rights.
//
// atomSource
//	Used to preserve metadata when copying items from one feed to
//	another. Doesn't seem to be used in practice.
//	XXX - Use this when it becomes desirable.
//
// atomSubtitle = element atom:subtitle { atomTextConstruct }
//	Feed's subtitle, human-readable.
//
// atomSummary = element atom:summary { atomTextConstruct }
//	Summary/abstract of an entry.
//
// atomTitle = element atom:title { atomTextConstruct }
//	Title of the entry/feed, human-readable.
//
// atomUpdated = element atom:updated { atomDateConstruct }
//	Time when entry/feed was last updated.

#echo "atom: [["; var_dump($rss); echo "]]\n";

	// Theoretically, an Atom feed can be just a single item. In
	// practice, we should probably assume it's a feed.

	$name = $rss->getName();
echo "rss name: [[$name]]\n";
	// XXX - Perhaps abort if $name != "feed".

	foreach ($rss->children() as $elt => $value)
	{
#		echo "\tElement [$elt] => [$value]\n";
		echo "\tElement [$elt]\n";
		switch ($elt) {
		    case "author":
			// atomAuthor: author of the feed
			$feed_author = atom_person($value);
			echo "Author: name [$feed_author[name]], url [$feed_author[url]], email [$feed_author[email]]\n";
			break;

		    case "category":
			// atomCategory: category, and optional scheme
			// and label for this feed.
			$attrs = $value->attributes();
			$cat_term = $attrs['term'];
			$cat_scheme = $attrs['scheme'];
			$cat_label = $attrs['label'];
			if (isset($cat_scheme))
				$feed_categories[] = "$cat_term\0$cat_scheme";
			else
				$feed_categories[] = $cat_term;
			echo "Categories: [["; print_r($feed_categories); echo "]]\n";
			break;

		    case "contributor":
			// atomContributor: someone who contributed to
			// this feed.
			// XXX - Not used in practice, apparently
			$feed_contributors[] = atom_person($value);
			echo "Contributors: [["; print_r($feed_contributors); echo "]]\n";
			break;

		    case "generator":
			// atomGenerator: the software that wrote this feed
			$attrs = $value->attributes();
			$feed_gen = $value[0];

			$feed_gen_url = $attrs['uri'];
			$feed_gen_version = $attrs['version'];
			echo "Generator: [$feed_gen]: [$feed_gen_url] v. [$feed_gen_version]\n";
			break;

		    case "icon":
			// atomIcon: an icon that goes with this feed
			$feed_icon = $value[0];
			echo "Feed icon: [$feed_icon]\n";
			break;

		    case "id":
			// atomId: GUID for this feed
			$feed_guid = $value[0];
			echo "Feed GUID: [$feed_guid]\n";
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
echo "link rel [$link_rel], type [$link_type], href [$link_href]\n";
			if ($link_rel == "alternate")
			{
				// Real page for this feed
				$feed_link = $link_href;
				echo "Link alternate: [$feed_link]\n";
			} elseif ($link_rel == "replies" &&
				  $link_type == "text/html")
			{
				$feed_comment_url = $attrs['href'];
				echo "Comments link: [$feed_comment_url]\n";
			} elseif ($link_rel == "replies" &&
				  $link_type == "application/atom+xml")
			{
				$feed_comment_rss = $attrs['href'];
				echo "Comments feed: [$feed_comments_rss]\n";
			}
			break;

		    case "logo":
			// atomLogo: a logo that goes with this feed.
			// XXX - Not sure how it differs from an icon,
			// except that an icon should be small and
			// square, whereas a logo should have an
			// aspect ratio of 2:1.
			$feed_logo = $value[0];
			echo "Feed logo: [$feed_logo]\n";
			break;

		    case "rights":
			// atomRights: copying rights and whatnot.
			// XXX - I don't have an example of this in use.
			break;

		    case "subtitle":
			// atomSubtitle: feed subtitle
			$feed_subtitle = atom_text_construct($value);
			echo "Feed subtitle: [$feed_subtitle]\n";
			break;

		    case "title":
			// atomTitle: feed title
			$feed_title = atom_text_construct($value);
			echo "Feed title: [$feed_title]\n";
			break;

		    case "updated":
			// atomUpdated: when the feed was last updated
			// XXX - Use strtotime() to convert to time_t.
			$feed_lastbuild = $value[0];
			echo "Feed last updated: [$feed_lastbuild]\n";
			break;

		    case "entry":
			// atomEntry: an entry in this feed
			parse_atom_entry($value);
			break;

		    default:
			echo "Unrecognized feed element [$elt]\n";
			break;
		}
	}
}

// parse_atom_entry
// Parse an Atom <entry> tag.
function parse_atom_entry($entry)
{
echo "Inside parse_atom_entry()\n";
	foreach ($entry->children() as $elt => $value)
	{
		echo "\tentry element: [$elt] => [$value]\n";
		switch ($elt) {
		    case "author":
			// atomAuthor: author of this entry
			$entry_author = atom_person($value);
			echo "Entry author: name [$entry_author[name]], url [$entry_author[url]], email [$entry_author[email]]\n";
			break;

		    case "category":
			// atomCategory: a category, and optional
			// scheme and label, for this entry.
			$attrs = $value->attributes();
			$cat_term = $attrs['term'];
			$cat_scheme = $attrs['scheme'];
			$cat_label = $attrs['label'];
			if (isset($cat_scheme))
				$entry_categories[] = "$cat_term\0$cat_scheme";
			else
				$entry_categories[] = $cat_term;
			echo "Categories: [["; print_r($entry_categories); echo "]]\n";
			break;

		    case "content":
			// atomContent: the actual content of the entry.
			$attrs = $value->attributes();
			switch ($attrs['type']) {
			    case "text":
			    case "html":
			    case "xhtml":
				$entry_content = atom_text_construct($value);
				echo "Content (type [$attrs[type]]): [$entry_content]\n";
				break;

			    default:
				echo "Unrecognized content type: [$attrs[type]]\n";
				// XXX - Actually, this could be link
				// to external content, and this is legal.
				// XXX - Find a test case.
				break;
			}
			break;

		    case "contributor":
			// atomContributor
			$entry_contributors[] = atom_person($value);
			echo "Entry contributors: [["; print_r($entry_contributors); echo "]]\n";
			break;

		    case "id":
			// atomId: GUID for this entry
			$entry_guid = $value[0];
			echo "Entry GUID: [$entry_guid]\n";
			break;

		    case "link":
			// atomLink: links to various documents
			// pertaining to this entry.
			$attrs = $value->attributes();

			// There's an infinite number of possible
			// '<link rel's, and we only care about a few
			// of them. Look for those and ignore the
			// others.
			$link_rel = $attrs['rel'];
			$link_type = $attrs['type'];
			$link_href = $attrs['href'];
echo "link rel [$link_rel], type [$link_type], href [$link_href]\n";
			if ($link_rel == "alternate")
			{
				// Real page for this entry
				$entry_link = $link_href;
				echo "Entry link alternate: [$entry_link]\n";
			} elseif ($link_rel == "replies" &&
				  $link_type == "text/html")
			{
				$entry_comment_url = $attrs['href'];
				echo "Entry comments link: [$entry_comment_url]\n";
			} elseif ($link_rel == "replies" &&
				  $link_type == "application/atom+xml")
			{
				$entry_comment_rss = $attrs['href'];
				echo "Entry comments feed: [$entry_comment_rss]\n";
			}
			break;

		    case "published":
			// atomPublished: when the entry was first published.
			// XXX - Use strtotime() to convert to time_t.
			$entry_pubdate = $value[0];
			echo "PubDate: [$entry_pubdate]\n";
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
			$entry_summary = atom_text_construct($value);
			echo "Entry summary: [[$entry_summary]]\n";
			break;

		    case "title":
			// atomTitle: entry title
			$entry_title = atom_text_construct($value);
			echo "Entry title: [$entry_title]\n";
			break;

		    case "updated":
			// atomUpdated: when the entry was last updated
			// XXX - Use strtotime() to convert to time_t.
			$entry_lastupdate = $value[0];
			echo "Entry last updated: [$feed_lastupdate]\n";
			break;

		    default:
			echo "Unrecognized entry element: [$elt]\n";
			break;
		}
	}
}

// atom_text_construct
// Parse an atomTextConstruct node, and return it as a string of HTML
// that can be sent to a browser.
function atom_text_construct($node)
{
echo "Inside atom_text_construct()\n";
	$attrs = $node->attributes();

	switch ($attrs['type']) {
	    case "text":
	    case "":		// No type -> assume text
		// This is a text node. Its value has already been
		// HTML-escaped for the XML feed. So leave it alone.
echo "Text node: text: [$node[0]]\n";
		return $node[0];

	    case "html":
		// This is an HTML node. Its value has had the HTML
		// markup escaped so that it doesn't interfere with
		// the surrounding XML. Unescape one level
echo "Text node: html: [", htmlspecialchars_decode($node[0]), "]\n";
		return htmlspecialchars_decode($node[0]);

	    case "xhtml":
		// This is an XHTML node: it's inside a <div>, but is
		// otherwise fine the way it is. Strip off the <div>.
		// XXX - I don't have any examples of this to test with.
		return (string) $node[0]->children();

	    default:
		echo "Unrecognized atomTextConstruct type: [$attrs[type]]\n";
		break;
	}
}

// atom_person
// Parse an atomPersonConstruct node, and return it as an array with
// keys "name", "url", and "email".
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
