<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: {$feed.title}</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
</head>
<body id="view-body">

{if (isset($feed.image))}
<img class="feed_icon" src="{$feed.image}"/>
{/if}

{strip}
<h1>
  {if ($feed.url == "")}
    {$feed.title}
  {else}
    <a href="{$feed.url}">{$feed.title}</a>
  {/if}
</h1>
{if $feed.subtitle != ""}
<div class="feed_subtitle">{$feed.subtitle}</div>
{/if}
{/strip}

{if $feed.description != ""}
<div class="feed_description">{$feed.description}</div>
{/if}

<!-- List of items -->
{if (count($items) > 0)}
<form name="mark-items" method="post" action="markitems.php">
<input type="reset" name="clearit" value="Clear changes"/>
<input type="submit" name="doit" value="Apply changes"/>

{section name=i loop=$items}
{strip}
<div class="item" id="item_{$items[i].guid}" style="border: 1px solid black">
  <div class="item_header">
    <a class="item_title" href="{$items[i].url}">
      {if ($items[i].title == "")}
        [no title]
      {else}
        {$items[i].title}
      {/if}
    </a><br/>
    {if isset($items[i].author)}
      by {$items[i].author}<br/>
    {/if}

    {if ($items[i].category != "")}
{* XXX - Do something with categories *}
{* category: [{$items[i].category}]<br/>*}
{* XXX - There should be a box of icons, like /. categories *}
    {/if}
{* guid: [{$items[i].guid}]<br/> *}
    {$items[i].pub_date|date_format:"%c"}
    &nbsp; (updated {$items[i].last_update|date_format:"%c"})
{* XXX - Do something with the state *}
    &nbsp; state: [{$items[i].state}]
{* Note that there are two groups of radio buttons per item: one group
 * at the top, and another at the bottom. These all have the same
 * radio group name: "state_{id}". Otherwise, confusion can arise if
 * the item is marked as read at the top, and unread at the bottom.
 * The values are "na", "ua", "ra" at the top (for new, unread, read)
 * and "nb", "ub", and "rb" at the bottom. The "a" and "b" are just
 * there because w3.org says that all the radio buttons in a group
 * should have different values.
 *}
    <br/>
    (New: <input type="radio" name="state_{$items[i].id}" value="na" />
     Unread: <input type="radio" name="state_{$items[i].id}" value="ua" />
     Read:<input type="radio" name="state_{$items[i].id}" value="ra" />
    )
    <br/>
  </div>

{* XXX - If JavaScript is turned on, should have selectable tabs for the
 * summary and full content.
 *}
  {if ($items[i].summary != "")}
{*    <h5>Summary:</h5>*}
    <div class="item_summary">
      {$items[i].summary}

      {* This is for items with floating elements in them (such as
       * tall images): make sure the image is contained within the
       * <div> and doesn't go overflowing where we don't want it.
       *}
      <br style="clear: both"/>
    </div>
  {/if}

  {if ($items[i].content != "")}
{*    <h5>Content:</h5>*}
    <div class="item_content">{$items[i].content}</div>
  {/if}

  <div class="item_footer">
    {if (isset($items[i].comment_url))}
      <a href="{$items[i].comment_url}">Comments</a>
      {if (isset($items[i].comment_rss))}
        &nbsp;
        <a href="{$items[i].comment_rss}">(feed)</a>
      {/if}
      <br/>
    {/if}
{* XXX - Control buttons to mark as read and whatnot. *}
    &nbsp;
    (New: <input type="radio" name="state_{$items[i].id}" value="nb" />
     Unread: <input type="radio" name="state_{$items[i].id}" value="ub" />
     Read:<input type="radio" name="state_{$items[i].id}" value="rb" />
    )
  </div>
</div>
{/strip}
{/section}

<input type="reset" name="clearit" value="Clear changes"/>
<input type="submit" name="doit" value="Apply changes"/>
</form>
{else}
{* XXX - Would it be worth having a separate template for an empty feed? *}
<p>There are no articles to display.</p>
{/if}

</body>
</html>
