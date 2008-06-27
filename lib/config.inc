<?
/* Database access settings */
$DB_HOST = "localhost";
$DB_NAME = "newsbite";
$DB_USER = "newsbite";
$DB_PASS = "newsbitepass";

/* Age in days when articles are automatically deleted */
$AUTODELETE_DAYS = 90;

/* How many feeds to update in parallel. It looks as though the total
 * time goes down until about 10 or so, and then there's a point of
 * diminishing returns. So probably best to leave this at 10.
 * Of course, if you have hundreds or thousands of feeds, it might be
 * worth increasing this.
 */
$PARALLEL_UPDATES = 10;

/* If FEED_CACHE is set and refers to an existing directory,
 * update.php will save a copy of the raw feed to "FEED_CACHE/$id", to
 * make it easier to debug.
 * To turn this feature off, comment out the following line.
 */
define('FEED_CACHE', "/tmp/feed-cache");

define('SMARTY_DIR', "/folks/arensb/proj/newsbite/trunk/Smarty/Smarty/");
define('SMARTY_PATH', "smarty/");
?>
