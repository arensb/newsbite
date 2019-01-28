<?php
$default_fmt = "json";
require_once("lib/common.inc");	// For authentication, mostly
require_once("lib/rest.inc");

$rreq = new RESTReq();
$retval = array();
$retval["verb"] = $rreq->verb();
$retval["path"] = $rreq->path();
$retval["class"] = $rreq->classname();
$retval["pathv"] = $rreq->pathv();
$retval["outfmt"] = $rreq->url_param("o");
$retval["content_type"] = $rreq->content_type();
$retval["body_text"] = $rreq->body_text();
$retval["body"] = $rreq->body();
// XXX - Should there be a method for getting all the URL parameters?

// XXX - If the class is "login", we don't require authentication. But
// the client needs to present credentials.

// XXX - Check authentication.

// XXX - Figure out where to send the request
$classname = $rreq->classname();
	// XXX - Perhaps sanitize $classname: allow only letters,
	// digits, and underscore?
switch ($classname)
{
    case "test":
    case "info":	// Information about Newsbite
    case "opml":	// OPML feeds
    case "feed":
    case "group":
    case "article":
	try {
		// Load the code that'll handle this class.
		$err = require_once("rest/$classname.inc");
			// XXX - Error-checking.

		// Create and run the controller for this class.
		$ctrl_classname = "RESTController_$classname";
		$controller = new $ctrl_classname();
			// XXX - Error-checking
		$retval = $controller->run($rreq);
			// XXX - How can we figure out whether this
			// was a normal return, or an error, or
			// whatever? Do we want to rely on exceptions?
	} catch (Exception $e) {
		error_log("Exception while loading rest/$classname.inc: " .
			  (isset($e->errno) ? $e->errno . ": " : "") .
			  (isset($e->errmsg) ? $e->errmsg : ""));
		$rreq->finish(400, "Class $classname: Caught an exception");
	} 
	break;

    default:
	// XXX - Die with an error?
	$rreq->finish(400, "Invalid class");
	break;
}

// XXX - HTTP response. Usually 200, but we might need to send 4xx or
// even 5xx.

// Send the return value, in the format the user wants.
$rreq->finish(200, NULL, $retval);
?>
