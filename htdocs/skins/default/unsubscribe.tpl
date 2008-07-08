<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Unsubscribe</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
<link rel="stylesheet" type="text/css" href="unsubscribe.css" media="all" />
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
<body id="unsubscribe-body">

<h1>Unsubscribe</h1>

<form name="unsubscribe-form" method="post" action="unsubscribe.php">
<input type="hidden" name="id" value="{$feed.id}"/>

<table id="show-feed">
  <tr>
    <th>Title</th>
    <td>{$feed.title}</td>
  </tr>

  {if $feed.nickname != ""}
  <tr>
    <th>Nickname</th>
    <td>{$feed.nickname}
  </tr>
  {/if}

  {if $feed.description != ""}
  <tr>
    <th>Description</th>
    <td>{$feed.description}</td>
  </tr>
  {/if}

  <tr>
    <td colspan="2">
      Check here if you really want to unsubscribe:&nbsp;
      <input type="checkbox" name="confirm" value="yes"/>
    </td>
  </tr>
</table>

<input type="reset" value="Clear changes"/>
<input type="submit" name="unsub" value="Unsubscribe"/>
</form>

</body>
</html>
