/* debug.js
 * Debugging functions.
 */
#ifndef _debug_js_
#define _debug_js_

var debug_window = undefined;
var mark_read = {};		// Hash of item_id -> is_read? values
var mark_request = null;	// Data for marking items as read/unread

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

#endif	// _debug_js_
