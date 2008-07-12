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

{if (isset($feed.image))}
<img class="feed-icon" src="{$feed.image}"/>
{/if}

{* XXX - Link to edit feed/feed options *}
{strip}
<h1>
  {if ($feed.url == "")}
    {$feed.title}
  {else}
    <a href="{$feed.url}">{$feed.title}</a>
  {/if}
</h1>
{if $feed.subtitle != ""}
<div class="feed-subtitle">{$feed.subtitle}</div>
{/if}
{/strip}

{if $feed.description != ""}
<div class="feed-description">{$feed.description}</div>
{/if}

<!-- List of items -->
{if (count($items) > 0)}
{* XXX - Should have navigation strip:
 *  <- first [20] [21] [22] _23_ [24] [25] [26] last ->
 * Put this at top and bottom. So probably ought to have a separate
 * template for it.
 *}
<form name="mark-items" method="post" action="markitems.php">
<input type="reset" name="clearit" value="Clear changes"/>
<input type="submit" name="mark-all" value="Mark all as read"/>
<input type="submit" name="doit" value="Apply changes"/>
{* This next field says how to mark checked items *}
{* XXX - When showing read items, change to value="unread"/> *}
<input type="hidden" name="mark-how" value="read"/>

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
</div>

{* List of items. Items are displayed using the separate "item.tpl"
 * template
 *}
{foreach from=$items item=i}
  {include file='item.tpl' item=$i}
{/foreach}

<input type="reset" name="clearit" value="Clear changes"/>
{* When displaying read items, should this say "Mark all as unread"?
 * Should there be separate "Mark all as read" and "Mark all as unread"
 * buttons?
 *}
<input type="submit" name="mark-all" value="Mark all as read"/>
<input type="submit" name="doit" value="Apply changes"/>
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
{/if}

</body>
</html>
