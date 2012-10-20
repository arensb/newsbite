<?php
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Adding feed</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/addfeed.css" media="all" />
<script type="text/javascript">
// Function to add NewsBite as an RSS subscriber in Firefox
var ff_subscribe_url = "<?=$skin_vars['subscribe_url']?>?feed_url=%s";	// Subscription URL

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

<p>Bookmarklet: <a href="javascript:void(location.href='<?=$skin_vars['subscribe_url']?>?page_url='+escape(location))">Subscribe in NewsBite</a>.</p>

<form name="add-feed-form" method="post" action="addfeed.php">

<table id="add-feed">
<?
/* If we've been given a list of URLs (presumably extracted from a
 * page by addfeed.php), display that list.
 */
if (isset($skin_vars['feed_list'])):
?>
  <tr>
    <th id="th-feed-list">Pick a URL</th>
    <td>
      <ul class="feed-list">
<?
	$is_default = true;
	foreach ($skin_vars['feed_list'] as $f)
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
<? else:	/* No list of URLs given. Display a text entry field */ ?>
  <tr>
    <th>Feed URL</th>
    <td>
<? if (isset($skin_vars['errors']['feed_url'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['feed_url']?></div>
<? endif ?>
      <input type="text" name="feed_url" value="<?=$skin_vars['feed']['feed_url']?>"/>
    </td>
  </tr>
<? endif ?>
  <tr>
    <th>Username</th>
    <td>
<? if (isset($skin_vars['errors']['username'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['username']?></div>
<? endif ?>
      <input type="text" name="username" value="<?=$skin_vars['feed']['username']?>"/>
    </td>
  </tr>

  <tr>
    <th>Password</th>
    <td>
<? if (isset($skin_vars['errors']['passwd'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['passwd']?></div>
<? endif ?>
      <input type="password" name="password" value="<?=$skin_vars['feed']['passwd']?>"/>
    </td>
  </tr>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>
</body>
</html>
