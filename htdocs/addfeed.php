<?php
/* addfeed.php
 * Add a feed.
 */
// XXX - Should accept OPML file.
// XXX - Split this up into HTML form vs. REST

$out_fmt = "html";		// Mandatory output format
require_once("common.inc");
require_once("database.inc");
require_once("net.inc");

$feed_url = $_REQUEST['feed_url'];
	// XXX - Probably needs to be escaped. Can there be quotes in URLs?
$page_url = $_REQUEST['page_url'];
	// URL of page whose feed to subscribe to.

/* If we were given the URL to a content page, rather than directly to
 * the feed, download the page and look for links to RSS feeds.
 */
if (isset($page_url))
{
	global $feeds;		// Array of feeds found in this page
	$feeds = Array();

	// Read the page
	@$page = file_get_contents($page_url);
	if ($page === false)
	{
		// XXX - Better error-reporting
		// error_get_last()['message'] arguably gives too much
		// information, so it'd be nice to pare it down to
		// just what the user needs to know.
		// ['type'] is a numeric error message, but for both
		// "no such file" and "authorization required", it has
		// value 2. So not very useful.
		$errors = error_get_last();
		$errmsg = $errors['message'];
		echo "Error: ", $errmsg, "<br/>\n";
		exit(0);
	}

	// Parse as XML
	$dom = new DOMDocument();
	@$dom->loadHTML($page);
		// Tends to return lots of warnings on bad HTML.
		// Suppress this output.
	$head = $dom->getElementsByTagName("head");
	if ($head)
		$head = $head->item(0);
	$links = $head->getElementsByTagName("link");

	foreach ($links as $link)
	{
		$rel = $link->getAttribute("rel");
		if ($rel != "alternate")
			continue;
		$type = $link->getAttribute("type");
		$title = $link->getAttribute("title");
		$href = $link->getAttribute("href");

		// Append this feed to the list
		$feeds[] = Array("type" => $type,
				 "title" => $title,
				 "url" => $href);

	}

	if (count($feeds) == 0)
	{
		/* If there aren't any feed links, put up an error
		 * message to that effect.
		 */
		abort("Couldn't find any RSS links in that page.");
	} elseif (count($feeds) == 1)
	{
		/* Exactly one RSS feed link. Assume that that's what
		 * the user wants to subscribe to.
		 */
		$feed_url = $feeds[0]['url'];
	}
}

/* If we were given a single feed_url, subscribe to it, and refresh it
 * immediately.
 */
if (isset($feed_url))
{
	$params = Array();

	$params['feed_url'] = $feed_url;
	if (isset($_REQUEST['username']))
		$params['username'] = $_REQUEST['username'];
	if (isset($_REQUEST['password']) &&
	    $_REQUEST['password'] != "")
		$params['passwd'] = $_REQUEST['password'];

	// XXX - Check whether we're already subscribed to that URL

	$feed_id = db_add_feed($params);
	if ($feed_id === false)
		abort("Error adding feed.");

	/* Refresh the new feed, to get info and new articles */
	$err = update_feed($feed_id);
	if (!$err)
		// XXX - Better error reporting: include error message
		abort("Error updating new feed.");
	if (isset($err['status']) && $err['status'] != 0)
		abort($err['errmsg']);

	/* Redirect to the feed's page */
	redirect_to("view.php#id=$feed_id");
	exit(0);
}

/* Construct the URL for subscribing to a feed (i.e., the URL of this
 * script), so we can pass it to JavaScript magic.
 */
$subscribe_url = "http://";
if ($_SERVER['SERVER_NAME'] != "")
	$subscribe_url .= $_SERVER['SERVER_NAME'];
else
	$subscribe_url .= $_SERVER['SERVER_ADDR'];
if ($_SERVER['SERVER_PORT'] != "" && $_SERVER['SERVER_PORT'] != 80)
	$subscribe_url .= ":$_SERVER[SERVER_PORT]";
$subscribe_url .= $_SERVER['SCRIPT_NAME'];

/* Display a form for adding a URL */
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Adding feed</title>
<link rel="stylesheet" type="text/css" href="css/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="css/addfeed.css" media="all" />
<script type="text/javascript">
// Function to add NewsBite as an RSS subscriber in Firefox
var ff_subscribe_url = "<?=$subscribe_url?>?feed_url=%s";	// Subscription URL

// register_feed_reader
// Function to add 'ff_subscribe_url' as a subscription URL in Firefox
function register_feed_reader()
{
	navigator.registerContentHandler(
		"application/vnd.mozilla.maybe.feed",
		ff_subscribe_url,
		"NewsBite");
}
</script>
</head>
<body id="add-feed">

<h1>Adding feed</h1>

<script type="text/javascript">
// If this is Firefox, put in a link to add NewsBite as an RSS subscriber.
if (navigator.registerContentHandler)
{
	var sub_button = document.createElement("button");

	sub_button.type = "button";
	sub_button.onclick = register_feed_reader;
	sub_button.innerHTML = "Add one-click subscription";
	document.body.appendChild(sub_button);
}
</script>

<p>Bookmarklet: <a href="javascript:void(location.href='<?=$subscribe_url?>?page_url='+escape(location))">Subscribe in NewsBite</a>.</p>

<form name="add-feed-form" method="post" action="addfeed.php">

<table id="add-feed">
<?php
/* If we've been given a list of URLs (presumably extracted from a
 * page by addfeed.php), display that list.
 */
if (isset($feeds)):
?>
  <tr>
    <th id="th-feed-list">Pick a URL</th>
    <td>
      <ul class="feed-list">
<?php
	$is_default = true;
	foreach ($feeds as $f)
	{
		echo "<li><input type=\"radio\" name=\"feed_url\" value=\"$f[url]\"";
		// Check the first item by default
		if ($is_default)
			echo ' checked="checked" ';
		$is_default = false;	// Subsequent items are not checked
		echo "/>";
		echo "<a href=\"$f[url]\">$f[title]</a> ($f[type])";
		echo "</li>";
	}
?>
      </ul>
    </td>
  </tr>
<?php else:	/* No list of URLs given. Display a text entry field */ ?>
  <tr>
    <th>Feed URL</th>
    <td>
<?php if (isset($errors['feed_url'])): ?>
        <div class="error-msg"><?=$errors['feed_url']?></div>
<?php endif ?>
      <input type="text" name="feed_url" value="<?=$params['feed_url']?>"/>
    </td>
  </tr>
<?php endif ?>
  <tr>
    <th>Username</th>
    <td>
<?php if (isset($errors['username'])): ?>
        <div class="error-msg"><?=$errors['username']?></div>
<?php endif ?>
      <input type="text" name="username" value="<?=$params['username']?>"/>
    </td>
  </tr>

  <tr>
    <th>Password</th>
    <td>
<?php if (isset($errors['passwd'])): ?>
        <div class="error-msg"><?=$errors['passwd']?></div>
<?php endif ?>
      <input type="password" name="password" value="<?=$params['passwd']?>"/>
    </td>
  </tr>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>
</body>
</html>
