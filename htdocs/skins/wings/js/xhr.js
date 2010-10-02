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

		// Find last newline (other than the one at the end)
		off2 = req.responseText.lastIndexOf("\n",
						    req.responseText.length-2);
		var substr = req.responseText.slice(off1, off2);
		user_func(substr);
		break;
	    default:
		return;
	}
}

#endif	// _xhr_js_
