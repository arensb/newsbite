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
var createXMLHttpRequest = function()
{
	var request = false;
	try {
		request = new XMLHttpRequest();
	} catch (e) {
		console.error("Error creating XMLHttpRequest: "+e);
		request = false;
	}
	return request;
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
	var last_off = 0;	// Last offset we've seen, for get_json_callback_nonbatch().

	/* Inner helper functions */

	/* get_json_callback_batch
	 * XMLHttpRequest callback for batch mode.
	 */
	function get_json_callback_batch()
	{
		switch (request.readyState)
		{
		    case 0:		// Uninitialized
		    case 1:		// Loading
			return;
		    case 2:		// Loaded
			// XXX - Do something intelligent in case of error
			var err;
			var errmsg;

			/* Get HTTP status */
			// XXX - Is this try/catch even necessary?
			// AFAICT it comes from the earliest revision
			// of this function, when I may have been
			// overly paranoid.
			try {
				err = request.status;
				errmsg = request.statusText;
			} catch (e) {
				err = 1;
msg_add("I caught a weird error: "+e);
			}

			/* If the HTTP status isn't 200, abort the request */
			if (err != 200)
			{
console.log("JSON "+url+" failed, status "+request.status+": "+request.statusText);
console.trace();
msg_add("JSON "+url+" failed, status "+request.status+": "+request.statusText);
				request.abort();
				request.aborted = true;

				// Call a user function, if defined.
				// XXX - If the status is 401 (not
				// logged in), then ought to log in
				// through login.php, then resubmit
				// the original request.
				if (err_handler != null)
					err_handler(request.status,
						    request.statusText);
			}
			return;
		    case 3:		// Got partial text
			// XXX - Call handler if !batch
			return;
		    case 4:
			// The response is a JSON object.
			if (request.responseText == "")
				// XXX - No text given. Should have better
				// error-handling.
				return;

			// Use a try{}, in case the server sent bad JSON.
			var value;
			try {
				value = JSON.parse(request.responseText);
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
				console.error(request);
				console.error("Can't parse response: "+e);
msg_add("get_json_data: Can't parse response");
				console.log(request.responseText);
				value = undefined;
			}
			handler(value);
			break;
		    default:
			return;
		}
	}

	/* get_json_callback_nonbatch
	 * XMLHttpRequest callback function for non-batch mode: the
	 * response is a series of JSON objects, separated by
	 * newlines.
	 * As each one comes in, we parse it and call the user's handler.
	 */
	function get_json_callback_nonbatch()
	{
		switch (request.readyState)
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
				err = request.status;
				errmsg = request.statusText;
			} catch (e) {
				err = 1;
			}

			/* If the HTTP status isn't 200, abort the request */
			if (err != 200)
			{
console.log("JSON "+url+" failed, status "+request.status+": "+request.statusText);
console.trace();
msg_add("JSON "+url+" failed, status "+request.status+": "+request.statusText);
				request.abort();
				request.aborted = true;

				// Call a user function, if defined.
				// XXX - If the status is 401 (not
				// logged in), then ought to log in
				// through login.php, then resubmit
				// the original request.
				if (err_handler != null)
					err_handler(request.status,
						    request.statusText);
			}
			return;
		    case 3:		// Got partial text
		    case 4:
			/* Get text from where we stopped last time to the end
			 * of what we've got now.
			 */
			var str = request.responseText.substr(last_off);

			/* Remember how much of the string we've gotten so far */
			last_off = request.responseText.length;

			/* Split the current input into lines */
			var lines = str.split("\n");
			for (var i = 0; i < lines.length; i++)
			{
				var l;
				var line = lines[i];

				if (line.length == 0)
					// Got a blank line.
					continue;
				try {
					// Inside a try{} in case the server sent
					// bad JSON.
					l = JSON.parse(line);
				} catch (e) {
					console.error("Can't parse JSON: "+e+
						      ", offending string: "+
						      line);
msg_add("Can't parse JSON line");
					// If this isn't a complete line, put it
					// back for later. Yeah, this is a bit of
					// a hack.
					if (i == lines.length-1)
						last_off -= line.length;
					continue;
				}

				handler(l);
					// XXX - If there are multiple
					// lines in this pass, should
					// we pass them all to the
					// handler at once, rather
					// than calling the handler
					// multiple times?
			}
			break;
		    default:
			return;
		}
	}

	/* get_json_data() main */

	var request = createXMLHttpRequest();
	if (!request)
	{
msg_add("get_json_data: can't createXMLHttpRequest: "+request);
		return null;
	}

	request.open('POST', url);
	request.setRequestHeader('Content-Type',
		'application/x-www-form-urlencoded');

	var param_string = "o=json";
	for (var p in params)
	{
		if (param_string != "")
			param_string += "&";
		param_string += p + "=" +
			encodeURIComponent(params[p]);
	}

	if (handler)
	{
		request.onreadystatechange = batch ?
			get_json_callback_batch :
			get_json_callback_nonbatch;
	}
	request.send(param_string);
		// XXX - Error-checking

	return true;	// Success
}

#endif	// _xhr_js_
