<?php
/* Add paragraph marks to feeds that don't have them. Notably The Blaze
 */
function paragraph_hook($nodename, &$retval, &$context)
{
	/* XXX - Check whether the text already has paragraph breaks */
	/* XXX - If not, replace
	 * ^ with <p>
	 * $ with </p>
	 * \n\n with </p>\n\n<p>
	 */
	if (preg_match(',</?p\b,', $retval))
	{
		/* Summary/body already has paragraph breaks */
		return;
	}

	/* Replace \n\n with "</p>\n\n</p>" */
	$retval = preg_replace('/\n\n/', "</p>\n\n</p>", $retval);

	/* Prepend <p> if necessary */
	if (!preg_match('/^\s*</s', $retval))
		$retval = "<p>" . $retval;

	/* Append </p> if necessary */
	if (!preg_match('/>\s*$/s', $retval))
		$retval .= "</p>";
}

add_hook("summary", "googleads_hook");
add_hook("body", "paragraph_hook");

?>
