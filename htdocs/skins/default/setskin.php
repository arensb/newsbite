<?
/* XXX - Should probably have separate template for page top (and
 * possibly page bottom). Should include all the CSS magic.
 */

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
$feed = &$skin_vars['feed'];

echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: <?=$feed['title']?></title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/setskin.css" media="all" />
</head>
<body id="setskin-body">

<div class="notice">
Cookies must be turned on for skins to work.
</div>

<form method="post" action="setskin.php">
  Choose a skin:
  <select name="newskin">
<?
foreach ($skin_vars['skins'] as $s)
{
	echo "<option";
	if ($s['dir'] == $skin_vars['current_skin'])
		echo " selected";
	echo " value=\"$s[dir]\">";
	echo htmlspecialchars($s['name']);
	echo "</option>";
}
?>
  </select>
  <br/>
  <input type="submit" name="set-skin" value="Set Skin"/>
</form>
<?
/* XXX - Might be nice to have an iframe with a sample document,
 * so can preview the skin.
 */
?>
</body>
</html>
