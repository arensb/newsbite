/* xhr.js
 * A browser-independent way of creating XMLHttpRequest objects.
 */
#ifndef _xhr_js_
#define _xhr_js_

/* createXMLHttpRequest
 * Create a new XMLHttpRequest object, hopefully in a
 * browser-independent manner.
 */
// I'm not sure why, but Chrome really wants
//	createXMLHttpRequest = function() ...
// rather than
//	function createXMLHttpRequest()
var createXMLHttpRequest = undefined;
if (window.XMLHttpRequest && typeof XMLHttpRequest != "undefined")
{
	/* Firefox, Safari, etc. */
	createXMLHttpRequest = function()
	{
		var request = false;
		try {
			request = new XMLHttpRequest();
		} catch (e) {
			request = false;
		}
		return request;
	}
} else if (window.ActiveXObject) {
	/* IE */
	createXMLHttpRequest = function()
	{
		var request = false;

		/* Create a new ActiveX XMLHTTP object */
		try {
			request = new ActiveXObject('Msxml2.XMLHTTP');
		} catch (e) {
			request = false;
		}
		return request;
	}
} else {
	// Don't define createXMLHttpRequest.
	// Hopefully this will never happen, especially with
	// newer browsers.
}

/* get_json_data
 * Send a request for JSON data.
 * 'url' is the URL from which to fetch the data.
 * 'params' is an object of POST pararameters to send.
 * 'handler' is a function to call when the data has arrived.
 * 'err_handler' is a function to call in case of error.
 * 'batch' is a boolean: if true, wait until all the data has come in to
 * call the handler. Otherwise, call the handler for each line as it
 * comes in.
 */
function get_json_data(url, params, handler, err_handler, batch)
{
	var request = createXMLHttpRequest();
	if (!request)
		return null;
			// XXX - Better error-reporting?

	request.open('POST', url, batch);
	request.setRequestHeader('Content-Type',
		'application/x-www-form-urlencoded');

	var param_string = "";
	for (var p in params)
	{
		if (param_string != "")
			param_string += "&";
		param_string += p + "=" +
			encodeURIComponent(params[p]);
	}

	if (handler)
	{
		request.onreadystatechange =
			function() {
				get_json_callback_batch(request,
							handler,
							err_handler,
							batch);
			};
	}
	request.send(param_string);
		// XXX - Error-checking

	return true;	// Success
}

// XXX - Move get_json_callback_batch() inside get_json_data(). This
// should also help us remember how the request was originally
// submitted.
function get_json_callback_batch(req, user_func, user_err, batch)
{
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

			// Call a user function, if defined.
			if (user_err != null)
				user_err(req.status, req.statusText);
		}
		return;
	    case 3:		// Got partial text
		// XXX - Call handler if !batch
		return;
	    case 4:
		// The response is a JSON object.
		if (req.responseText == "")
			// XXX - No text given. Should have better
			// error-handling.
			return;

		// Use a try{}, in case the server sent bad JSON.
		var value;
console.log("req.status: %d", req.status);
		try {
			value = JSON.parse(req.responseText);
		} catch (e) {
			// XXX - Do something smarter?

			// XXX - When the session times out, this is
			// where things fail, because the server
			// returns HTTP. Probably ought to check the
			// status code; if it's 401, then we need to
			// log back in, then resubmit the AJAX
			// request.
			// Is there any memory anywhere of the URL,
			// parameters, etc. of the original request?
			console.error(req);
			console.error("Can't parse response: %o", e);
			console.log(req.responseText);
			value = undefined;
		}
		user_func(value);
		break;
	    default:
		return;
	}
}

#endif	// _xhr_js_
