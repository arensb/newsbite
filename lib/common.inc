<?php
/* common.inc
 * Widely-useful functions that don't fit anywhere else.
 * Also common startup stuff that gets done on every request.
 */

require_once("config.inc");

/* Try to guess whether we're viewing this on an iPhone or other
 * mobile device.
 */
$mobile = false;
if (preg_match(',Mozilla/\S+ \(iPod;,', $_SERVER['HTTP_USER_AGENT']))
{
	$mobile = "iPhone";
} else if (preg_match(',Mozilla/\S+ \(iPad;,', $_SERVER['HTTP_USER_AGENT']))
{
	$mobile = "iPad";
} else if (preg_match(',Mozilla/\S+ .*Android,', $_SERVER['HTTP_USER_AGENT']))
{
	$mobile = "Android";
}
else $mobile = FALSE;

/* Check to make sure the user is properly logged in. We put this here
 * because common.php is included by every script.
 *
 * A script can set $NO_AUTH_CHECK to avoid this check. In particular,
 * the login form shouldn't be blocked because the user hasn't
 * properly logged in.
 */
if (!(isset($NO_AUTH_CHECK) && $NO_AUTH_CHECK) &&
    !isset($_ENV['CRON']))	# Allow me to run update.php from cron
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
		// the user isn't logged in.
		if ($_REQUEST['o'] == "json")
		{
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
	elseif ($argv % 2 == 0)
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

if (!function_exists("json_encode")):
			// Define json_encode if the JSON PHP module
			// isn't loaded.
	// XXX - Perhaps ought to stick this in a separate file and
	// require() it.
/* json_encode
 * Helper function for jsonify: takes one argument, and returns its
 * representation as a JSON string.
 */
function json_encode($it)
{

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
		$retval = '"' .
			addcslashes($it, "\\\n\r\"") .
			'"';
	elseif (is_array($it))
	{
		if (array_key_exists("0", $it))
		{
			/* Output a JavaScript array */
			// We need to do the silly dance with $it[0]
			// to avoid having an extra comma in the
			// output.
			$retval = '[' . json_encode($it[0]);
			for ($i = 1; $i < count($it); $i++)
				$retval .= ',' . json_encode($it[$i]);
			$retval .= ']';
		} else {
			/* Output a JavaScript object */
			$keys = array();
			$retval = '{';
			foreach ($it as $key => $value)
				$keys[] = json_encode($key) . ':' . json_encode($value);
			$retval .= implode(',', $keys);
			$retval .= '}';
		}
	} else
		return FALSE;

	return $retval;
}
endif;	// !defined json_encode

?>
