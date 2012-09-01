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
<script type="text/javascript">
  /* XXX - Which of these variables are actually needed? */
  var skin_dir = "skins/<?=$skin_dir?>";
  var show_details = false;
  var show_tools = false;
  var feed_title_tmpl_text = '<a href="view.php?id=@id@">@display_title@</a>&nbsp;<span class="feed-details">(<a href="@url@">site</a>, <a href="@feed_url@">RSS</a>)</span>';
  var feed_tools_tmpl_text = '<a href="update.php?id=@id@" onclick="return update_feed(@id@)">update</a>&nbsp;<a href="editfeed.php?id=@id@">edit</a>&nbsp;<a href="unsubscribe.php?id=@id@">unsub</a> <img src="skins/fancy/Attraction_transfer_icon.gif"/>';
</script>
<script type="text/javascript" src="skins/<?=$skin_dir?>/feeds.js"></script>
<?
if ($skin_vars['mobile'] == "iPhone")
	echo '<meta name="viewport" content="width = device-width, initial-scale=0.8">', "\n";
elseif ($skin_vars['mobile'] == "Android")
	echo '<meta name="viewport" content="width = device-width, initial-scale=1.0" />', "\n";
?>
</head>
<body>
<ul class="msglist" id="feed-msglist">
</ul>

<h1>Feeds</h1>

<ul class="tools">
  <li><a href="update.php?id=all" onclick="return update_feed('all')">Update all feeds</a></li>
  <li><a href="view.php?id=all">View all feeds</a></li>
  <li class="submenu">
    <span class="submenu-title">Feeds</span>
    <ul class="tools-menu">
      <li><a href="addfeed.php">Add a feed</a></li>
      <li><a href="opml.php">Subscription list as OPML</a></li>
      <li><a href="loadopml.php">Add OPML file</a></li>
      <li><a href="setskin.php">Change skin</a></li>
    </ul>
  </li>
  <li class="submenu">
    <span class="submenu-title">Display</span>
    <ul class="tools-menu">
      <li><a onclick="javascript:toggle_details()">Toggle details</a></li>
      <li><a onclick="javascript:toggle_tools()">Toggle tools</a></li>
      <li><a onclick="javascript:toggle_show_empty()">Toggle empty feeds</a></li>
      <li><a onclick="javascript:toggle_show_inactive()">Toggle inactive feeds</a></li>
      <li><a onclick="javascript:toggle_show_stale()">Toggle stale feeds</a></li>
    </ul>
  </li>
</ul>

<table id="feeds" class="hide-details hide-tools">
<thead>
  <th class="icon-col">&nbsp;<!-- status indicator --></th>
  <th class="count-col">#</th>
  <th class="title-col">Title</th>
  <th class="feed-tools">Tools</th>
</thead>
<tbody id="feeds-tbody">
  <td colspan="3"><img src="skins/<?=$skin_dir?>/Ajax-loader.gif"/></td>
</tbody>
</table>
</body>
</html>
