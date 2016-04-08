<?php

class RESTController_test {
	function __construct()
	{
		//error_log("Inside RESTController_test::__construct");
	}

	/* hello
	 * Just return a fixed "hello world" result.
	 */
	function hello_test(&$rreq)
	{
		if ($rreq->verb() == "HEAD")
			header("X-Head: Yes");
		else
			header("X-Head: No");
		return array("hello" => "world");
	}

	/* echo
	 * Get the "s"(tring) parameter, and send back an array of the form
	 * { "answer" => $string }
	 */
	function echo_test(&$rreq)
	{
		$string = $rreq->url_param("s");
		if (!isset($string))
			$string = "You didn't say anything.";
		return array("answer"	=> $string,
			     "verb"	=> $rreq->verb(),
			     "path"	=> $rreq->path(),
			     "class"	=> $rreq->classname(),
			     "content-type"	=> $rreq->content_type()
			);
	}

	/* cat
	 * Echo the sender's body back at them.
	 */
	function cat_test(&$rreq)
	{
		$body_type = $rreq->content_type();
		$body = $rreq->body();

		if (!isset($body) || $body == "")
			$body = "[This body intentionally left blank.]";

		return array(
			"body_type" =>  $body_type,
			"body" => print_r($body, true),
			);
	}

	/* run
	 * Take an instance of a REST request, and dispatch it.
	 */
	function run($rreq)
	{
		$cmd = $rreq->shift_pathv();
		switch ($cmd)
		{
		    case "hello":
			// Just return a fixed string.
			return $this->hello_test($rreq);

		    case "echo":
			// Echo what the user sent us.
			return $this->echo_test($rreq);

		    case "cat":
			// Print the body back at the caller.
			return $this->cat_test($rreq);

		    default:
			throw new RESTException(NULL, "Invalid command");
		}

		// This should never happen
		return array("Australia" => "WTF?");
	}
}
?>
