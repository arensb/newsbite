<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Editing {$feed.title}</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
<link rel="stylesheet" type="text/css" href="editfeed.css" media="all" />
<!-- If JavaScript is turned on, slurp in the JavaScript-specific
     stylesheet
-->
<script type="text/javascript">
  document.write('<link rel="stylesheet" type="text/css" href="style-js.css" media="all" />\n');
</script>
<!-- If JavaScript is turned off, slurp in the no-JavaScript-specific
     stylesheet
-->
<noscript>
  <link rel="stylesheet" type="text/css" href="style-nojs.css" media="all" />
</noscript>
<script type="text/javascript" src="view.js"></script>
</head>
<body id="edit-feed">

{* XXX - Links to get back to interesting places, like feed list *}
<h1>Editing feed {$feed.title}</h1>

<form name="edit-feed" method="post" action="editfeed.php">
{* Feed ID *}
<input type="hidden" name="id" value="{$feed.id}"/>
<input type="hidden" name="command" value="{$command}"/>

<table id="show-feed">
  {* XXX - Is it worth displaying the feed ID? *}
  <tr>
    <th>ID</th>
    <td>{$feed.id}</td>
  </tr>

  {* XXX - Is it worth displaying the title? It's right above *}
  <tr>
    <th>Title</th>
    <td>{$feed.title}
  </tr>

  <tr>
    <th>Subtitle</th>
    <td>{$feed.subtitle || "&nbsp;"}</td>
  </tr>

  {* User-settable nickname *}
  <tr>
    <th>Nickname</th>
    <td>
      {if isset($errors.nickname)}
        <div class="error-msg">{$errors.nickname}</div>
      {/if}
      <input type="text" name="nickname" value="{$feed.nickname}"/>
    </td>
  </tr>

  {* XXX - There should be a button or something to try to
   * auto-discover the feed URL from the site URL.
   *}
  <tr>
    <th>Site URL</th>
    <td>
      {if isset($errors.url)}
        <div class="error-msg">{$errors.url}</div>
      {/if}
      <input type="text" name="url" value="{$feed.url}"/>
    </td>
  </tr>

  <tr>
    <th>Feed URL</th>
    <td>
      {if isset($errors.feed_url)}
        <div class="error-msg">{$errors.feed_url}</div>
      {/if}
      <input type="text" name="feed_url" value="{$feed.feed_url}"/>
    </td>
  </tr>

  <tr>
    <th>Description</th>
    <td>
      <div>{$feed.description}</div>
    </td>
  </tr>

  {* XXX - Probably not worth displaying this *}
  <tr>
    <th>Last update</th>
    <td>{$feed.last_update}</td>
  </tr>

  {* XXX - Need better way of saying "don't update this more than once
   * a day" or "don't update except on Tuesdays".
   *}
  <tr>
    <th>TTL</th>
    <td>{$feed.ttl}</td>
  </tr>

  <tr>
    <th>Image</th>
    <td>
      {if isset($feed.image)}
        <img src="{$feed.image}"/>
      {else}
        No image.
      {/if}
    </td>
  </tr>

  {* XXX - Ought to manage passwords separately, so can have one
   * username/password for all of livejournal.com.
   *
   * There's also the problem that Firefox stores passwords for sites
   * and fills them in automatically in pages. So this can fill in the
   * wrong password.
   *}
  <tr>
    <th>Username</th>
    <td>
      {if isset($errors.username)}
        <div class="error-msg">{$errors.username}</div>
      {/if}
      <input type="text" name="username" value="{$feed.username}"/>
    </td>
  </tr>

  <tr>
    <th>Password</th>
    <td>
      {if isset($errors.passwd)}
        <div class="error-msg">{$errors.passwd}</div>
      {/if}
      <input type="password" name="password" value="{$feed.passwd}"/>
    </td>
  </tr>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>

</body>
</html>
