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

<table id="feeds">
{section name=feed loop=$feeds}
{strip}
  <tr class="{cycle values="odd-row,even-row"}">
    <td>
      <a href="view.php?id={$feeds[feed].id}">{$feeds[feed].title}</a>
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
