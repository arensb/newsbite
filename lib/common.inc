<?php
/* common.inc
 * Widely-useful functions that don't fit anywhere else.
 * Also common startup stuff that gets done on every request.
 */

require_once("lib/config.inc");

if (php_sapi_name() == "cli")
{
	ini_set("error_log", "");	// Log to stderr
	// We're running from the command line
	$NO_AUTH_CHECK = TRUE;	// Disable login check
}

$NO_AUTH_CHECK = TRUE;
/* See what kind of output the user wants
 * If the script uses
 *	$out_fmt = "foo"
 *	require_once("lib/common.inc");
 * then it will only use "foo" (this is typically used for HTML pages).
 *
 * If the script uses
 * If the script uses
 *	$default_fmt = "foo"
 *	require_once("lib/common.inc");
 * then the code below will try the "-o lang" option if called from
 * the command line, or the "o=lang" parameter if called as a CGI
 * script, and default to "$out_fmt = foo" if it's not set, or if
 * there's a problem.
 */
if (!isset($default_fmt))
	$default_fmt = "json";
if (!isset($out_fmt))
{
	if (isset($_REQUEST['o']))
	{
		// $out_fmt isn't set, but $_REQUEST[o] is.
		switch ($_REQUEST['o'])
		{
		    case "json":
		    case "xml":
			$out_fmt = $_REQUEST['o'];
			break;
		    case "":		// Empty string. Use default
			$out_fmt = $default_fmt;
			break;
		    default:
			error_log("Invalid output format \"" . $_REQUEST['o'] . "\". Using default.");
			$out_fmt = $default_fmt;
			break;
		}
	} else {
		// Neither $out_fmt nor $_REQUEST[o] is set.
		$out_fmt = $default_fmt;
	}
}

/* Initialize for the given output format */
switch ($out_fmt)
{
    case "json":
	header("Content-type: text/plain; charset=utf-8");
	break;
    case "xml":
	header("Content-type: text/xml; charset=utf-8");
	require_once("lib/xml-output.inc");
	break;
    case "console":
	break;
    default:
	header("Content-type: text/html; charset=utf-8");
	$out_fmt = "html";
	break;
}

/* Check to make sure the user is properly logged in. We put this here
 * because common.php is included by every script.
 *
 * A script can set $NO_AUTH_CHECK to avoid this check. In particular,
 * the login form shouldn't be blocked because the user hasn't
 * properly logged in.
 */
if (!(isset($NO_AUTH_CHECK)) || !$NO_AUTH_CHECK)
{
	/* !!NB: A lot of the identifiers in this section are
	 * replicated in login.php, which can lead to subtle problems.
	 * (E.g., name of the cookie, names of the config variables,
	 * etc.)
	 */
	$login_cookie = NULL;
	if (array_key_exists('newsbite_user', $_COOKIE))
		$login_cookie = $_COOKIE['newsbite_user'];

	# XXX - This code probably belongs in login.php. Except, want
	# to avoid infinite recursion of #includes.
	if (!isset($login_cookie))
	{
		// If output type is JSON, return a JSON string saying
		// the user isn't logged in. Also, send them a 401
		// status code.
		if ($out_fmt == "json")
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
<?php
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
<?php
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

	// What follows is basically a template for the abort error page.
########################################
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Error</title>
<link rel="stylesheet" type="text/css" href="css/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="css/abort.css" media="all" />
</head>
<body id="abort-body">

<h1>Error</h1>

<p><?=htmlspecialchars($msg)?></p>

<hr/>

<ul>
  <li><a href="index.html">Feed index</a></li>
</ul>

</body>
</html>
<?php
########################################
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

/* print_struct
 * Print a data structure, according to $out_fmt.
 *
 * This is intended to be used at the bottom of REST-like scripts:
 * they take "o=fmt" as a standard request option (see above), then
 * typically collect some data and return it as a data structure. This
 * function sends that to the caller.
 */
// XXX - Perhaps this should return a value saying whether the conversion
// was successful. That way, other scripts can try print_struct(), and if
// that fails, resort to echo or whatever.
function print_struct(&$val)
{
	global $out_fmt;

	switch ($out_fmt)
	{
	    case "json":
		echo jsonify($val);
		break;

	    case "xml":
		echo xmlify($val);
		break;

	    default:
		error_log("Unknown output format in print_struct: $out_fmt");
		// Default to JSON.
		echo jsonify($val);
		break;
	}
}

if (!function_exists("json_encode"))
	require_once("lib/json.inc");	// Load compatibility library
?>
