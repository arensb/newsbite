<?php
/* view.php
 * Display a feed.
 */
require_once("common.inc");
require_once("skin.inc");

/* If we get this far, user has requested HTML output */
$skin = new Skin();

## XXX - Debugging
$skin->assign('auth_user', $auth_user);
$skin->assign('auth_expiration', strftime("%c", $auth_expiration));
## XXX - end debugging
$skin->display("view");
?>
