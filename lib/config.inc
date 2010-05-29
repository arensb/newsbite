<?
/* Database access settings */
$DB_HOST = "localhost";		// Host on which database resides
$DB_NAME = "newsbite";		// Database name
$DB_USER = "newsbiteuser";	// Database user name
$DB_PASS = "newsbitepass";	// Database user's password

/* Age in days when articles are automatically deleted */
$AUTODELETE_DAYS = 90;

/* How old the latest post must be, in days, before the feed is
 * considered stale.
 */
$STALE_DAYS = 90;

/* How many feeds to update in parallel. It looks as though the total
 * time goes down until about 10 or so, and then there's a point of
 * diminishing returns. So probably best to leave this at 10.
 * Of course, if you have hundreds or thousands of feeds, it might be
 * worth increasing this.
 */
$PARALLEL_UPDATES = 20;

/* If FEED_CACHE is set and refers to an existing directory,
 * update.php will save a copy of the raw feed to "FEED_CACHE/$id", to
 * make it easier to debug.
 * To turn this feature on, uncomment the following line and set it to
 * a directory you like. The directory must be writable by the httpd
 * user.
 */
//define('FEED_CACHE', "/tmp/feed-cache");

/* SMARTY_DIR: where the Smarty classes are (i.e., the directory
 * containing Smarty.class.php). Must end in slash.
 */
define('SMARTY_DIR', "/path/to/Smarty/");

/* SMARTY_PATH: where to find smarty directories: cache and compiled
 * templates.
 * May be relative to the htdocs directory, or an absolute path.
 * Must end with slash.
 */
define('SMARTY_PATH', "smarty/");	// Relative to htdocs/

/* Default skin. This is the name of a directory in htdocs/skins */
define('DEFAULT_SKIN', "default");

/* Plugin directory. All *.php files in this directory will be loaded
 * by modules that take plugins or hooks.
 */
define('PLUGIN_DIR', "/path/to/plugins/");

/* Server secret password. This is only used to generate a checksum
 * for the authentication cookie. It is never used by humans, and is
 * never sent to the user. Use a long (>= 64-char?) random string.
 */
define('SERVER_SECRET', "very-secret-string");

/* It is recommended that you let the web server authenticate you. If
 * you're unwilling or unable to do this, however, you can set the
 * username and password below.
 */
define('USERNAME', 'theuser');
define('PASSWORD', 'thepass');

/* Authorization cookie duration, in seconds: how long an
 * authorization cookie lasts. The longer this value, the less often
 * you have to re-log in, but the more vulnerable you are to replay
 * attacks.
 *
 * Set to one week by default.
 */
define('AUTH_COOKIE_DURATION', 87600*7);
?>
