/* status-msg.js
 * Module for displaying status messages.
 */
#ifndef _status_msg_js_
#define _status_msg_js_

document.addEventListener("DOMContentLoaded", msg_init, false);

var msg_box;		// Box where status messages go.
function msg_init()
{
	// Initialize status messages
	msg_box = document.createElement("ul");
	msg_box.setAttribute("class", "msglist");
	msg_box.setAttribute("id", "msglist");
	var body = document.getElementsByTagName("body")[0];
	body.appendChild(msg_box);
}

/* msg_add
 * Display a status message that will be displayed for 'duration'
 * milliseconds.
 */
function msg_add(msg, duration)
{
	if (msg_box == undefined)
		// msg_init() apparently hasn't run yet. Just abort.
		return;

	if (!duration)
		// XXX - Perhaps multiply the length of the message by
		// some constant?
		duration = 5000;
	var item = document.createElement("li");
	item.innerHTML = msg;

	// If the user clicks on the message, make it go away.
	item.addEventListener("click", function() {
				msg_expire(item);
			      }, false);

	// Set an alarm to remove the message after a certain timeout,
	// and remember where we put that alarm, so we can remove it
	// later.
	item.expire = setTimeout(function() {
			msg_expire(item);
		}, duration);
	msg_box.appendChild(item);
}

function msg_expire(msg)
{
	var parent = msg.parentNode;

	// Cancel the timed expiration we'd set earlier, in case we
	// got here by the user clicking on the message.
	if (msg.expire != null)
		clearTimeout(msg.expire);
	parent.removeChild(msg)
}

#endif	// _status_msg_js_
