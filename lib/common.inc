<?php
/* common.inc
 * Widely-useful functions that don't fit anywhere else.
 */
// XXX - abort()
// It'd be nice if abort() could work even if the installation is broken.

require_once("config.inc");

/* Check to make sure the user is properly logged in. We put this here
 * because common.php is included by every script.
 *
 * A script can set $NO_AUTH_CHECK to avoid this check. In particular,
 * the login form shouldn't be blocked because the user hasn't
 * properly logged in.
 */
if (!$NO_AUTH_CHECK &&
    !isset($ENV['CRON']))	# Allow me to run update.php from cron
{
	/* XXX - A lot of the identifiers in this section are
	 * replicated in login.php, which can lead to subtle problems.
	 * (E.g., name of the cookie, names of the config variables,
	 * etc.)
	 */
	$login_cookie = $_COOKIE['newsbite_user'];

	# XXX - This code probably belongs in login.php. Except, want
	# to avoid infinite recursion of #includes.
	if (!isset($login_cookie))
	{
		# XXX - Ought to allow redirect to https://...
#		redirect_to("login.php?from=" .
#			 urlencode($_SERVER['REQUEST_URI']));
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
#		redirect_to("login.php?from=" . urlencode($_SERVER['REQUEST_URI']));
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

	/* Cookie hasn't expired. Make sure the signature is good */
	// XXX - Probably want a separate function for calculating the HMAC.
	$good_hmac = md5(implode("|",
			    array($auth_user, $auth_expiration, SERVER_SECRET)));
	if ($hmac_sum != $good_hmac)
	{
		// This appears to be a breakin attempt. Do nothing.
echo "<p>You seem to be trying to break in.</p>\n";
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

/* xmlify
 * Convert the argument to an XML document, and return it.
 */
function xmlify($arg, $name="contents")
{
	$retval = '<'.'?xml version="1.0" encoding="utf-8"?'.">\n";
	// XXX - For now, assume the argument is an assoc.
	$retval .= "<$name>\n" . xmlify1($arg) . "</$name>\n";
	return $retval;
}

function xmlify1($it)
{
	if (!isset($it))
		;
	elseif (is_numeric($it))
		$retval .= $it;
	elseif (is_bool($it))
		$retval .= ($it ? "true" : "false");
	elseif (is_string($it))
		$retval .= '<![CDATA[' .
			htmlspecialchars($it) .		// XXX - Escape chars
			']]>';
	elseif (is_array($it))
	{
		if (array_key_exists("0", $it))
		{
			/* Output an array */
			foreach ($it as $i)
			{
#echo "xmlifying [$i]\n";
				$retval .= "<item>\n" .
					xmlify1($i) .
					"</item>\n";
#echo "retval == [$retval]\n";
			}
		} else {
			/* Output a series of tags */
			// XXX
			foreach ($it as $key => $value)
			{
//				if (!isset($value))
//					continue;
				$retval .= "<$key>" .
					xmlify1($value) .
					"</$key>\n";
			}
		}
	} else
		return FALSE;

	return $retval;
}

/* print_xml
 * Like xmlify(), this converts $arg to XML, but it doesn't bother
 * converting it to a string; it just prints the result.
 */
function print_xml($arg, $name="contents")
{
	echo  '<','?xml version="1.0" encoding="utf-8"?',">\n";
	// XXX - For now, assume the argument is an assoc.
	echo "<$name>\n";
	print_xml1($arg);
	echo "</$name>\n";
}

function print_xml1($it)
{
	if (!isset($it))
		return TRUE;
	if (is_numeric($it))
	{
		echo $i;
		return TRUE;
	}
	if (is_bool($it))
	{
		echo ($it ? "true" : "false");
		return TRUE;
	}
	if (is_string($it))
	{
		echo '<![CDATA[',
			htmlspecialchars($it),		// XXX - Escape chars
			']]>';
		return TRUE;
	}
	if (is_array($it))
	{
		if (array_key_exists("0", $it))
		{
			/* Output an array */
			foreach ($it as $i)
			{
				echo "<item>\n";
				print_xml1($i);
				echo "</item>\n";
			}
		} else {
			/* Output a series of tags */
			foreach ($it as $key => $value)
			{
				if (!isset($value))
					continue;
				echo "<$key>";
				print_xml1($value);
				echo "</$key>\n";
			}
		}
		return TRUE;
	}
	return FALSE;
}

?>
