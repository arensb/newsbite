<?php
/* setskin.php
 * Set the skin to be used.
 */
// XXX - Make sure the cookie is being set correctly. Perhaps set a
// "cookie-test" cookie when the form is loaded, and make sure it's
// there when the filled-in form has been submitted.
require_once("common.inc");
require_once("skin.inc");

$skin = new Skin;

$current_skin = $_COOKIE['skin'];

if (isset($_REQUEST['set-skin']))
{
	// We've been given a skin by the form.
	$new_skin = $_REQUEST['newskin'];
		// XXX - Security: make sure skin is valid

	if ($skin->setskin($new_skin))
	{
		// XXX - Should set domain and path
		// domain: $_SERVER['SERVER_NAME'] ?
		// path: dirname($_SERVER['PHP_SELF']) ?
		setcookie("skin", $new_skin, 9999999999);
		redirect_to("index.php");
		exit(0);
	}
	// XXX - Otherwise, should fail somehow.
}

$skins = array();		// Array of available skins

/* See what directories exist in the "skins" directory */
$dh = opendir("skins");
if (!$dh)
{
	abort("Can't open skin directory.");
}

while (($fname = readdir($dh)) !== FALSE)
{
	if ($fname[0] == ".")
		continue;
	if (!is_dir("skins/$fname"))
		continue;

	$skins[] = array(
		"dir"	=> $fname,
		"name"	=> $fname	// XXX - Could be a fancier name
		);
}

$skin->assign('skins', $skins);
$skin->assign('current_skin', $current_skin);
$skin->assign('new_skin', $new_skin);
$skin->display("setskin.tpl");
?>
