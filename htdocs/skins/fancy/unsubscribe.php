<?php
// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
$feed = &$skin_vars['feed'];

echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Unsubscribe</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/unsubscribe.css" media="all" />
</head>
<body id="unsubscribe-body">

<h1>Unsubscribe</h1>

<form name="unsubscribe-form" method="post" action="unsubscribe.php">
<input type="hidden" name="id" value="<?=$feed['id']?>"/>

<table id="show-feed">
  <tr>
    <th>Title</th>
    <td><?=$feed['title']?></td>
  </tr>

<?php if ($feed['nickname'] != ""):
?>
  <tr>
    <th>Nickname</th>
    <td><?=$feed['nickname']?>
  </tr>
<?php endif
?>

<?php if ($feed['description'] != ""):
	$description = $feed['description'];
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
    <td><span class="url"><a href="<?=$feed['url']?>"><?=$feed['url']?></a></span></td>
  </tr>

  <tr>
    <th>Feed URL</th>
    <td><span class="url"><a href="<?=$feed['feed_url']?>"><?=$feed['feed_url']?></a></span></td>
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
