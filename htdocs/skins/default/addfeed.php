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
<body id="add-feed">

<h1>Adding feed</h1>

<form name="add-feed-form" method="post" action="addfeed.php">

<table id="add-feed">
  <tr>
    <th>Feed URL</th>
    <td>
<? if (isset($skin_vars['errors']['feed_url'])): ?>
        <div class="error-msg"><?=$skin_vars['errors']['feed_url']?></div>
<? endif ?>
      <input type="text" name="feed_url" value="<?=$skin_vars['feed']['feed_url']?>"/>
    </td>
  </tr>
<?
  /* XXX - Ought to manage passwords separately, so can have one
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
