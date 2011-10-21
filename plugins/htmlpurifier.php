<?
/* Plugin to use HTMLpurifier to clean up HTML.
 * See http://www.htmlpurifier.org/
 */
/* To enable this plugin, install HTMLPurifier, then add a line like
 * the following to your config.php to say where the class files are:
 *
 * define(HTMLPURIFIER_LIB, "/path/to/htmlpurifier/library");
 *
 * This directory should be the one that contains the file
 * "HTMLPurifier.auto.php".
 */
//define(HTMLPURIFIER_LIB, "/folks/arensb/proj/newsbite/HTMLPurifier/htmlpurifier-4.1.1-lite/library");

# XXX - Should accept a base URL with which to expand relative URLs.
# See %URI.Base and %URI.MakeAbsolute.

if (!defined('HTMLPURIFIER_LIB'))
	# We don't have HTMLPurifier. Abort.
	return;

# If we get this far, we have HTMLPurifier. So include it
require_once HTMLPURIFIER_LIB . "/HTMLPurifier.auto.php";

$purifier = NULL;	# Singleton purifier
$purifier_config = NULL;

# htmlpurify_init
# Set up the singleton purifier
function htmlpurify_init()
{
	global $purifier;
	global $purifier_config;

	$purifier_config = HTMLPurifier_Config::createDefault();

	# No cache directory (less efficient):
	$purifier_config->set('Cache.DefinitionImpl', NULL);
	# Cache directory writable by httpd user:
#	$purifier_config->set('Cache.SerializerPath', '/path/to/cache/dir');

	# Allow YouTube videos
	$purifier_config->set('Filter.YouTube', TRUE);

	# XXX - Specify configuration options
	$purifier = new HTMLPurifier($purifier_config);
}

function htmlpurify(&$retval)
{
	global $purifier;
	if (!is_string($retval))
		return;

	if (!isset($purifier))
		htmlpurify_init();

	$newretval = $purifier->purify($retval);
#if ($newretval != $retval)
#{
#echo "Old HTML: [<pre>", htmlspecialchars($retval), "</pre>]<br/>\n";
#echo "New HTML: [<pre>", htmlspecialchars($newretval), "</pre>]<br/>\n";
#}
	$retval = $newretval;
}

add_hook("clean-html", "htmlpurify");

?>
