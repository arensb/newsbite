<?php
/* rest.php
 * REST-related PHP stuff.
 */

// REST-related classes and such. Inspired by
// http://www.lornajane.net/posts/2012/building-a-restful-php-server-understanding-the-request
class RESTException extends Exception {
	public $errno = NULL;
	public $errmsg = NULL;

	function __construct($_errno = NULL, $_errmsg = NULL)
	{
		if (isset($_errno))
			$this->errno = $_errno;
		if (isset($_errmsg))
			$this->errmsg = $_errmsg;
	}
};

/* XmlElement
 * Used when converting from XML to data structure. Used by
 * _parse_xml, below.
 */
class XmlElement {
	var $name;
	var $attributes;
	var $content;
	var $children;

	function findChildrenByName($name, $limit = 0)
	{
		$retval = array();
		if (!isset($this->children))
			// No such child
			return FALSE;

		foreach ($this->children as $child)
		{
			if (isset($child->name) &&
			    $child->name == $name)
				array_push($retval, $child);
		}
		if (count($retval) > 0)
			return $retval;
		return FALSE;
	}
};

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
	protected $verb = NULL;
	// $path is the path underneath the root.
	// $classname is the first element of $path, and
	// $pathv (array) is the rest of it.
	// If the REST root is http://foo.com/w1/ , then
	// http://foo.com/w1/feeds/stats/123 ->
	//	$path == "feeds/stats/123"
	//	$classname == "feeds"
	//	$pathv == ("stats", "123")
	protected $path = NULL;
	protected $pathv = array();
	protected $classname = NULL;
	protected $url_params = array();
	protected $content_type = NULL;
	// XXX - Is there a reason to keep both the text and parsed
	// versions of the body?
	protected $body_text = NULL;
	protected $body = NULL;
	protected $outfmt = "json";	// Desired output format

	function __construct(&$server = NULL, &$body_text = NULL)
	{
		/* We don't authenticate or authorize the caller here.
		 * Rather, we rely on .htaccess to authenticate (and
		 * assume that anyone who's authenticated is
		 * authorized).
		 */
		global $_SERVER;

		// If the server variables weren't specified, use
		// $_SERVER;
		if (!isset($server))
			$server = &$_SERVER;

		// Query verb: GET, PUT, POST, etc.
		if (!isset($server['REQUEST_METHOD']))
			throw new RESTException(NULL, "No verb");
		$this->verb = $server['REQUEST_METHOD'];

		// Get the path. The first part is the class, and the
		// rest is either a subclass, an identifier, or
		// something.
		// It's not an error to just specify a class. In that
		// case, $pathv is an empty array.
		$this->path = $_GET['path'];

		/* Break up the path into components */
		// XXX - Do we want to use 'preg_*' or something to
		// strip the leading slash?
		$this->pathv = explode("/", $this->path);
		if ($this->pathv[0] == "")
			array_shift($this->pathv);
		if (count($this->pathv) > 0)
		{
			// There was a slash at the beginning of the path.
			$this->classname = $this->pathv[0];
			array_shift($this->pathv);
		} else
			throw new RESTException(NULL, "Invalid command");

		// Parameters passed in through the URL
		if (isset($server['QUERY_STRING']))
			parse_str($server['QUERY_STRING'], $this->url_params);

		if (isset($server['CONTENT_TYPE']))
		{
			$fields = preg_split('/\s*;\s*/', $server['CONTENT_TYPE']);
				// XXX - According to
				// http://www.ietf.org/rfc/rfc3875 ,
				// the "Content-Type" header can be
				// followed by parameters of the form
				// "var=value", separated by
				// semicolons. This is most usually
				// used for "charset=UTF-8". Right
				// now, we don't care about those, but
				// we do need to get the content type.
			$this->content_type = $fields[0];
		}

		// If the body wasn't specified, use stdin.
		// We use this rather than $_POST because if the
		// verb wasn't POST, PHP won't parse it for us.
		if (isset($body_text))
			$this->body_text = $body_text;
		else
			$this->body_text = file_get_contents("php://input");

		// XXX - Parse the body: get the content type, and
		// parse it as JSON, XML, YAML, or whatever.
		// json_decode(): http://php.net/manual/en/function.json-decode.php

		// XXX - Do we want to move this code to body()? That
		// way, if the implementing code doesn't actually need
		// the body to be parsed, we needn't waste time
		// parsing it. Then again, that's probably a rare
		// case.
		switch ($this->content_type)
		{
		    case "application/json":
			$this->body = json_decode($this->body_text, true);
			break;

		    case "text/xml":
			$this->body = $this->_parse_xml($this->body_text);
			break;

		    default:
			// Leave it alone. Maybe a handler class knows
			// what to do with it.
			break;
		}

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
	}

	/* _parse_xml
	 * Convert XML text to a data structure. Based on
	 * xml_to_object by efredricksen at gmail dot com, at
	 * http://php.net/manual/en/function.xml-parse-into-struct.php
	 */
	function _parse_xml($xml)
	{
		$parser = xml_parser_create();
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
		xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
		xml_parse_into_struct($parser, $xml, $tags);
		xml_parser_free($parser);

		$elements = array();
				// the currently filling [child] XmlElement array
		$stack = array();
		foreach ($tags as $tag)
		{
			$index = count($elements);
			if ($tag['type'] == "complete" ||
			    $tag['type'] == "open")
			{
				$elements[$index] = new XmlElement;
				$elements[$index]->name = $tag['tag'];
				if (isset($tag['attributes']))
					$elements[$index]->attributes = $tag['attributes'];
				if (isset($tag['value']))
					$elements[$index]->content = $tag['value'];
				if ($tag['type'] == "open")
				{
					// push
					$elements[$index]->children = array();
					$stack[count($stack)] = &$elements;
					$elements = &$elements[$index]->children;
				}
			}
			if ($tag['type'] == "close")
			{
				// pop
				$elements = &$stack[count($stack) - 1];
				unset($stack[count($stack) - 1]);
			}
		}
		return $elements[0];  // the single top-level element
	}

	// XXX - dispatch(), to decide where the request should go:
	// which function should handle this request? Notionally, I
	// guess it should just look up a table with
	// { verb, class+subclass, ID } -> function
	//
	// Let's leave this until a bit later, when we have a better
	// idea of what makes sense.

	// Accessors
	function verb() {
		return $this->verb;
	}

	function path() {
		return $this->path;
	}

	function classname() {
		return $this->classname;
	}

	/* pathv
	 * With a non-NULL argument, return the $n-th element of
	 * $pathv (starting at 0). If $pathv is shorter than $n,
	 * return NULL.
	 * Without an argument, just return $pathv.
	 */
	function pathv($n = NULL)
	{
		if (isset($n))
		{
			if ($n >= count($this->pathv))
				return NULL;
			return $this->pathv[$n];
		} else
			return $this->pathv;
	}

	function shift_pathv()
	{
		return array_shift($this->pathv);
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

	// body: returns parsed version of the body
	function body() {
		return $this->body;
	}

	// body_text: returns raw text version of the body
	function body_text() {
		return $this->body_text;
	}

	// body_param
	// Commonly, the body will be a JSON hash, of the form
	//	{
	//		"param1": value1,
	//		"param2": value2,
	//		...
	//	}
	// For these, it's convenient to just pull a given parameter
	// out of the body.
	function body_param($param)
	{
		if (!is_array($this->body))
			return NULL;
		if (!array_key_exists($param, $this->body))
			return NULL;
		return $this->body[$param];
	}

	// print_struct
	// Print a data structure in the desired output format.
	function print_struct(&$val)
	{
		switch ($this->outfmt)
		{
		    case "json":
			header("Content-type: application/json; charset=utf-8");
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
		// If it's an error, log it.
		if ($status < 200 || $status > 299)
			error_log("Exiting " .
				  $this->verb() . " " .
				  $this->path() .
				  " with status $status, " .
				  (isset($msg) ?
				   "Error message \"$msg\"." :
				   "No error message."));
		http_response_code($status);
		if (isset($msg) && $msg != "")
			header("X-Newsbite-Error: " . $msg);

		if (isset($retval))
			$this->print_struct($retval);
		exit(0);
	}
}
?>
