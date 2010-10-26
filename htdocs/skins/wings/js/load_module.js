/* load_module.js
 * Allow loading modules after the main page has loaded.
 */
#ifndef _load_module_js_
#define _load_module_js_

/* load_module
 * Load the JS file specified by the url. When it's loaded, call the
 * callback function.
 */
function load_module(url, callback)
{
	var module = document.createElement("script");
	module.type = "text/javascript";
	module.src = url;
	module.onload = callback;
	document.getElementsByTagName("head")[0].appendChild(module);
}

//load_module("foo.js",
//	    function() {
//		    alert("Inside load_module callback");
//		    foo();
//	    });

#endif	// _load_module_js_
