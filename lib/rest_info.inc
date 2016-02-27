<?php
/* rest_info
 * Information commands
 */
function info_stuff($rreq)
{
	switch ($rreq->subpath())
	{
	    case NULL:
	    case "":
		return GET_info($rreq);

	    case "version":
		switch ($rreq->verb())
		{
		    case "GET":
			return GET_info_version($rreq);

		    default:
			error_log("rest_info: bad verb " . $rreq->verb());
			throw new RESTInvalidVerb();
		}

	    default:
		error_log("rest_info: bad subpath " .
			  ($rreq->subpath() === NULL ? "NULL" :
			   $rreq->subpath()));
		throw new RESTInvalidCommand();
	}

	// This should never happen
	error_log("Somehow got to an impossible line.");
	$rreq->finish(501, "Internal error.", NULL);
}

function GET_info($rreq)
{
	return array(
		"info" => "I'd love to give you some info, really, but I can't.",
		);
}

function GET_info_version($rreq)
{
	return array(
		"version" => "Sorry, I don't know my own version yet."
		);
}
?>
