<?php
// REST-related classes and such. Inspired by
// http://www.lornajane.net/posts/2012/building-a-restful-php-server-understanding-the-request

// RESTNoMethodException
// Exception thrown when one tries to create a REST request with no
// method (GET, POST, etc.)
class RESTNoMethodException extends Exception {};

class RESTInvalidCommand extends Exception {};

/* RESTReq
 * Main class for a REST request.
 * Typically, you would
 *
 *	$rreq = new RESTReq();
 *	[do things]
 *	$retval = array();
 *	$retval["var1"] = $value1;
 *	$retval["var2"] = $value2;
 *	...
 *	$rreq->finish(200, NULL, $retval);
 */
class RESTReq
{
	protected $method = NULL;
	// $path is the path underneath the root.
	// $classname is the first element of $path, and
	// $subpath is the rest of it.
	// Sometimes $subpath is an identifier, but it could be method
	// and identifier.
	// If the REST root is http://foo.com/w1/ , then
	// http://foo.com/w1/feeds/stats/123 ->
	//	$path == "feeds/stats/123"
	//	$classname == "feeds"
	//	$subpath == "stats/123"
	protected $path = NULL;
	protected $classname = NULL;
	protected $subpath = NULL;
	protected $url_params = array();
	protected $content_type = NULL;
	protected $body= NULL;
	protected $outfmt = "json";	// Desired output format

	function __construct(&$server = NULL, &$body = NULL)
	{
		global $_SERVER;

		// If the server variables weren't specified, use
		// $_SERVER;
		if (!isset($server))
			$server = &$_SERVER;

		// Query method: GET, PUT, POST, etc.
		if (!isset($server['REQUEST_METHOD']))
		{
			// XXX - Abort: we need a method.
			throw new RESTNoMethodException();
		}
		$this->method = $server['REQUEST_METHOD'];

		// Get the path. The first part is the class, and the
		// rest is either a subclass, an identifier, or
		// something.
		$this->path = $server['PATH_INFO'];
		$this->path = preg_replace(',^/,', '', $this->path);
					// Remove leading slash
		list ($this->classname, $this->subpath) =
			// Split up into class and sub-path.
			explode("/", $this->path, 2);
		// XXX - If there's only a class, presumably ought to
		// throw an exception.

		// Parameters passed in through the URL
		if (isset($server['QUERY_STRING']))
			parse_str($server['QUERY_STRING'], $this->url_params);

		if (isset($server['CONTENT_TYPE']))
			$this->content_type = $server['CONTENT_TYPE'];

		// If the body wasn't specified, use stdin.
		// We use this rather than $_POST because if the
		// method wasn't POST, PHP won't parse it for us.
		if (!isset($body))
			$this->body = file_get_contents("php://input");

		// XXX - Parse the body: get the content type, and
		// parse it as JSON, XML, YAML, or whatever.
		// json_decode(): http://php.net/manual/en/function.json-decode.php
		switch ($this->content_type)
		{
		    case "text/json":
			$this->body = json_decode($this->body);
			break;
		    default:
			// Leave it alone. Maybe a handler class knows
			// what to do with it.
			break;
		}

		// XXX - Authenticate the client.
		// Authorization should probably happen at the class
		// level.
		// Parse the "newsbite_user" cookie, if there is one.

		// Find out what kind of output the client wants:
		$outfmt = $this->url_param("o");
		if (isset($outfmt))
		{
			switch ($outfmt)
			{
			    case "json":
			    //case "yaml":	// XXX - Maybe add this later
				$this->outfmt = $outfmt;
				break;

			    case "xml":
				require_once("lib/xml-output.inc");
				$this->outfmt = $outfmt;
				break;

			    default:
				$this->finish(406, "Unknown output format.");
			}
		}

		// XXX
	}

	// XXX - dispatch(), to decide where the request should go:
	// which function should handle this request? Notionally, I
	// guess it should just look up a table with
	// { method, class+subclass, ID } -> function
	//
	// Let's leave this until a bit later, when we have a better
	// idea of what makes sense.

	// Accessors
	function method() {
		return $this->method;
	}

	function path() {
		return $this->path;
	}

	function classname() {
		return $this->classname;
	}

	function subpath() {
		return $this->subpath;
	}

	function url_param($name)
	{
		if (array_key_exists($name, $this->url_params))
			return $this->url_params[$name];
		else
			return NULL;
	}

	function content_type() {
		return $this->content_type;
	}

	// XXX - Should this return the parsed version of the body?
	function body() {
		return $this->body;
	}

	// print_struct
	// Print a data structure in the desired output format.
	function print_struct(&$val)
	{
		switch ($this->outfmt)
		{
		    case "json":
			header("Content-type: text/json; charset=utf-8");
			echo json_encode($val);
			break;

		    case "xml":
			header("Content-type: text/xml; charset=utf-8");
			// XXX - Ought to give a name to the XML container.
			// Perhaps this can be the class name.
			echo xmlify($val, $this->classname());
			break;

	    	//    case "yaml":
		//	header("Content-type: text/yaml; charset=utf-8");
		//	echo yaml_emit($val);
		//	break;

		    default:
			error_log("Unknown output format in print_struct: $out_fmt");
			// Default to JSON.
			echo jsonify($val);
			break;
		}
	}

	// finish - Finish the REST request
	// Set the HTTP status and error message; return a data
	// structure to the caller; and exit.
	//
	// By default, this
	function finish($status = 200, $msg = NULL, $retval = NULL)
	{
		http_response_code($status);
		if (isset($msg) && $msg != "")
			header("X-Newsbite-Error: " . $msg);

		if (isset($retval))
			$this->print_struct($retval);
		exit(0);
	}
}

$rreq = new RESTReq();
$retval = array();
$retval["method"] = $rreq->method();
$retval["path"] = $rreq->path();
$retval["class"] = $rreq->classname();
$retval["subpath"] = $rreq->subpath();
$retval["outfmt"] = $rreq->url_param("o");
$retval["content_type"] = $rreq->content_type();
$retval["body"] = $rreq->body();
// XXX - Should there be a method for getting all the URL parameters?

// XXX - If the class is "login", we don't require authentication. But
// the client needs to present credentials.

// XXX - Check authentication.

// XXX - Figure out where to send the request
switch ($rreq->classname())
{
    case "test":
	try {
		$err = require_once("rest_test.inc");
		$retval = test_stuff($rreq);
	} catch (Exception $e) {
		// echo "Caught exception ", print_r($e, true);
		$rreq->finish(400, "Caught an exception");
	}
	// XXX
	break;
    case "feed":
	// XXX
	break;
    case "group":
	// XXX
	break;
    case "article":
	// XXX
	break;
    default:
	// XXX - Die with an error?
	break;
}

// XXX - HTTP response. Usually 200, but we might need to send 4xx or
// even 5xx.

// XXX - Send the return value, in the format the user wants.

$rreq->finish(200, NULL, $retval);
?>
