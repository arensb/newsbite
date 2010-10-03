/* xhr.js
 * A browser-independent way of creating XMLHttpRequest objects.
 */
#ifndef _xhr_js_
#define _xhr_js_

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
		}
	}
	return request;
}

/* get_json_data
 * Send a request for JSON data.
 * 'url' is the URL from which to fetch the data.
 * 'params' is an object of POST pararameters to send.
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
		request.onreadystatechange =
			function() {
				get_json_callback_batch(request, handler, batch);
			};
	}
	request.send(param_string);
		// XXX - Error-checking

	return true;	// Success
}

function get_json_callback_batch(req, user_func, batch)
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

		var value = JSON.parse(req.responseText);
		user_func(value);
		break;
	    default:
		return;
	}
}

#endif	// _xhr_js_
