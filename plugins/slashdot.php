<?php
/* Remove junk from the bottom of Slashdot posts (classic mode) */

function slashdot_noise_hook($nodename, &$retval, &$context)
{
	if (!is_string($retval))
		return;

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

	# Unnecessary link to story
	$retval = preg_replace('{\s*<p><a href="http://(\w+\.)?slashdot.org/story/[^\"]*">Read more of this story</a> at Slashdot.</p>}',
			       '',
			       $retval);
}

add_hook("summary", "slashdot_noise_hook");

?>
