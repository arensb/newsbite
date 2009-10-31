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

window.onload = init;

var feed_list;			// List of feeds and their properties
var pages = new Array();	// The different pages we could be displaying
var orientation;		// iPhone orientation

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

	debug("initial orientation: "+window.orientation);
	if (window.navigator.standalone)
		debug("Standalone mode");
	else
		debug("Not standalone mode");

	// Write to the Safari developer console
	//console.log("Hello log world");
	//console.warn("Hello warning world");
	//console.error("Hello error world");
	//console.info("Hello info world");

	get_feeds();
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

/* get_feeds
 */
function get_feeds()
{
	var request = createXMLHttpRequest();
	if (!request)
	{
		// XXX - Error-reporting
		return;
	}

	request.open('POST',
		     'feeds.php?o=json',
		     false);	// Don't call me until you have all the text
	request.setRequestHeader('Content-Type',
		'application/x-www-form-urlencoded');
	request.onreadystatechange = function() { get_feeds_callback(request) };

	// XXX - Should put up a spinner or something to indicate that a
	// net request has gone out.
	request.send('');
}

/* get_feeds_callback
 * Handle the state changes for the request issued by get_feeds().
 */
function get_feeds_callback(req)
{
	var feed_items;

//debug("Inside get_feeds_callback("+req.readyState+")");
	switch (req.readyState)
	{
	    case 0:		// Uninitialized
	    case 1:		// Loading
		return;
	    case 2:		// Loaded
		// XXX
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
	    case 4:		// Got all text
		// XXX

		/* The response is a JSON object wrapped inside an XML
		 * CDATA chunk (see feeds.php): the first line is the
		 * xml header; the second is the start of the CDATA
		 * block; the third is the data we're interested in;
		 * the fourth closes the CDATA block.
		 *
		 * So here we split the response text into lines, take
		 * the third, and eval it as JavaScript code.
		 */
		try {
			eval("feed_items = " + req.responseText.split("\n")[2]);
		} catch (e) {
			// XXX - Error-reporting
debug("Something went wrong: "+e);
			return;
		}

//		feed_list.innerHTML = feed_items.length + " lists";
		break;
	}

	/* If we get this far, feed_items is a list of objects describing
	 * feeds.
	 */
	// XXX - Sort the items.
	for (var i in feed_items)
	{
var j = i;
		var li = document.createElement("li");
		var item = feed_items[i];
		li.innerHTML = item['id']+": "+item['title'];
		if (i % 2)
		{
			li.setAttribute("class", "item even-item");
		} else {
			li.setAttribute("class", "item odd-item");
		}

		/* Okay, this is convoluted, but it's necessary
		 * because of JavaScript's closures, to make sure we
		 * don't wind up with a hundred callbacks all
		 * referring to the same value of 'item'. Basically,
		 * we need to create a new local variable in each
		 * iteratin of the loop, which is why we have the
		 * outer anonymous function.
		 */
		li.onclick = (function(id) {
			return function() { show_feed(id) }
		})(item['id']);
		feed_list.appendChild(li);
	}
}

function show_feed(id)
{
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
