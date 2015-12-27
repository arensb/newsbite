<?php
/* Plugin to remove the annoying embedly frames that Cheezburger uses. Or at
 * least the YouTube ones.
 */

function deembedly(&$retval, $maxlen = NULL)
{
	# We're looking for:
	#    <div id='video-66153729'>
	#<iframe class="embedly-embed" src="//cdn.embedly.com/widgets/media.html?src=http%3A%2F%2Fwww.youtube.com%2Fembed%2FXDvMBPZ5Bik%3Ffeature%3Doembed&url=http%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3DXDvMBPZ5Bik&image=http%3A%2F%2Fi.ytimg.com%2Fvi%2FXDvMBPZ5Bik%2Fhqdefault.jpg&key=bf0f5443890c4e8887da35fba9b1523a&type=text%2Fhtml&schema=youtube" width="500" height="281" scrolling="no" frameborder="0" allowfullscreen></iframe>    </div>

	$retval = preg_replace('{<iframe[^>]*src="[^\">]*src=http%3A%2F%2Fwww.youtube.com%2Fembed%2F([^\">%]*).*</iframe>}',
		'<!--Hello world--><iframe src="https://www.youtube.com/embed/\1"></iframe>',
		$retval);
#print "now retval <pre>$retval</pre><br/>\n";
}

add_hook("clean-html", "deembedly");
