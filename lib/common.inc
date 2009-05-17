<?php
/* common.inc
 * Widely-useful functions that don't fit anywhere else.
 */
// XXX - abort()
// It'd be nice if abort() could work even if the installation is broken.

/* redirect_to
 * Redirect the browser to $url.
 */
function redirect_to($url)
{
	header("Location: $url");
}

/* abort
 * Print the supplied error message, and abort.
 * First, tries to load the skin module and use a template to report
 * the error. If that fails, calls bare_abort() to print an ugly error
 * message.
 */
function abort($msg)
{
	/* Try to load the skin module. */
	$err = @include_once("skin.inc");
	if ($err === FALSE)
		bare_abort($msg);

	$skin = new Skin();
		// XXX - Error-checking. Actually, is any
		// error-checking possible?
	$skin->assign('message', $msg);

	/* Try to fetch the template, then print it: $skin->display()
	 * doesn't return a useful error code that I've found. So
	 * instead, fetch the text of the error page, then print it.
	 */
	$page = @$skin->fetch("abort.tpl");
	if ($page == "")
		bare_abort($msg);
	echo $page;
	exit(0);
}

/* bare_abort
 * A raw-PHP abort function. Doesn't use templates, but works
 * anywhere.
 */
function bare_abort($msg)
{
	echo '<', '?xml version="1.0" encoding="UTF-8"?', '>';
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Error</title>
</head>
<body id="view-body">

<p><?=htmlspecialchars($msg)?></p>

</body>
</html>
<?
	 exit(0);
}

/* jsonify
 * Convert the arguments to a JSON string, and return it.
 * If there's one argument, and it's a string, then it is assumed to
 * already be in JSON format, and it just needs to have characters
 * escaped.
 * If there are multiple arguments, or one argument that's an array,
 * it's assumed to be an assoc of key -> value pairs that will be
 * converted to a JavaScript object.
 * Returns FALSE in case of error.
 * The returned string is all on one line.
 * In the case of assocs (which return JavaScript objects), we assume
 * that the keys are valid JavaScript identifiers.
 */
// XXX - How to distinguish between assoc and array with an even
// number of elements? The way we have it now, with an even number of
// arguments indicating an assoc, is a hack. Perhaps change to
//	jsonify($is_assoc, $arg = NULL)
function jsonify($arg)
{
	/* See how many arguments there are, and assign $it to
	 * whatever it is we're converting.
	 */
	$argc = func_num_args();

	if ($argc == 1)
		$it =& $arg;
	elseif ($argv % 2 == 0)
	{
		// Convert array of arguments to an assoc.
		$it = array();
		for ($i = 0; $i < $argc; $i += 2)
			$it[func_get_arg($i)] = func_get_arg($i+1);
	} else
		$it = func_get_args();

	/* At this point, $it is the thing we're converting. Decide
	 * how to handle it, depending on whether $it is a string, an
	 * assoc, or an array.
	 */
	$retval = "";
	if (!isset($it))
		$retval = "null";
	elseif (is_numeric($it))
		$retval = &$it;
	elseif (is_bool($it))
		$retval = ($it ? "true" : "false");
	elseif (is_string($it))
		/* Escape chars in $it */
		$retval = "'" .
			addcslashes($it, "\\\'\n\r\"") .
			"'";
	elseif (is_array($it))
	{
		if (array_key_exists("0", $it))
		{
			/* Output a JavaScript array */
			// We need to do the silly dance with $it[0]
			// to avoid having an extra comma in the
			// output.
			$retval = '[' . jsonify($it[0]);
			for ($i = 1; $i < count($it); $i++)
				$retval .= ',' . jsonify($it[$i]);
			$retval .= ']';
		} else {
			/* Output a JavaScript object */
			$keys = array();
			$retval = '{';
			foreach ($it as $key => $value)
				$keys[] = $key . ':' . jsonify($value);
			$retval .= implode(',', $keys);
			$retval .= '}';
		}
	} else
		return FALSE;

	return $retval;
}

/* smarty_function_jsonify
 * Give Smarty access to jsonify().
 */
function smarty_function_jsonify($params, Smarty &$smarty)
{
	return jsonify($params['var']);
}
?>
