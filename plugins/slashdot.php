<?php
/* Remove junk from the bottom of Slashdot posts (classic mode) */

function slashdot_noise_hook($nodename, &$retval, &$context)
{
#echo "Inside slashdot_noise_hook<br/>\n";
	if (!is_string($retval))
		return;
#echo "(slashdot_noise) before: <pre>["; print_r($retval); echo "]</pre>\n";

	// Facebook and Twitter links
	$retval = preg_replace('{<a class="slashpop".*?</a>}',
			       '',
			       $retval);
	// Google+ link
	$retval = preg_replace('{<a class="nobg" href="http://plus.google.com/.*?</a>}',
			       '',
			       $retval);

	# Empty paragraphs
	$retval = preg_replace('{<p>\s*</p>}s',
			       '',
			       $retval);

#echo "(slashdot_noise) after 1: <pre>["; print_r($retval); echo "]</pre>\n";
	# Unnecessary link to story
	# XXX - Should this check to see whether the link is the same as in the "header"?
	$retval = preg_replace('{\s*<p><a href="http://(\w+\.)?slashdot.org/story/[^\"]*">Read more of this story</a> at Slashdot.</p>}',
			       '',
			       $retval);
#echo "(slashdot_noise) after 2: <pre>["; print_r($retval); echo "]</pre>\n";
}

add_hook("summary", "slashdot_noise_hook");

?>
