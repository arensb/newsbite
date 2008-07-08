<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Feeds</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
<link rel="stylesheet" type="text/css" href="index.css" media="all" />
</head>
<body>

<h1>Feeds</h1>

<div class="tools">
  <a href="update.php?id=all">Update all feeds</a>
</div>

{* Link to show all feeds *}
<p><a href="view.php?id=all">View all feeds</a></p>

<table id="feeds">
{foreach from=$feeds item=feed}
{* Feed ID gets used a lot, so it gets its own variable *}
{assign var="feed_id" value=$feed.id}
{strip}
  <tr class="{cycle values="odd-row,even-row"}" id="feed-{$feed_id}">
    <td class="icon-col">&nbsp;</td>{* XXX - Put refresh/status icons here *}
    <td>
      {* Feed title *}
      <a href="view.php?id={$feed_id}">
        {if $feed.nickname != ""}
          {$feed.nickname}
        {elseif $feed.title != ""}
          {$feed.title}
        {else}
          [no&nbsp;title]
        {/if}
      </a>:
      &nbsp;
      {* Number of new/unread/read items in the feed *}
      {* XXX - These should be links, to show only new items,
       * new and unread items, or all items.
       *}
      {$counts[$feed_id].new} new /
      {$counts[$feed_id].unread || 0} unread /
      {$counts[$feed_id].read} read
      <br/>
      {* Links to places related to the feed *}
      &nbsp;(<a href="{$feed.url}">site</a>)
      &nbsp;(<a href="{$feed.feed_url}">RSS</a>)
    </td>
    <td>
      {* Tools *}
      <a href="update.php?id={$feed.id}">update</a>
      &nbsp;
      <a href="editfeed.php?id={$feed.id}">edit</a>
    </td>
  </tr>
{/strip}
{/foreach}
</table>

</body>
</html>
