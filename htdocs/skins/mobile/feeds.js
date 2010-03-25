/* XXX - Need a wrapper for XMLHttpRequests: take a callback function
 * as argument. Set up the request, with xml_callback() as its
 * onstatechange callback. xml_callback(), in turn, collects the text,
 * strips off the XML and CDATA crap, and calls the user-supplied
 * callback function.
 *
 * Perhaps ought to have two versions: one that grabs the entire chunk
 * once it's all loaded, and one that calls the user callback function
 * with each line as it comes in.
 */

/* XXX - Need a mechanism for replacing variables in templates. Make
 * sure that external values (e.g., from a feed) can't cause problems.
 *

 * Probably the best way to do this is to search for /@(\w+)@/g in the
 * template, collect the results, and just use the offsets of each
 * match to build the result. The template comes from a clean source,
 * and the dirty data isn't matched against a regexp.
 */

var debug_window = undefined;
var browser_type = undefined;

window.onload = init;

var feed_list;			// List of feeds and their properties
var pages = new Array();	// The different pages we could be displaying
var orientation;		// iPhone orientation

var feed_tmpl = '<h1>@title@</h1>\
<img src="@image@"/>\
<button onclick="flip_to_feed_index()">Back to feed index</button>\
<hr/>\
<ol id="items">@+items@</ol>\
<hr/>\
';

var feed_item_tmpl = '<li>@title@</li>';

var last_time = null;
function debug(str)
{
/*return;*/
	if (debug_window == undefined)
		debug_window = document.getElementById("debug");
	if (debug_window == null)
		return;

	var t = new Date().getTime();
//	var delta = (last_time == null ? "" : t - last_time);
//	last_time = t;
//	debug_window.innerHTML += t + "(" + delta + "): " +
	debug_window.innerHTML += t + ": " +
		str + "<br/>\n";
}

function clrdebug()
{
	if (debug_window == null)
		return;
	debug_window.innerHTML = "";
}

// --- Template class ----------------------------------------
function Template(tmpl)
{
	// Look for sequences of the form @VAR@. These are the variables
	// that will be replaced in the template.
	// split() is perfect for this, since a template is by definition
	// a series of strings punctuated by variables to be expanded.
	// Thus we know that the even-numbered elements (0, 2, 4, ...) are
	// strings, and the odd-numbered ones are names of variables.
	this.template = tmpl.split(/@(\w+)@/);
}

// Template.expand
// Takes an array of values (as an object) and expands the template.
// Returns the result.
Template.prototype.expand = function(values)
{
	var retval = "";
	var n = this.template.length;

	for (var i = 0; i < n; i += 2)
	{
		// We're looking at a plain string. Append it to retval
		retval += this.template[i];

		// See if this string is followed by a variable name.
		// If so, expand it.
		if (i+1 < n)
		{
			retval += values[this.template[i+1]];
		}
	}

	return retval;
}
//--- end Template class ----------------------------------------

function init()
{
	window.onorientationchange = orientation_change;
	feed_list = document.getElementById("feeds");

	// Find all of the pages in the document
	var divs = document.getElementsByTagName("div");
	for (d in divs)
	{
		var div = divs[d];

		if (div.className != "multi-page")
			continue;
		pages.push(div);
	}

//	debug("initial orientation: "+window.orientation);
//	if (window.navigator.standalone)
//		debug("Standalone mode");
//	else
//		debug("Not standalone mode");

	// Write to the Safari developer console
	// XXX - Safari has 'console'. Firefox doesn't.
	//console.log("Hello log world");
	//console.warn("Hello warning world");
	//console.error("Hello error world");
	//console.info("Hello info world");

	get_feeds();
}

/* find_page
 * Look through the list of pages ('pages') and find the one whose
 * identifier matches the given title.
 */
function find_page(title)
{
	for (p in pages)
	{
		if (pages[p].id == title)
			return pages[p];	// Found it
	}
	return null;		// Not found
}

// XXX - This function shouldn't be replicated. Consolidate.
/* createXMLHttpRequest
 * Create a new XMLHttpRequest object, hopefully in a
 * browser-independent manner.
 */
function createXMLHttpRequest()
{
	var request = false;

	/* Firefox, Safari, etc. */
	if (window.XMLHttpRequest)
	{
		if (typeof XMLHttpRequest != 'undefined')
		{
			try {
				request = new XMLHttpRequest();
			} catch (e) {
				request = false;
				debug("Error allocating new XMLHttpRequest\n");
			}
		}
	} else if (window.ActiveXObject)
	{
		/* IE */
		/* Create a new ActiveX XMLHTTP object */
		try {
			request = new ActiveXObject('Msxml2.XMLHTTP');
		} catch (e) {
			request = false;
			debug("Error allocating ActiveX XMLHTTP\n");
		}
	}
	return request;
}

/* get_json_data
 * Send a request for JSON data.
 * 'url' is the URL from which to fetch the data.
 * 'params' is an array of POST pararameters to send.
 * 'handler' is a function to call when the data has arrived.
 * 'batch' is a boolean: if true, wait until all the data has come in to
 * call the handler. Otherwise, call the handler for each line as it
 * comes in.
 */
function get_json_data(url, params, handler, batch)
{
	var request = createXMLHttpRequest();
	if (!request)
		return null;
			// XXX - Better error-reporting?

	request.open('POST', url, batch);
	request.setRequestHeader('Content-Type',
		'application/x-www-form-urlencoded');

	var param_string = "";
	for (p in params)
	{
		if (param_string != "")
			param_string += "&";
		param_string += p + "=" +
			encodeURIComponent(params[p]);
	}

	if (handler)
	{
		// XXX - Need separate case for calling handler as
		// data comes in. Or maybe pass the 'batch' argument
		// to get_json_callback_batch.
		request.onreadystatechange =
			function() {
//debug("In the other handler");
				get_json_callback_batch(request, handler);
			};
	}
	request.send(param_string);
		// XXX - Error-checking

	return true;	// Success
}

function get_json_callback_batch(req, user_func)
{
//debug("Inside get_json_callback_batch");
//debug("Inside get_json_callback_batch("+req.readyState+")");
	switch (req.readyState)
	{
	    case 0:		// Uninitialized
	    case 1:		// Loading
		return;
	    case 2:		// Loaded
		// XXX - Do something intelligent in case of error
		var err;
		var errmsg;

		/* Get HTTP status */
		try {
			err = req.status;
			errmsg = req.statusText;
		} catch (e) {
			err = 1;
		}

		/* If the HTTP status isn't 200, abort the request */
		if (err != 200)
		{
			req.abort();
			req.aborted = true;
		}
		return;
	    case 3:		// Got partial text
		return;
	    case 4:
		/* The response is a JSON object wrapped inside an XML
		 * CDATA chunk (see feeds.php): the first line is the
		 * xml header; the second is the start of the CDATA
		 * block; the third is the data we're interested in;
		 * the fourth closes the CDATA block.
		 */
		if (req.responseText == "")
			// XXX - No text given. Should have better
			// error-handling.
			return;

		var off1, off2;

		// Find first newline
		off1 = req.responseText.indexOf("\n");
		if (off1 < 0)
			return;
		// Find second newline
		off1 = req.responseText.indexOf("\n", off1+1);

		// Find last newline (other than the one at the end
		off2 = req.responseText.lastIndexOf("\n",
						    req.responseText.length-2);
		var substr = req.responseText.slice(off1, off2);
		user_func(substr);
		break;
	    default:
		return;
	}
}

/* get_feeds
 */
function get_feeds()
{
//debug("Inside get_feeds()");
	get_json_data("feeds.php",
		      {o:	"json",
		      },
		      get_feeds_callback,
		      true);
	return;
}

function get_feeds_callback(jstr)
{
//debug("get_feeds_callback("+jstr+")");
debug("Inside get_feeds_callback()");

	var feeds;

	// Get the feed description from the data returned by the server
	try {
//debug("About to eval ["+jstr+"]");
		eval("feeds = "+jstr);
	} catch (e) {
		console.error("Caught error " + e);
		return;
	}
//debug("first feed: ["+feeds+"]");

	var index_page = find_page("index-page");
	var index_text = "";

	for (var i = 0; i < feeds.length; i++)
	{
debug("Adding feed "+i);
		index_text += feed_index_tmpl.expand(feeds[i]);
	}
	feed_list.innerHTML = index_text;
}

function show_feed(id)
{
debug("Inside show_feed()");
//	flip_to_page("feed-page");
	get_json_data("view.php",
		      {id: id,
		       o: "json",
		      },
		      show_feed_callback,
		      true);
}

function show_feed_callback(jstr)
{
debug("Inside show_feed_callback()");
	var feed;		// Structure describing the feed

	// Get the feed description from the data returned by the server
	try {
debug("About to eval ["+jstr+"]");
		eval("feed = "+jstr);
	} catch (e) {
		console.error("Caught error " + e);
		return;
	}

	var feed_page = find_page("feed-page");

	/* Create the page from the feed_tmpl template: look for
	 * substrings of the form "@foo@" in feed_tmpl, and replace
	 * them with feed[foo].
	 */
	var tmpl = feed_tmpl;
	var kwpat = /@([^@]+)@/g;	// Regexp for finding keywords
	var result;			// Result of regexp match
	var last_index = 0;		// How much of the template have
					// we seen?
	var rendered = "";		// The rendered page
	while ((result = kwpat.exec(tmpl)) != null)
	{
		rendered += tmpl.slice(last_index, result.index);
		if (result[1] == "+items")
		{
//debug("feed.items: ["+feed['items']+"] ("+feed.items.length+")");
			for (var i = 0; i < feed.items.length; i++)
//			for (i in feed.items)
			{
				/* Generate a list of items from the
				 * feed_item_tmpl template, using the same
				 * mechanism as above.
				 */
				var item = feed.items[i];
//debug("item "+i+": ["+item+"]: ["+item.title+"]");
				var tmpl2 = feed_item_tmpl;
				var kwpat2 = /@([^@]+)@/g;
				var result2;
				var last_index2 = 0;
				var rendered2 = "";
//var n = 0;
//debug("tmpl2: ["+tmpl2+"]");
//result2 = kwpat2.exec(tmpl2);
//debug("result2: ["+result2+"]");
				while ((result2 = kwpat2.exec(tmpl2)) != null)
				{
//debug("result2: ["+result2+"]");
					rendered2 += tmpl2.slice(last_index2, result2.index);
					rendered2 += item[result2[1]];
					last_index2 = result2.index+result2[0].length;
				}
				rendered2 += tmpl2.slice(last_index2);
//debug("rendered2 ["+rendered2+"]");
				rendered += rendered2;
			}
		} else if (feed[result[1]] == undefined)
			rendered += "--dunno--";
		else
			rendered += feed[result[1]];
		last_index = result.index+result[0].length;
	}
	rendered += tmpl.slice(last_index);
	feed_page.innerHTML = rendered;
	flip_to_page("feed-page");
}

function flip_to_page(the_page)
{
	for (i in pages)
	{
		var p = pages[i];
		if (p.id == the_page)
		{
			p.style.zIndex = "100";
			p.style.display = "block";
		} else {
			p.style.zIndex = "0";
			p.style.display = "none";
		}
	}
}

function flip_to_feed_index()
{
	flip_to_page("index-page");
}

function flip_to_settings_page()
{
	flip_to_page("settings-page");
}

/* orientation_change
 * Called when the orientation of the iPhone/iPod Touch changes.
 * -90: landscape mode, with the top to the right
 * 0: portrait mode, with the top at the top
 * 90: landscape mode, with the top to the left
 */
function orientation_change()
{
	orientation = window.orientation;
}

function showfeed(id)
{
	alert("I ought to show you feed "+id);
}

/* ---------------------------------------------------------------------- */
/* Try to guess the browser type.
 * This should be run once, when the file is loaded.
 */
if (browser_type == undefined)
{
	/* XXX - Really ought to check for various features independently.
	 * http://developer.apple.com/internet/webcontent/objectdetection.html
	 */
//alert("userAgent ["+navigator.userAgent+"]");
	if (navigator.userAgent.indexOf("AppleWebKit") != -1)
	{
//alert("found safari");
		browser_type = "safari";
	} else if (navigator.userAgent.indexOf("Firefox") != -1)
	{
//alert("found firefox");
		browser_type = "firefox";
	}
//else {alert("found other");}
}