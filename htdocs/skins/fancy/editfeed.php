<?php
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
$feeds = &$skin_vars['feed'];
$feed = &$skin_vars['feed'];
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Editing <?=htmlspecialchars($feed['title'])?></title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/editfeed.css" media="all" />
<!-- If JavaScript is turned on, slurp in the JavaScript-specific
     stylesheet
-->
<script type="text/javascript">
  document.write('<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style-js.css" media="all" />\n');
</script>
<!-- If JavaScript is turned off, slurp in the no-JavaScript-specific
     stylesheet
-->
<noscript>
  <link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style-nojs.css" media="all" />
</noscript>
</head>
<body id="edit-feed">

<? /* XXX - Links to get back to interesting places, like feed list */ ?>
<h1>Editing feed <?=htmlspecialchars($feed['title'])?></h1>

<form name="edit-feed" method="post" action="editfeed.php">
<?/* Feed ID */?>
<input type="hidden" name="id" value="<?=$feed['id']?>"/>
<input type="hidden" name="command" value="<?=$skin_vars['command']?>"/>

<table id="show-feed">
<?/* XXX - Is it worth displaying the feed ID? */ ?>
  <tr>
    <th>ID</th>
    <td><?=$feed['id']?></td>
  </tr>

<?/* XXX - Is it worth displaying the title? It's right above */?>
  <tr>
    <th>Title</th>
    <td><?=htmlspecialchars($feed['title'])?></td>
  </tr>

  <tr>
    <th>Subtitle</th>
    <td><?=htmlspecialchars($feed['subtitle']) || "&nbsp;"?></td>
  </tr>

<?/* User-settable nickname */ ?>
  <tr>
    <th>Nickname</th>
    <td>
<? if (isset($skin_vars['errors']['nickname'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['nickname']?></div>
<? endif ?>
      <input type="text" name="nickname" value="<?=$feed['nickname']?>"/>
    </td>
  </tr>

<?/* XXX - There should be a button or something to try to
   * auto-discover the feed URL from the site URL.
   */
?>
  <tr>
    <th>Site URL</th>
    <td>
<? if (isset($skin_vars['errors']['url'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['url']?></div>
<? endif ?>
      <input type="text" name="url" value="<?=$feed['url']?>"/>
    </td>
  </tr>

  <tr>
    <th>Feed URL</th>
    <td>
<?    if (isset($skin_vars['errors']['feed_url'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['feed_url']?></div>
<? endif ?>
      <input type="text" name="feed_url" value="<?=$feed['feed_url']?>"/>
    </td>
  </tr>

  <tr>
    <th>Description</th>
    <td>
      <div><?
	# Sanitize description before displaying it.
	$description = $feed['description'];
	run_hooks("clean-html", array(&$description));
	echo $description;
?></div>
    </td>
  </tr>

<?/* XXX - Probably not worth displaying this */ ?>
  <tr>
    <th>Last update</th>
    <td><?=$feed['last_update']?></td>
  </tr>

<?/* XXX - Need better way of saying "don't update this more than once
   * a day" or "don't update except on Tuesdays".
   */
?>
  <tr>
    <th>TTL</th>
    <td><?=$feed['ttl']?></td>
  </tr>

  <tr>
    <th>Image</th>
    <td>
<?    if (isset($feed['image'])): ?>
        <img src="<?=$feed['image']?>"/>
<? else: ?>
        No image.
<? endif ?>
    </td>
  </tr>

  <tr>
    <th>Active</th>
    <td>
      <input type="checkbox" name="active"
	<? if ($feed['active']) echo ' checked="checked"' ?>
<?/*        {if $feed.active}
          checked
        {/if}
*/?>
      />
    </td>
  </tr>

<?/* XXX - Ought to manage passwords separately, so can have one
   * username/password for all of livejournal.com.
   *
   * There's also the problem that Firefox stores passwords for sites
   * and fills them in automatically in pages. So this can fill in the
   * wrong password.
   */
?>
  <tr>
    <th>Username</th>
    <td>
<?    if (isset($skin_vars['errors']['username'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['username']?></div>
<? endif ?>
      <input type="text" name="username" value="<?=$feed['username']?>" autocomplete="off"/>
    </td>
  </tr>

  <tr>
    <th>Password</th>
    <td>
<?    if (isset($skin_vars['errors']['passwd'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['passwd']?></div>
<? endif ?>
      <input type="password" name="password" value="<?=$feed['passwd']?>" autocomplete="off"/>
    </td>
  </tr>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>

</body>
</html>
