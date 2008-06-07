<html>
<head>
<title>NewsBite: {$feed[title]}</title>
<link rel="stylesheet" type="text/css" href="style.css" media="all" />
</head>
<body>

<h1>Feed: {$feed[title]}</h1>

{section name=feed loop=$feed}
{strip}
<div class="item" style="border: 1px solid black">
url: [<a href="{$feed[feed].url}">{$feed[feed].url}</a>]<br/>
title: [{$feed[feed].title}]<br/>
author: [{$feed[feed].author}]<br/>
summary: <div style="border: 1px solid red">{$feed[feed].summary}</div>
content: <div style="border: 1px solid blue">{$feed[feed].content}</div>
category: [{$feed[feed].category}]<br/>
comment_url: [{$feed[feed].comment_url}]<br/>
comment_feed: [{$feed[feed].comment_feed}]<br/>
guid: [{$feed[feed].guid}]<br/>
pub_date: [{$feed[feed].pub_date}]<br/>
last_update: [{$feed[feed].last_update}]<br/>
state: [{$feed[feed].state}]
</div>
{/strip}
{/section}

</body>
</html>
