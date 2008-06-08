<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: {$feed.title}</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
</head>
<body>

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
{/strip}

{if $feed.description != ""}
<div class="feed_description">{$feed.description}</div>
{/if}

<!-- List of items -->
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
    {/if}
{* guid: [{$items[i].guid}]<br/> *}
    {$items[i].pub_date|date_format:"%c"}
    &nbsp; (updated {$items[i].last_update|date_format:"%c"})
    <br/>
{* XXX - Do something with the state *}
{* state: [{$items[i].state}] *}
  </div>

  {if ($items[i].summary != "")}
{*    <h5>Summary:</h5>*}
    <div class="item_summary">
      {$items[i].summary}

      {* This is for items with floating elements in them (such as tall
       * images): make sure the 
       *}
      <br style="clear: both"/>
    </div>
  {/if}

{*  {if ($items[i].content != "")}*}
    <h5>Content:</h5>
    <div class="item_content">{$items[i].content}</div>
{*  {/if}*}

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
  </div>
</div>
{/strip}
{sectionelse}
<p>There are no articles to display.</p>
{/section}

</body>
</html>
