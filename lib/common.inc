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
?>
