<?
/* login.php
 * Authenticate the user.
 */
$NO_AUTH_CHECK = true;		// Don't authenticate the user. That's
				// what the code below is for.
require_once("config.inc");
require_once("common.inc");

$from = Urldecode($_REQUEST['from']);	// Where did user come from?

function set_auth_cookie($user)
{
	// XXX - Set "user" cookie to
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
	redirect_to($from);
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
		exit(0);
?>
<html>
<body>
<h1>Login successful</h1>
<p>Welcome, <?=$user?></p>

<p><a href="<?=$from?>">Back to where you came from</a></p>
</body>
</html>
<?
	else:	// Login unsuccessful
?>
<html>
<body>
<h1>Login unsuccessful</h1>
<p>Sorry, <?=$user?> (from=<pre>[<?$from?>]</pre>)</p>

<p><a href="<?=$from?>">Back to where you came from</a></p>
</body>
</html>
<?	endif;
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
<? endif; ?>
