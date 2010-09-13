<?php
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
$feeds = &$skin_vars['feed'];
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Feeds</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/index.css" media="all" />
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
<body>

<h1>Feeds</h1>

<ul class="tools">
  <li><a href="update.php?id=all">Update all feeds</a></li>
  <li><a href="view.php?id=all">View all feeds</a></li>
  <li><a href="addfeed.php">Add a feed</a></li>
  <li><a href="opml.php">Subscription list as OPML</a></li>
  <li><a href="loadopml.php">Add OPML file</a></li>
  <li><a href="setskin.php">Change skin</a></li>
</ul>

<table id="feeds">
<?
for ($i = 0; $i < count($skin_vars['feeds']); $i++)
{
	$feed = $skin_vars['feeds'][$i];
	$feed_id = $feed['id'];
	echo "<tr class=\"",
		($i & 1 ? "odd-row" : "even-row"),
		"\" id=\"feed-${feed_id}\">";
	echo "<td class=\"icon-col\">&nbsp;</td>";
	echo "<td>";
	$the_skin->_include("feed-title",
			    array("feed" =>	$feed,
				  "counts" =>	$skin_vars['counts'][$feed_id]
				    )
		);
?>
    <td>
<?    /* Tools */ ?>
<a href="update.php?id=<?=$feed_id?>">update</a>&nbsp;<a href="editfeed.php?id=<?=$feed_id?>">edit</a>&nbsp;<a href="unsubscribe.php?id=<?=$feed_id?>">unsub</a>
</td>
  </tr>
<?
}
?>
</table>

</body>
</html>
