<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Feeds</title>
<link rel="stylesheet" type="text/css" href="skins/{$skin}/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/{$skin}/index.css" media="all" />
<!-- If JavaScript is turned on, slurp in the JavaScript-specific
     stylesheet
-->
<script type="text/javascript">
  document.write('<link rel="stylesheet" type="text/css" href="skins/{$skin}/style-js.css" media="all" />\n');
</script>
<!-- If JavaScript is turned off, slurp in the no-JavaScript-specific
     stylesheet
-->
<noscript>
  <link rel="stylesheet" type="text/css" href="skins/{$skin}/style-nojs.css" media="all" />
</noscript>
<script type="text/javascript">
  var skin_dir="skins/{$skin}";
</script>
<script type="text/javascript" src="skins/{$skin}/feeds.js"></script>
</head>
<body>

<h1>Feeds</h1>

{strip}
<ul class="tools">
  <li><a href="update.php?id=all" onclick="return update_feed('all')">Update all feeds</a></li>
  <li><a href="view.php?id=all">View all feeds</a></li>
  <li><a href="addfeed.php">Add a feed</a></li>
  <li><a href="opml.php">Subscription list as OPML</a></li>
  <li><a href="loadopml.php">Add OPML file</a></li>
  <li><a href="setskin.php">Change skin</a></li>
</ul>
{/strip}

<table id="feeds">
{foreach from=$feeds item=feed}
{* Feed ID gets used a lot, so it gets its own variable *}
{assign var="feed_id" value=$feed.id}
{strip}
  <tr class="{cycle values="odd-row,even-row"}
  {* NB: careful with whitespace in the class list: *}
  {if !$feed.active} inactive-feed{/if}" id="feed-{$feed_id}">
    <td class="icon-col">&nbsp;</td>{* Refresh/status icons go here *}
    <td>{include file='feed-title.tpl' feed=$feed counts=$counts[$feed_id]}</td>
    <td>
      {* Tools *}
      <a href="update.php?id={$feed.id}" onclick="return update_feed({$feed.id})">update</a>
      &nbsp;
      <a href="editfeed.php?id={$feed.id}">edit</a>
      &nbsp;
      <a href="unsubscribe.php?id={$feed.id}">unsub</a>
    </td>
  </tr>
{/strip}
{/foreach}
</table>

</body>
</html>
