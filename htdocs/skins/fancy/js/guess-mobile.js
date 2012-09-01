#ifndef _mobile_js_
#define _mobile_js_
/* guess-mobile.js
 * Try to guess what kind of mobile device this is, by examining the
 * user-agent string. Yes, this is an ugly hack, and it's better to
 * check for the existence of specific functions.
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
		return "iPad";
	else if (user_agent.match(/Mozilla\/\S+ .*Kindle/))
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
	if (mobile == "iPhone")
		document.write('<meta name="viewport" content="width = device-width, initial-scale=0.5" />');
	else if (mobile == "Android")
		document.write('<meta name="viewport" content="width = device-width, initial-scale=1.0" />');

	// Load a device-specific stylesheet
	document.write('<link rel="stylesheet" type="text/css" href="skins/' +
		       skin_dir +
		       '/' +
		       mobile.toLowerCase()+'.css" media="screen" />');
}

#endif	// _mobile_js_
