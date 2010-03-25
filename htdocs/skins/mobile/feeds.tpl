<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite</title>
<link rel="stylesheet" type="text/css" href="skins/{$skin}/index.css" media="screen"/>
<script type="text/javascript" src="skins/{$skin}/feeds.js"></script>
<script type="text/javascript">
var feed_index_tmpl = new Template('<li class="index-feed" onclick="showfeed(@id@)">@title@</li>');
</script>
<!-- Tags useful when saving the app to the desktop of an iPhone/iPod Touch -->
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent" />
<link rel="apple-touch-icon" href="star.png"/>
<link rel="apple-touch-startup-image" href="startup.png"/>
<!-- Other Apple/Safari tags -->
<meta name = "viewport"
      content = "width = device-width,
                 initial-scale = 1.0">
</head>
<body>

<div id="index-page" class="multi-page">
  <h1>Feeds</h1>
  <span onclick="flip_to_settings_page()">Settings</span>
  <ul id="feeds">
  </ul>
</div>
<div id="feed-page" class="multi-page">
  <h1>This is a feed</h1>
  <span onclick="flip_to_feed_index()">Back to feed index</span>
  <ul id="items"></ul> 
</div>
<div id="item-page" class="multi-page">
  <h1>This is an item</h1>
  <span onclick="flip_to_feed_index()">Back to feed index</span>
  <div id="item"></div>
</div>
<div id="settings-page" class="multi-page">
  <h1>These are things you can set</h1>
  <span onclick="flip_to_feed_index()">Back to feed index</span>
  <!-- XXX - skin -->
</div>

<h2>Debug:</h2>
<div id="debug"></div>
<button onclick="clrdebug()">Clear debug window</button>
</body>
