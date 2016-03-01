<?php
function test_stuff($rreq)
{
	switch ($rreq->subpath())
	{
	    case "hello":
		// Just return a fixed string.
		return test_hello($rreq);

	    case "echo":
		// Echo what the user sent us.
		return test_echo($rreq);

	    case "cat":
		// Print the body back at the caller.
		return test_cat($rreq);

	    default:
		throw new RESTInvalidCommand();
	}

	// This should never happen
	return array("Australia" => "WTF?");
}

/* test_hello
 * Just return a fixed "hello world" result.
 */
function test_hello(&$rreq)
{
	return array("hello" => "world");
}

/* test_echo
 * Get the "s"(tring) parameter, and send back an array of the form
 * { "answer" => $string }
 */
function test_echo(&$rreq)
{
	$string = $rreq->url_param("s");
	if (isset($string))
		return array("answer" => $string);
	return array("answer" => "You didn't say anything.");
}

/* test_cat
 * Echo the sender's body back at them.
 */
function test_cat(&$rreq)
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
?>
