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
{* XXX - Should have navigation strip:
 *  <- first [20] [21] [22] _23_ [24] [25] [26] last ->
 * Put this at top and bottom. So probably ought to have a separate
 * template for it.
 *}
<form name="mark-items" method="post" action="markitems.php">
<input type="reset" name="clearit" value="Clear changes"/>
<input type="submit" name="doit" value="Apply changes"/>

{* List of items. Items are displayed using the separate "item.tpl"
 * template
 *}
{section name=i loop=$items}
  {include file='item.tpl' item=$items[i]}
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
