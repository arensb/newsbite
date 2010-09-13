<?php
echo "<", '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Feeds</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/index.css" media="all" />
<?
/* Include a mobile device-specific stylesheet. */
switch ($skin_vars['mobile'])
{
    case 'iPhone':
	$mobile_css = "iphone.css";
	break;
    case 'iPad':
	$mobile_css = "ipad.css";
	break;
    default:
	break;
}
if (isset($mobile_css))
{
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"skins/$skin_vars[skin]/$mobile_css\" media=\"screen\" />\n";
}
?>	  
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
<script type="text/javascript">
  var skin_dir="skins/<?=$skin_dir?>";
  var show_details = false;
  var show_tools = false;
</script>
<script type="text/javascript" src="skins/<?=$skin_dir?>/feeds.js"></script>
<?
if ($skin_vars['mobile'] == "iPhone")
	echo '<meta name="viewport" content="width = device-width, initial-scale=0.8">', "\n";
?>
</head>
<body>

<h1>Feeds</h1>

<ul class="tools">
  <li><a href="update.php?id=all" onclick="return update_feed('all')">Update all feeds</a></li>
  <li><a href="view.php?id=all">View all feeds</a></li>
  <li><a href="addfeed.php">Add a feed</a></li>
  <li><a href="opml.php">Subscription list as OPML</a></li>
  <li><a href="loadopml.php">Add OPML file</a></li>
  <li><a href="setskin.php">Change skin</a></li>
  <li><a onclick="javascript:toggle_details()">Toggle details</a></li>
  <li><a onclick="javascript:toggle_tools()">Toggle tools</a></li>
</ul>

<table id="feeds" class="hide-details hide-tools">
<?
for ($i = 0; $i < count($skin_vars['feeds']); $i++)
{
	$feed = $skin_vars['feeds'][$i];
	$feed_id = $feed['id'];

#echo "i: [$i]<br/>\n";
	echo "<tr class=\"",
		($i & 1 ? "odd-row" : "even-row"),
		($feed['active'] ? "" : " inactive-feed"),
		($feed['stale'] ? " stale-feed" : ""),
		"\" id=\"feed-${feed_id}\">";
	echo "<td class=\"icon-col\">&nbsp;</td>";
	echo "<td class=\"title-col\">";
	$the_skin->_include("feed-title",
			    array("feed" =>	$feed,
				  "counts" =>	$skin_vars['counts'][$feed_id]
				    )
		);
?>
<td class="feed-tools">
<a href="update.php?id=<?=$feed_id?>" onclick="return update_feed(<?=$feed_id?>)">update</a>&nbsp;<a href="editfeed.php?id=<?=$feed_id?>">edit</a>&nbsp;<a href="unsubscribe.php?id=<?=$feed['id']?>">unsub</a>
</td>
</tr>
<?
}
?>
</table>
</body>
</html>
