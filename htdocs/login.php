<?php
/* login.php
 * Authenticate the user.
 *
 * Note that _checking_ whether the user is authenticated is done in
 * "lib/common.inc" (since authentication has to be done for
 * everything, and every page includes common.inc).
 *
 * The authentication method is Fu's, as described in
 * http://www.cse.msu.edu/~alexliu/publications/Cookie/cookie.pdf
 *
 * The basic approach, though, is that once the user has authenticated,
 * to set a "newsbite_user" cookie with the value
 *	user "|" expiration "|" md5(user "|" expiration "|" secret)
 * The pipes are literal pipe characters, and are used as
 * concatenation operators, to ensure that we can tell the fields
 * apart.
 *
 * "user" is the user name, either as given by Apache authentication,
 * or as defined in "config.inc".
 *
 * "expiration" is the expiration time of the login session, as a
 * time_t.
 *
 * The third field in the cookie is the md5 sum of everything before
 * that, plus a secret value known only to the server (defined in
 * "config.php"). This ensures that the other data in the cookie is
 * good.
 *
 * Note that this system is vulnerable to a replay attack: if I can
 * steal your cookie, I can authenticate as you. So cookies expire to
 * reduce the window of vulnerability.
 *
 * To prevent the replay attacks, it would be best to add another
 * piece of data to the HMAC that is unique to the session, such as
 * the IP address (questionable) or the SSL session.
 *
 * Actually, the IP address isn't constant, since I want to move from
 * one WiFi net to the next. SSL would suffer from the same problem,
 * since every time I moved to another network, I'd get a new SSL
 * session.
 *
 *
 * The reason for using an authentication cookie (rather than, say,
 * Apache digest authentication) is that the iPod Touch and iPad don't
 * actually store passwords, even though they say they do. Or maybe
 * I'm misunderstanding how this is supposed to work, but the upshot
 * is that with just digest authentication, I have to type the
 * password in a lot more often than I want to.
 */
$NO_AUTH_CHECK = true;		// Don't authenticate the user in
				// common.inc. That's what the code
				// below is for.
require_once("common.inc");

$from = urldecode($_REQUEST['from']);	// Where did user come from?

if ($from == "")
	# If no "from=" specified, dump them back to the main page.
	$from = dirname($_SERVER['REQUEST_URI']);

function set_auth_cookie($user)
{
	// Set "user" cookie to
	// $user | $expiration | md5($user | $expiration | $secret);
	$now = time();
	$expiration = $now + AUTH_COOKIE_DURATION;

	$hmac = md5(implode("|",
			    array($user, $expiration, SERVER_SECRET)));
	setcookie('newsbite_user',
		  implode("|",
			  array($user, $expiration, $hmac)),
		  $expiration);
}

/* See if the user has already been authenticated through Apache */
if (isset($_SERVER['REMOTE_USER']))
{
	/* Apache has authenticated this user. Set the cookie and
	 * send the browser back to where the user originally wanted
	 * to go.
	 */
	set_auth_cookie($_SERVER['REMOTE_USER']);

	// If $out_fmt is "json", then send a JSON object saying the
	// user is authenticated.
	if ($out_fmt == "json")
	{
		// XXX - Convert to print_struct()?
		echo jsonify(
			'status',	"logged in"
			);
		exit(0);
	}

	// Otherwise (HTML output, presumably) redirect to where they
	// came from.
	redirect_to($from);
?>
<html>
<head><title>Authenticated by Apache</title></head>
<body>
<h1>You've been authenticated through Apache</h1>
<p><a href="<?=$from?>">Back to where you came from</a></p>
</body>
</html>
<?php
	exit(0);
}

/* Apache hasn't authenticated the user. Do it ourselves. */
echo "Apache user not set<br/>\n";
$user = $_REQUEST['user'];
$pass = $_REQUEST['pass'];

if (isset($user) &&
    isset($pass)) :
	if ($user == USERNAME &&
	    $pass == PASSWORD) :
		set_auth_cookie($user);
		redirect_to($from);
#		exit(0);
?>
<html>
<body>
<h1>Login successful</h1>
<p>Welcome, <?=$user?></p>

<p><a href="<?=$from?>">Back to where you came from</a></p>
</body>
</html>
<?php
	else:	// Login unsuccessful
?>
<html>
<body>
<h1>Login unsuccessful</h1>
<p>Sorry, <?=$user?> (from=<pre>[<?=$from?>]</pre>)</p>

<p><a href="<?=$from?>">Back to where you came from</a></p>
</body>
</html>
<?php	endif;
else:
/* User hasn't authenticated correctly */
?>
<html>
<body>
<p>Referer: <pre>[<?=$from?>]</pre></p>

<p>You're not yet authenticated.</p>

<form method="post" action="login.php">
Username: <input name="user" type="text"/><br/>
Password: <input name="pass" type="password"/><br/>
<input type="hidden" name="from" value="<?=urlencode($from)?>"/>
<input type="submit" value="Log in"/>
</form>

<!-- <?phpinfo()?> -->
</body>
</html>
<?php endif; ?>
