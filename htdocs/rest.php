<?php
// XXX - See
// http://www.lornajane.net/posts/2012/building-a-restful-php-server-understanding-the-request
// for ideas

class RESTReq
{
	protected $method = NULL;
	protected $classname = NULL;
	protected $resource = NULL;
	protected $url_params = array();
	protected $content_type = NULL;

	function __construct(&$server, &$body)
	{
		// Query method: GET, PUT, POST, etc.
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

		// XXX
	}

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
}

// Read the contents of the file we were passed in the body. If it's a
// form submitted with POST, then PHP will parse the fields nicely and
// put them in $_POST, but it doesn't parse things like JSON bodies.
$body = file_get_contents("php://input");

$rreq = new RESTReq($_SERVER, $body);
#echo "object's method is ", $rreq->method(), "<br/>\n";
echo "Method: [", $rreq->method(), "]<br>\n";
echo "class [", $rreq->classname(), "]<br/>\n";
echo "resource [", $rreq->resource(), "]<br/>\n";
echo "output type: [", $rreq->url_param("o"), "]<br/>\n";
// XXX - Should there be a method for getting all the URL parameters?

echo "content-type: [", $rreq->content_type(), "]<br/>\n";
echo "body:<pre>[$body]<br/>\n";

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
