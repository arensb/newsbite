{* XXX - Should probably have separate template for page top (and
 * possibly page bottom). Should include all the CSS magic.
 *}
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: {$feed.title}</title>
<link rel="stylesheet" type="text/css" href="skins/{$skin}/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/{$skin}/view.css" media="all" />
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
<script type="text/javascript" src="skins/{$skin}/view.js"></script>
</head>
<body id="view-body">

{if ($feed.image != "")}
<img class="feed-icon" src="{$feed.image}"/>
{/if}

{strip}
<h1>
  {if ($feed.url == "")}
    {$feed.title|escape}
  {else}
    <a href="{$feed.url}">{$feed.title|escape}</a>
  {/if}
</h1>
{if $feed.subtitle != ""}
<div class="feed-subtitle">{$feed.subtitle}</div>
{/if}
{/strip}

{if $feed.description != ""}
<div class="feed-description">{$feed.description}</div>
{/if}

{strip}
<ul class="feed-tools">
  <li><a href="index.php">Feed index</a></li>
  {if $feed.id == ""}
    <li><a href="update.php?id=all">Update all feeds</a></li>
  {else}
    <li><a href="update.php?id={$feed.id}">Update feed</a></li>
  {/if}
  {if $feed.id != ""}
    <li><a href="editfeed.php?id={$feed.id}">Edit feed</a></li>
    <li><a href="unsubscribe.php?id={$feed.id}">Unsubscribe from feed</a></li>
  {/if}
</ul>
{/strip}

{* Navigation bar. *}
{* XXX - Should probably be a template, since it should be identical to
 * the one below.
 *}
<div class="navbar">
  <span class="earlier">
    <a href="{$prev_link}">{$prev_link_text}</a>
  </span>
  <span class="later">
    <a href="{$next_link}">{$next_link_text}</a>
  </span>
  {* If the only things inside the navbar span are the links, Firefox
   * and Safari render the navbar as a box of height 0, with the links
   * dangling underneath. But if we put something, inside it, it will
   * have the height of the contents.
   * One problem: if the links are taller than expected, they'll still
   * overflow the navbar div. CSS positioning sucks.
   *}
  &nbsp;
</div>

<!-- List of items -->
{if (count($items) > 0)}
{* XXX - Should have navigation strip:
 *  <- first [20] [21] [22] _23_ [24] [25] [26] last ->
 * Put this at top and bottom. So probably ought to have a separate
 * template for it.
 *}
<form name="mark-items" method="post" action="markitems.php">
<div class="button-box">
  <input type="reset" name="clearit" value="Clear changes"/>
  <input type="submit" name="mark-all" value="Mark all as read"/>
  <input type="submit" name="doit" value="Apply changes"/>
</div>
{* This next field says how to mark checked items *}
{* XXX - When showing read items, change to value="unread"/> *}
<input type="hidden" name="mark-how" value="read"/>

{* List of items. Items are displayed using the separate "item.tpl"
 * template
 *}
{foreach from=$items item=i}
  {assign var="date" value=$i.pub_date|date_format:"%d %b, %Y"}
  {if $date != $olddate}
    <h3 class="date-header">{$date}</h3>
  {/if}
  {include file='item.tpl' item=$i}
  {assign var="olddate" value=$date}
{/foreach}

<div class="button-box">
  <input type="reset" name="clearit" value="Clear changes"/>
  {* When displaying read items, should this say "Mark all as unread"?
   * Should there be separate "Mark all as read" and "Mark all as
   * unread" buttons?
   *}
  <input type="submit" name="mark-all" value="Mark all as read"/>
  <input type="submit" name="doit" value="Apply changes"/>
</div>
</form>

{* Navigation bar. *}
{* XXX - Should probably be a template, since it should be identical to
 * the one above.
 *}
<div class="navbar">
  <span class="earlier">
    <a href="{$prev_link}">{$prev_link_text}</a>
  </span>
  <span class="later">
    <a href="{$next_link}">{$next_link_text}</a>
  </span>
</div>

{else}

{* XXX - Would it be worth having a separate template for an empty feed? *}
<p>There are no articles to display.</p>
{{* item.tpl
 * Display an item.
 * This is just a fragment template. It is intended to be included in
 * another template.
 *
 * Variables:
 *	$item - The item to display
 *}
{strip}
{* XXX - Perhaps add a 'feed=<feed_id>', so we can have different colors
 * for different feeds.
 *}
<div class="item" id="item-{$item.guid}">
  {* This hidden field just lists the ID of an item that was displayed,
   * so that we can mark it with the "Mark all read" button.
   *}
  <input type="hidden" name="item-{$item.id}" value=""/>

  <table class="item-header">
    <tr>
      <td class="info">
        <h3 class="item-title">
          {if $item.url != ""}
            <a href="{$item.url}">
          {/if}
          {if ($item.title == "")}
            [no title]
          {else}
            {$item.title}
          {/if}
          {if $item.url != ""}
            </a>
          {/if}
        </h3>
        {if ($items != "")}
          <h3 class="feed-title">
            <a href="{$feeds[$item.feed_id].url}">
              {$feeds[$item.feed_id].title}
            </a>
          </h3>
        {/if}
        {if $item.author != ""}
          {* XXX - If ever add email address:
           * U+2709 == envelope; U+270d == writing hand
           *}
          <p class="item-author">by {$item.author|escape}</p>
        {/if}
      </td>
      <td class="icon-box">
{* XXX - Do something with categories *}
{* category: [{$item.category}]<br/>*}
       {* XXX - When showing read items, change this to "mark as unread" *}
        Mark as read:&nbsp;
        {* "cbt": checkbox top *}
        <input class="mark-check" type="checkbox" name="cbt-{$item.id}" value="1"/>
      </td>
    </tr>
  </table>

{* Four cases:
 * no summary, no content: not collapsible; set content to "left blank"
 * no summary,    content: not collapsible; show content.
 *    summary, no content: not collapsible; show summary, "More->" link.
 *    summary,    content:     collapsible; show content. With JS, add toggle bars.
 *}
  {if $item.summary == ""}
    {* No summary *}
    {if $item.content == ""}
      {* No summary, no content *}
      {assign var="collapsible" value="no"}
      {assign var="which" value="content"}
      {assign var="content" value="This space intentionally left blank"}
    {else}
      {* No summary,    content *}
      {assign var="collapsible" value="no"}
      {assign var="which" value="content"}
    {/if}
  {else}
    {* Summary *}
    {if $item.content == ""}
      {*    summary, no content *}
      {assign var="collapsible" value="no"}
      {assign var="which" value="summary"}
    {else}
      {*    summary,    content *}
      {assign var="collapsible" value="yes"}
      {assign var="which" value="content"}
    {/if}
  {/if}
  <div class="content-panes" collapsible="{$collapsible}" which="{$which}">
    <div class="collapse-bar"
         onclick="javascript:collapse(this)">
      &#x25b2;{* Upward-pointing triangle *}
    </div>

    <div class="item-summary">
      {$item.summary}
      {* This is for items with floating elements in them (such as
       * tall images): make sure the image is contained within the
       * <div> and doesn't go overflowing where we don't want it.
       *}
      <br style="clear: both"/>
    </div>

    <div class="item-content">
      {$item.content}
      <br style="clear: both"/>
    </div>
    <div class="collapse-bar"
         onclick="javascript:collapse(this)">
      &#x25b2;{* Upward-pointing triangle *}
    </div>
    <div class="expand-bar"
         onclick="javascript:expand(this)">
      &#x25bc;{* Downward-pointing triangle *}
    </div>
  </div>

  <table class="item-footer">
    <tr>
      <td class="bottom-link-box">
        <ul class="bottom-links">
          {if $item.summary != "" && $item.content == "" && $item.url != ""}
            <li><a href="{$item.url}">Read more</a></li>
          {/if}
          {if (isset($item.comment_url))}
            <li><a href="{$item.comment_url}">Comments</a></li>
          {/if}
          {if (isset($item.comment_rss))}
            <li><a href="{$item.comment_rss}">(feed)</a></li>
          {/if}
        </ul>
      </td>
      <td class="mark-td">
        {* XXX - When showing read items, change this to "mark as unread" *}
        Mark as read:&nbsp;
        {* "cbb": checkbox bottom *}
        <input class="mark-check" type="checkbox" name="cbb-{$item.id}" value="1"/>
      </td>
    </tr>
  </table>
</div>
{/strip}
/if}

</body>
</html>
