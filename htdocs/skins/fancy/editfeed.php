<?php
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
$feeds = &$skin_vars['feed'];
$feed = &$skin_vars['feed'];
$groups = &$skin_vars['groups'];
$feed_opts = db_get_feed_options($feed['id']);

/* group_list
 * Print a tree of groups, each with a checkbox.
 */
function group_list($group)
{
	// Display an <li> for this group.
	// The name is "group_<gid>" (e.g., "group_-19").
	// If the current feed is a member of this group
	// ($group['marked'] is set), then the checkbox is marked.
	echo "<li>",
		"<input type=\"checkbox\" name=\"group_",
		$group['id'],
		"\"",
		($group['marked'] ? " checked" : ""),
		"/>",
		 htmlspecialchars($group['name']);

	// If this group has children, recursively call group_list()
	// to display their tree.
	if (isset($group['members']) && count($group['members']) > 0)
	{
		echo "<ul>";
		foreach ($group['members'] as $g)
		{
			if (isset($g['id']) && $g['id'] < 0)
				group_list($g);
		}
		echo "</ul>";
	}
	echo "</li>\n";
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Editing <?=htmlspecialchars($feed['title'])?></title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/editfeed.css" media="all" />
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

  <tr>
    <th>Title</th>
    <td><?=htmlspecialchars($feed['title'])?></td>
  </tr>

  <tr>
    <th>Subtitle</th>
    <td><?=$feed['subtitle'] ? htmlspecialchars($feed['subtitle']) : "&nbsp;"?></td>
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
   * auto-discover the feed URL from the site URL. Presumably the way
   * to do this is to fetch the site URL and check for "link
   * rel=alternate", where the MIME type is RSS or Atom.
   *
   * However, I'm not sure this can be done in JavaScript: we can't
   * just fetch an arbitrary URL.
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
    <th>Groups</th>
    <td>
<?
      if (isset($groups['members'])  && count($groups['members']) > 0)
      {
		echo "<ul>";
		foreach ($groups['members'] as $g)
			if ($g['id'] < 0)
				group_list($g);
		echo "</ul>";
      }
?>
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

  <tr>
    <th class="section-title" colspan="0">Options</th>
  </tr>
<?
    if (count($feed_opts) > 0):
	foreach ($feed_opts as $opt => $value):
?>
    <tr>
      <th><?=$opt?></th>
      <td><input type="number"
		name="opt_<?=$opt?>"
		value="<?=$feed_opts[$opt]?>" /></td>
    </tr>
<?
	  endforeach;
    endif;
?>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>

</body>
</html>
