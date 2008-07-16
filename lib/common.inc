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
?>
