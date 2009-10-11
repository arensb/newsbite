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
<link rel="stylesheet" type="text/css" href="skins/{$skin}/setskin.css" media="all" />
</head>
<body id="setskin-body">

<div class="notice">
Cookies must be turned on for skins to work.
</div>

<form method="post" action="setskin.php">
  Choose a skin:
  <select name="newskin">
    {foreach from=$skins item=s}
      {strip}
      <option
        {if $s.dir == $current_skin} selected {/if}
        value="{$s.dir}">
        {$s.name|escape}</option>
      {/strip}
    {/foreach}
  </select>
  <br/>
  <input type="submit" name="set-skin" value="Set Skin"/>
</form>
{* XXX - Might be nice to have an iframe with a sample document,
 * so can preview the skin.
 *}
</body>
</html>
