<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Feeds</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
</head>
<body>

<h1>Feeds</h1>

<div class="tools">
  <a href="update.php?id=all">Update all feeds</a>
</div>

{* Link to show all feeds *}
<p><a href="view.php?id=all">View all feeds</a></p>

<table id="feeds">
{section name=feed loop=$feeds start=1}
{assign var="feed_id" value=$feeds[feed].id}
{strip}
  <tr class="{cycle values="odd-row,even-row"}">
    <td class="icon-col">&nbsp;</td>{* XXX - Put refresh/status icons here *}
    <td>
      <a href="view.php?id={$feed_id}">{$feeds[feed].title}</a>:
      &nbsp;
      {* XXX - These should be links, to show only new items,
       * new and unread items, or all items.
       *}
      {$counts[$feed_id].new} new /
      {$counts[$feed_id].unread || 0} unread /
      {$counts[$feed_id].read} read
      <br/>
      &nbsp;(<a href="{$feeds[feed].url}">site</a>)
      &nbsp;(<a href="{$feeds[feed].feed_url}">RSS</a>)
    </td>
    <td>
      {* Tools *}
      <a href="update.php?id={$feeds[feed].id}">update</a>
    </td>
  </tr>
{/strip}
{/section}
</table>

</body>
</html>
