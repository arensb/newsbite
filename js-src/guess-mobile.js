#ifndef _mobile_js_
#define _mobile_js_
/* guess-mobile.js
 * Try to guess what kind of mobile device this is, by examining the
 * user-agent string. Yes, this is an ugly hack, and it's better to
 * check for the existence of specific functions.
 *
 * navigator.platform also gives the hardware/OS that the browser is
 * running on, though this doesn't seem terribly useful.
 * More generally, see
 *	http://www.quirksmode.org/js/detect.html
 *	http://www.w3schools.com/jsref/obj_navigator.asp
 * for detecting the browser.
 */

// In my defense, this is mostly used to load an appropriate
// stylesheet, and in a few other cases where we want one look for the
// desktop and another for all mobile devices; rather than
// distinguishing one mobile device from another.
var mobile = function(){
	var user_agent = "";
	try {
		user_agent = navigator.userAgent;
	} catch (e) {
		// Leave it as empty string
	}

	if (user_agent.match(/Mozilla\/\S+ \(iPod;/))
		return "iPhone";
	else if (user_agent.match(/Mozilla\/\S+ \(iPad;/))
	{
// XXX - For some reason, on iPad, I get desktop background.
// Here's what I get, in case it's an agent problem:
// Firefox on iPad:
// 56.40.255.16 - - [18/Nov/2019:11:16:13 -0800] "GET /newsbite/newsbite.manifest HTTP/1.1" 304 3783 "https://www.ooblick.com/newsbite/view.php" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15) AppleWebKit/605.1.15 (KHTML, like Gecko)"
// Safari on iPad:
// 156.40.255.16 - - [18/Nov/2019:11:23:02 -0800] "GET /newsbite/newsbite.manifest HTTP/1.1" 304 3783 "https://www.ooblick.com/newsbite/view.php" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.0.1 Safari/605.1.15"
// Firefox on iMac:
// 130.14.12.225 - - [18/Nov/2019:11:27:01 -0800] "GET /newsbite/newsbite.manifest HTTP/1.1" 304 3978 "https://www.ooblick.com/newsbite/" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.14; rv:60.0) Gecko/20100101 Firefox/60.0"
	    // alert("This is an iPad")
		return "iPad";
	} else if (user_agent.match(/Mozilla\/\S+ .*Kindle/))
		// This needs to go above the Android line, because
		// the user-agent string is
		// Mozilla/5.0 (X11; U; Linux armv7l like Android; en-us) AppleWebKit/531.2+ (KHTML, like Gecko) Version/5.0 Safari/533.2+ Kindle/3.0+
		// so it matches /^Mozilla.*Android.*Kindle/.
		return "Kindle";
	else if (user_agent.match(/Mozilla\/\S+ .*Android/))
		return "Android";
	return false;
}();

if (mobile)
{
	// Set the viewport, if necessary.
	// XXX - Should there be a default viewport for iPad?
	if (mobile == "iPhone")
	{
		// Define a viewport:
		// <meta name="viewport" content="width=device-width, initial-scale=0.5" />
		var meta_node = document.createElement("meta");

		meta_node.name = "viewport";
		meta_node.content = "width=device-width, initial-scale=0.5";
		document.head.appendChild(meta_node);
	} else if (mobile == "iPad")
	{
		// Define a viewport:
		// <meta name="viewport" content="width=device-width, initial-scale=1.0" />
		var meta_node = document.createElement("meta");

		meta_node.name = "viewport";
		meta_node.content = "width=device-width, initial-scale=1.0";
		document.head.appendChild(meta_node);
	} else if (mobile == "Android")
	{
		// Define a viewport:
		// <meta name="viewport"
		//	content="width=device-width, initial-scale=1.0" />
		var meta_node = document.createElement("meta");

		meta_node.name = "viewport";
		meta_node.content = "width=device-width, initial-scale=1.0";
		document.head.appendChild(meta_node);
	}

	// Load a device-specific stylesheet:
	// <link rel="stylesheet"
	//	type="text/css"
	//	href="path/to/mobile.css"
	//	media="screen" />
	var link_node = document.createElement("link");

	link_node.rel = "stylesheet";
	link_node.type = "text/css";
	link_node.href = "css/" + mobile.toLowerCase() + ".css";
	link_node.media = "screen";
	document.head.appendChild(link_node);
} else {	// !mobile -> desktop
	// Load a device-specific stylesheet:
	// <link rel="stylesheet"
	//	type="text/css"
	//	href="path/to/mobile.css"
	//	media="screen" />
	var link_node = document.createElement("link");

	// XXX - Could also have a browser-specific stylesheet.
	link_node.rel = "stylesheet";
	link_node.type = "text/css";
	link_node.href = "css/desktop.css";
	link_node.media = "screen";
	document.head.appendChild(link_node);
}

#endif	// _mobile_js_
