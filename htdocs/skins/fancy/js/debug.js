/* debug.js
 * Debugging functions.
 */
#ifndef _debug_js_
#define _debug_js_

if (window.console == undefined)
{
	window.console = {
		debug:	function(str) {},
		info:	function(str) {},
		warn:	function(str) {},
		error:	function(str) {},
		dir:	function(thing) {},
	};
}

#endif	// _debug_js_
