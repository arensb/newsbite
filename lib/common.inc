<?php
/* common.inc
 * Widely-useful functions that don't fit anywhere else.
 * Also common startup stuff that gets done on every request.
 */

require_once("config.inc");

/* See what kind of output the user wants */
if (!isset($out_fmt))
{
	switch (isset($_REQUEST['o']) ? $_REQUEST['o'] : "")
	{
	    case "json":
		// XXX - If another browser comes along that can't deal with
		// standard JSON, add a "hack" parameter or something to say
		// how to work around it.
		$out_fmt = "json";
		header("Content-type: text/plain; charset=utf-8");
		break;
	    case "xml":
		$out_fmt = "xml";
		header("Content-type: text/xml; charset=utf-8");
		break;
	    default:
		header("Content-type: text/html; charset=utf-8");
		$out_fmt = "html";
		break;
	}
}

/* Check to make sure the user is properly logged in. We put this here
 * because common.php is included by every script.
 *
 * A script can set $NO_AUTH_CHECK to avoid this check. In particular,
 * the login form shouldn't be blocked because the user hasn't
 * properly logged in.
 */
if (!(isset($NO_AUTH_CHECK) && $NO_AUTH_CHECK))
{
	/* !!NB: A lot of the identifiers in this section are
	 * replicated in login.php, which can lead to subtle problems.
	 * (E.g., name of the cookie, names of the config variables,
	 * etc.)
	 */
	$login_cookie = $_COOKIE['newsbite_user'];

	# XXX - This code probably belongs in login.php. Except, want
	# to avoid infinite recursion of #includes.
	if (!isset($login_cookie))
	{
		// If output type is JSON, return a JSON string saying
		// the user isn't logged in. Also, send them a 401
		// status code.
		if ($_REQUEST['o'] == "json")
		{
			header("HTTP/1.0 401 You are not logged in");
			header("Content-type: text/plain; charset=utf-8");
			echo jsonify('state',	"error",
				     'error',	"You are not logged in.");
			exit(0);
		}

		# XXX - Ought to allow redirect to https://...
		redirect_to("login.php?from=" .
			 urlencode($_SERVER['REQUEST_URI']));
?>
<html>
<head><title>You're not logged in</title></head>
<body>
<h1>You're not logged in</h1>
<p>You came from [<?=urlencode($_SERVER['REQUEST_URI'])?>]</p>
<p><a href="login.php?from=<?=urlencode($_SERVER['REQUEST_URI'])?>">Go log in</a></p>
</body>
</html>
<?
		exit(0);
	}

	/* Check whether the cookie has expired */
	$now = time();

	global $auth_user;
	global $auth_expiration;
	list($auth_user, $auth_expiration, $hmac_sum) =
		explode("|", $_COOKIE['newsbite_user']);
		// XXX - Ought to check all of these fields
	if (!is_numeric($auth_expiration) ||
	    ($now > $auth_expiration))
	{
		redirect_to("login.php?from=" . urlencode($_SERVER['REQUEST_URI']));
?>
<html>
<head><title>You're not logged in</title></head>
<body>
<h1>Your login is invalid</h1>
<p>You came from [<?=urlencode($_SERVER['REQUEST_URI'])?>]</p>
<p><a href="login.php?from=<?=urlencode($_SERVER['REQUEST_URI'])?>">Go log in</a></p>
</body>
</html>
<?
		exit(0);
	}

	/* Cookie hasn't expired. Make sure the signature is good */
	// XXX - Probably want a separate function for calculating the HMAC.
	$good_hmac = md5(implode("|",
			    array($auth_user, $auth_expiration, SERVER_SECRET)));
	if ($hmac_sum != $good_hmac)
	{
		// This appears to be a breakin attempt. Do nothing.
#echo "<p>You seem to be trying to break in.</p>\n";
		exit(0);
	}
}

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
	global $out_fmt;

	# Abort handling for non-HTML output types.
	if ($out_fmt == "json")
	{
		echo jsonify(array("error" => $msg));
		exit(0);
	} elseif ($out_fmt == "xml")
	{
		print_xml(array("error" => $msg));
		exit(0);
	}

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
	$page = @$skin->fetch("abort");
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
 *
 * NB: Mozilla's JSON.parse() expects identifiers to be quoted as well
 * (using double quotes). Thus:
 *	{x:1}	fails, and
 *	{'x':1} fails, but
 *	{"x":1}	succeeds.
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
	elseif ($argc % 2 == 0)
	{
		// Convert array of arguments to an assoc.
		$it = array();
		for ($i = 0; $i < $argc; $i += 2)
			$it[func_get_arg($i)] = func_get_arg($i+1);
	} else
		$it = func_get_args();

	/* Now that we've got the complete data structure in one variable,
	 * pass it to json_encode().
	 */
	return json_encode($it);
}

if (!function_exists("json_encode"))
	require_once("json.inc");	// Load compatibility library
?>
