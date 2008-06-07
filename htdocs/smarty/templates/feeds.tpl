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

<table id="feeds">
{section name=feed loop=$feeds}
{strip}
  <tr>
    <td>
      <a href="view.php?id={$feeds[feed].id}">{$feeds[feed].title}</a>
      &nbsp;(<a href="{$feeds[feed].url}">site</a>)
      &nbsp;(<a href="{$feeds[feed].feed_url}">RSS</a>)
    </td>
    <td>
      {if $feeds[feed].image eq ""}
        &nbsp;
      {else}
        <img src="{$feeds[feed].image}" />
      {/if}
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
