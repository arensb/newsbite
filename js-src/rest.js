/* rest.js
 * REST functions.
 */
#ifndef _rest_js_
#define _rest_js_

#include "config.js"	// Get local configuration Config {}.

/* REST
 */
var REST = {
	base: Config.REST_prefix+"/w1",
}

/* rest_call
 * Make a REST call.
 * 'verb' is the HTTP verb, e.g., "GET", "POST", etc.
 * 'path' is the subdirectory underneath the base.
 * 'params' is a data structure of parameters to pass along, or undefined
 *	if none.
 * 'handler' is a function that will be called when the call finishes and
 *	data is returned.
 * 'err_handler' is a function that will be called in case of error.
 */
REST.call = function(verb, path, params, handler, err_handler)
{
console.debug("Inside REST.call "+verb+" "+path, params, handler, err_handler);
	var request;

	/* login_retry
	 * If a REST call fails because we're not logged in, log in
	 * and try again.
	 */
	function login_retry()
	{
		// Fetch login.php. The "o=json" bit is really to make
		// it work with a RESTful request.
		REST.call("GET", "login?o=json", null,
			  function(err, errmsg, value) {
				  // XXX - Error-checking.

				  // Retry the call
				  return REST.call(verb, path, params, handler, err_handler);
			  },
			  function(err, errmsg) {
				  console.error("Failed to log in: "+err+": "+errmsg);
			  });
	}

	function rest_call_callback()
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
			err = request.status;
			errmsg = request.statusText;

			/* If the HTTP status is 401, that means the
			 * request failed because you're not logged
			 * in. So log in and try again.
			 */
			if (err == 401)
			{
				// Abort the current request.
				request.abort();
				request.aborted = true;

				login_retry();
			}
//			/* If the HTTP status isn't 200, abort the request */
//			if (err != 200)
//			{
//console.log("REST "+verb+" "+path+" failed, status "+request.status+": "+request.statusText);
//console.trace();
//msg_add("REST "+verb+" "+path+" failed, status "+request.status+": "+request.statusText);
//				request.abort();
//				request.aborted = true;
//
//				// Call a user function, if defined.
//
//				// XXX - Does a non-200 HTTP request
//				// count as an error in the REST
//				// world?
//			}
			return;
		    case 4:
			var err;
			var errmsg;

			/* Get HTTP status */
			err = request.status;
			errmsg = request.statusText;

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

				// XXX - When the session times out,
				// this is where things fail, because
				// the server returns HTTP. Probably
				// ought to check the status code. If
				// it's a timeout, retry the original
				// request.
				console.error(request);
				console.error("Can't parse response: "+e);
msg_add("REST.call: Can't parse response");
				console.log(request.responseText);
				value = undefined;
			}
			handler(err, errmsg, value);
			break;
		    default:
			return;
		}
	}

	/* rest_call main */
	// XXX - Check verb for sanity: should be one of
	// GET, POST, PUT, DELETE, PATCH
	request = new XMLHttpRequest();
	if (!request)
	{
msg_add("rest_call: can't create XMLHttpRequest: "+request);
		return null;
	}

	var url = REST.base + "/" + path;
	request.open(verb, url);
	request.setRequestHeader('Content-Type', 'application/json');
	request.timeout = 10000;		// Timeout, in ms.

	// Construct body
	var body = null;
	if (params != undefined)
		body = JSON.stringify(params);

	request.ontimeout = function(e) {
		console.debug("Request timed out.");
		console.debug(request);

		// Give the caller a chance to deal with this.
		if (err_handler != null)
			err_handler(null, "Request timed out.");
	};

	if (handler)
		request.onreadystatechange = rest_call_callback;

	if (verb == "GET" && body != null && body != "")
		console.error("GET "+verb+" "+request+" has a body. It won't be sent:", body);
	request.send(body);
}

#endif	// _rest_js_
