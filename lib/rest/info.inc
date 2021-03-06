<?php
class RESTController_info {

	/* rest_info
	 * Information commands
	 */
	function run($rreq)
	{
		$cmd = $rreq->shift_pathv();
		switch ($cmd)
		{
		    case NULL:
		    case "":
			// XXX - Ought to check verb.
			return $this->info($rreq);

		    case "version":
			switch ($rreq->verb())
			{
			    case "GET":
				return $this->info_version($rreq);

			    default:
				error_log("rest_info: bad verb " . $rreq->verb());
				throw new RESTException(NULL, "Invalid verb.");
			}

		    default:
			error_log("rest info: bad command " .
				  ($cmd === NULL ? "NULL" : $cmd));
			throw new RESTException(NULL, "Invalid command");
		}

		// This should never happen
		error_log("Somehow got to an impossible line.");
		$rreq->finish(501, "Internal error.", NULL);
	}

	function info($rreq)
	{
		return array(
			"info" => "I'd love to give you some info, really, but I can't.",
			);
	}

	function info_version($rreq)
	{
		return array(
			"version" => "Sorry, I don't know my own version yet."
			);
	}
}
?>
