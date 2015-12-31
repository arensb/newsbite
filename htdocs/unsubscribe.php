<?php
/* unsubscribe.php
 * Remove a feed.
 */
// XXX - Should be possible to unsubscribe from a feed, but keep it in
// the database for later. This can be good for subscribing to
// political feeds only during election season, or sports feeds only
// during playoffs, that sort of thing.
require_once("common.inc");
require_once("database.inc");
require_once("hooks.inc");

load_hooks(PLUGIN_DIR);

$feed_id = $_REQUEST['id'];		// ID of feed to show
/* Make sure $feed_id is an integer */
if (is_numeric($feed_id) && is_integer($feed_id+0))
	$feed_id = (int) $feed_id;
else
	abort("Invalid feed ID: $feed_id");

/* Has confirmation been given? */
$confirm = $_REQUEST['confirm'];
//echo "confirm == [$confirm]<br/>\n";
if ($confirm == "yes")
{
	/* Go ahead and unsubscribe */
	db_delete_feed($feed_id);
		// XXX - Error-checking

	/* Redirect back to the main page */
	redirect_to("index.php");
	exit(0);
}

/* Confirmation has not been given. Show feed info and ask for
 * confirmation.
 */
// We've already established above that $feed_id is numeric
$feed_info = db_get_feed($feed_id);
if ($feed_info === NULL)
	/* No such feed. Abort */
	abort("No such feed: $feed_id.");

	// What follows is basically a template.
########################################
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Unsubscribe</title>
<link rel="stylesheet" type="text/css" href="css/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="css/unsubscribe.css" media="all" />
</head>
<body id="unsubscribe-body">

<h1>Unsubscribe</h1>

<form name="unsubscribe-form" method="post" action="unsubscribe.php">
<input type="hidden" name="id" value="<?=$feed_info['id']?>"/>

<table id="show-feed">
  <tr>
    <th>Title</th>
    <td><?=$feed_info['title']?></td>
  </tr>

<?php if ($feed_info['nickname'] != ""):
?>
  <tr>
    <th>Nickname</th>
    <td><?=$feed_info['nickname']?>
  </tr>
<?php endif
?>

<?php if ($feed_info['description'] != ""):
	$description = $feed_info['description'];
	run_hooks("clean-html", array(&$description));
?>
  <tr>
    <th>Description</th>
    <td><?=$description?></td>
  </tr>
<?php endif
?>

  <tr>
    <th>Site URL</th>
    <td><span class="url"><a href="<?=$feed_info['url']?>"><?=$feed_info['url']?></a></span></td>
  </tr>

  <tr>
    <th>Feed URL</th>
    <td><span class="url"><a href="<?=$feed_info['feed_url']?>"><?=$feed_info['feed_url']?></a></span></td>
  </tr>

  <tr>
    <td colspan="2">
      Check here if you really want to unsubscribe:&nbsp;
      <input type="checkbox" name="confirm" value="yes"/>
    </td>
  </tr>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="unsub" value="Unsubscribe"/>
</form>

</body>
</html>
