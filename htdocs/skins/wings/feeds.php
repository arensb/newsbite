<?php
echo "<", '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xml:lang="en"
      lang="en"
      manifest="skins/<?=$skin_dir?>/wings.manifest">
<head>
<title>NewsBite</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<?
/* Include a mobile device-specific stylesheet. */
switch ($skin_vars['mobile'])
{
    case 'iPhone':
	$mobile_css = "iphone.css";
	break;
    case 'iPad':
	$mobile_css = "ipad.css";
	break;
    case "":	// Not a mobile device
	break;
    default:	// Generic mobile device
	$mobile_css = "mobile.css";
	break;
}
?>
<script type="text/javascript" src="skins/<?=$skin_dir?>/wings.js"></script>
<? if (isset($mobile_css))
{
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"skins/$skin_vars[skin]/$mobile_css\" media=\"screen\" />\n";
}
?>
<!-- Icon for standalone app -->
<link rel="apple-touch-icon" href="skins/<?=$skin_dir?>/images/icon.png"/>
<!-- Splash screen for standalone app -->
<link rel="apple-touch-startup-image"
      href="skins/<?=$skin_dir?>/images/splash.png"
      media="screen
      and (max-device-width: 480)"/>
<link rel="apple-touch-startup-image"
      href="skins/<?=$skin_dir?>/images/splash-ipad.png"
      media="screen
      and (min-device-width: 481) and (orientation: portrait)"/>
<link rel="apple-touch-startup-image"
      href="skins/<?=$skin_dir?>/images/splash-ipad-landscape.png"
      media="screen
      and (min-device-width: 481) and (orientation: landscape)"/>
<meta name="viewport"
      content="width = device-width,
                initial-scale = 1.0,
                user-scalable = yes"/>
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-status-bar-style" content="black" />
</head>
<body>

<div class="page-list" id="page-list">
<div class="page" id="index-page">
<!-- Index page shows list of feeds -->
<h1>Feeds</h1>

<div>
<ul>
<li><a href="javascript:flip_to_page('tool-page')">Tools</a></li>
</ul>
</div>

<!-- XXX - The feeds div should start out with a "please wait" spinner
     until the data is loaded.
 -->
<div id="feeds"></div>

<noscript>
<p>Sorry, but you really, really need JavaScript turned on to get anywhere
with this skin.</p>
</noscript>
</div><!-- End of feeds page -->

<div class="page" id="feed-page">
<!-- Feed page shows the contents of a single feed -->
<h1>Feed title</h1>
<h2 class="feed-subtitle">Feed subtitle</h2>
<div class="feed-description">Feed description</div>

<ul>
<li><a href="javascript:flip_to_page('index-page')">Index</a></li>
<li><a href="javascript:foo()">Get some items</a></li><!-- XXX - Temporary -->
</ul>

<div id="articles"></div>
</div><!-- End of feed page -->

<div class="page" id="article-page">
<!-- Article page shows the contents of a single article -->
<h1>Article title</h1>
<a href="javascript:flip_to_page('feed-page')">Feed list</a> |
<a href="javascript:flip_to_page('index-page')">Index</a>
</div><!-- End of article page -->

<div class="page" id="tool-page">
<!-- Tool page shows various admin tools -->
<h1>Admin Tools</h1>

<a href="javascript:flip_to_page('index-page')">Index</a>

<ul>
<li><a href="javascript:localStorage.clear()">Clear local storage</a></li>
</ul>
</div><!-- End of tool page -->
</div><!-- End of page-list" -->
<div id="debug"></div>
</body>
</html>
