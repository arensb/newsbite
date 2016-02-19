<?php
// XXX - See
// http://www.lornajane.net/posts/2012/building-a-restful-php-server-understanding-the-request
// for ideas

// RESTNoMethodException
// Exception thrown when one tries to create a REST request with no
// method (GET, POST, etc.)
class RESTNoMethodException extends Exception {};

class RESTReq
{
	protected $method = NULL;
	protected $classname = NULL;
	protected $resource = NULL;
	protected $url_params = array();
	protected $content_type = NULL;
	protected $outfmt = "json";	// Desired output format

	function __construct(&$server = NULL, &$body = NULL)
	{
		global $_SERVER;

		// If the server variables weren't specified, use
		// $_SERVER;
		if (!isset($server))
			$server = &$_SERVER;

		// If the body wasn't specified, use stdin.
		// We use this rather than $_POST because if the
		// method wasn't POST, PHP won't parse it for us.
		if (!isset($body))
			$body = file_get_contents("php://input");

		// XXX - Parse the body: get the content type, and
		// parse it as JSON, XML, YAML, or whatever.

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
		$path = $server['PATH_INFO'];
		$path = preg_replace(',^/,', '', $path);	// Remove leading slash
		list ($this->classname, $this->resource) =
			// Split up into class and resource ID.
			explode("/", $path, 2);

		// Parameters passed in through the URL
		if (isset($server['QUERY_STRING']))
			parse_str($server['QUERY_STRING'], $this->url_params);

		if (isset($server['CONTENT_TYPE']))
			$this->content_type = $server['CONTENT_TYPE'];

		// XXX - Authenticate/authorize the client.

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

		$this->finish(200, "Looks like things went okay.",
			      array("foo" => "bar"));

		// XXX
	}

	// XXX - dispatch(), to decide where the request should go:
	// which function should handle this request? Notionally, I
	// guess it should just look up a table with
	// { method, class+subclass, ID } -> function

	// XXX - error(): Send an error code and status to the caller.
	// Should this be further divided into HTTP_error and
	// REST_error? If the user is unauthorized, that should give
	// an HTTP status of 401 or whatever. But if it's something
	// like "no such feed", then the HTTP status should be 200.
	// Consensus seems to be leaning slightly toward the idea of
	// using HTTP codes for the success or failure of the
	// operation, rather than merely the HTTP part of it (that is,
	// if there are no network problems, no Apache problems, no
	// database problems, but the authorization cookie has
	// expired, it should still return a 401 HTTP status).

	// I guess in any case there should be "status" and "errmsg"
	// fields in the response.

	// XXX - return(): send the output back to the caller in the
	// desired format (JSON, XML, YAML).
	// Include "status" and "errmsg", I guess.

	// Accessors
	function method() {
		return $this->method;
	}

	function classname() {
		return $this->classname;
	}

	function resource() {
		return $this->resource;
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
			echo xmlify($val);
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

	// exit
	// Set the HTTP status and error message, and exit.
	function exit($status = 200, $msg = "OK")
	{
		http_response_code($status);
		$retval = array("errmsg" => $msg);
		// XXX - Print it in the desired output form
		print_r($retval);
		exit(0);
	}
}

$rreq = new RESTReq();
echo "Method: [", $rreq->method(), "]<br>\n";
echo "class [", $rreq->classname(), "]<br/>\n";
echo "resource [", $rreq->resource(), "]<br/>\n";
echo "output type: [", $rreq->url_param("o"), "]<br/>\n";
// XXX - Should there be a method for getting all the URL parameters?

echo "content-type: [", $rreq->content_type(), "]<br/>\n";
echo "body:<pre>[", $rreq->body(), "]<br/>\n";

// XXX - Check authentication

// XXX - Figure out where to send the request
switch ($class)
{
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

exit(0);
phpinfo();
?>
