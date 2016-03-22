<?php
/* view.php
 * Display a feed.
 */
$out_fmt = "html";
require_once("common.inc");

// Tell the client to cache this for a week
// XXX - Rewrite this as a static HTML document so it can be cached
// for a long time.
$tomorrow = new DateTime("now + 1 week");
header("Expires: " . $tomorrow->format("D, d M Y H:i:s T"));

echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html manifest="newsbite.manifest">
<head>
<!-- Icon for standalone app -->
<link rel="apple-touch-icon" href="images/newsbite-icon.png"/>
<link rel="icon" sizes="114x114" href="images/newsbite-icon.png"/>
<!-- Header color for Chrome -->
<meta name="theme-color" content="#8080c0" />
<!-- Splash screen for standalone app, iPad, portrait: -->
<link rel="apple-touch-startup-image"
      href="images/newsbite-splash-ipad.png"
      media="screen
      and (min-device-width: 481) and (orientation: portrait)"/>
<?php
// Actually, don't use this: it make iOS consider this app a
// standalone app, which means that all links open a separate instance
// of Mobile Safari (rather than just a new tab), which is inelegant.
// Also makes startup seem longer.
//<!-- iOS: This standalone app runs in full-screen mode -->
//<meta name="apple-mobile-web-app-capable" content="yes" />
?>
<script type="text/javascript">
// Set various useful variables to pass on to scripts

var page_top_tmpl_text = '<img class="feed-icon" src="@image@"/>\
\
<!-- Shouldn\'t be a link if there\'s no URL -->\
<h1><a class="feed-link" href="@url@">@title@</a></h1>\
<div class="feed-subtitle">@subtitle@</div>\
\
<div class="feed-description">@description@</div>\
\
<ul class="feed-tools">\
  <li><a href="index.php">Feed index</a></li>\
  <li><a href="update.php?id=@id@">Update feed</a></li>\
  <li><a href="editfeed.php?id=@id@">Edit feed</a></li>\
  <li><a href="unsubscribe.php?id=@id@">Unsubscribe from feed</a></li>\
  <li><a href="login.php?from=view.php%3fid=@id@">Log in</a></li>\
</ul>';

// XXX - This template needs an awful lot of work.
// XXX - Don't show left checkboxes on non-iPads.
var item_tmpl_text = '<article class="item" id="item-@id@">\
  <table class="item-header">\
    <tr>\
      <!-- Second set of checkboxes, for iPad -->\
      <td class="icon-box button-box-left">\
        <input class="mark-check" type="checkbox" name="cbX-@id@" value="1"/>\
      </td>\
      <td class="info">\
        <h3 class="item-title">\
          <a href="@url@" @url_attr@>@title@</a>\
        </h3>\
        <h3 class="feed-title">\
          <a href="@feed_url@">@feed_title@</a>\
        </h3>\
        <span class="item-author">by @author@</span>,&nbsp;\
        <time datetime="@pub_date@" pubdate>@pretty_pub_date@</time>\
      </td>\
      <td class="icon-box button-box-right">\
        <label class="mark-read">Mark as read:&nbsp;</label>\
        <input class="mark-check" type="checkbox" name="cbt-@id@" value="1"/>\
      </td>\
    </tr>\
  </table>\
  <div class="content-panes show-@which@" collapsible="@collapsible@" which="@which@">\
    <div class="collapse-bar upper-bar">\
      &#x25b2;\
    </div>\
\
    <div class="item-summary">\
      @summary@\
      <br style="clear:both"/>\
    </div>\
\
    <div class="item-content">\
      @content@\
      <br style="clear: both"/>\
    </div>\
    <div class="collapse-bar lower-bar">\
      &#x25b2;\
    </div>\
    <div class="expand-bar">\
      &#x25bc;\
    </div>\
  </div>\
\
  <table class="item-footer">\
    <tr>\
      <td class="icon-box button-box-left">\
        <input class="mark-check" type="checkbox" name="cbY-@id@" value="1"/>\
      </td>\
      <td class="bottom-link-box">\
        <ul class="bottom-links">\
          <li><a href="@url@">Read more</a></li>\
          <li><a class="comment-link" href="@comment_url@">Comments</a></li>\
          <li><a href="@comment_rss@">(feed)</a></li>\
        </ul>\
      </td>\
      <td class="mark-td button-box-right">\
        <label class="mark-read">Mark as read:&nbsp;</label>\
        <input class="mark-check" type="checkbox" name="cbb-@id@" value="1"/>\
      </td>\
    </tr>\
  </table>\
</article>';
</script>
<title>NewsBite</title>
<link rel="stylesheet" type="text/css" href="css/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="css/view.css" media="all" />
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/view.js"></script>
<script type="text/javascript">
$(function() {
	// Leave one browser-window's worth of height at the bottom of
	// the page. This is so that when you collapse the last post on the
	// page, it doesn't jump jarringly.
	// XXX - Ought to attach a listener or something that updates this
	// when the window is resized. Unless "100%" DTRT.
	$("#bottom-button-box").css("margin-bottom", $(window).height());
});
</script>
</head>
<body id="view-body" orientation="up">
<div id="test-info">Width: <label id="width">?</label>, Height: <label id="height">?</label>; dpi: <label id="dpi">?</label></div>
<div id="one-inch" style="width: 1in;"><!--1in--></div>
<div id="navbar" class="open">
  <ul class="content">
    <li><a onclick="window.scrollTo(0,0)">Top</a></li>
    <li><a onclick="slow_sync()">Slow Sync</a></li>
  </ul>
  <span class="open-button">&gt;</span>
  <span class="close-button">&lt;</span>
</div>

<div id="page-top">Feed information goes here</div>

<div class="button-box" id="top-button-box">
  <button onclick="this.blur(); slow_sync()">Slow Sync</button>
  <button onclick="this.blur(); hello_test()">Hello test</button>
</div>

<div id="itemlist"><img src="images/Ajax-loader.gif"/></div>

<div class="button-box" id="bottom-button-box">
  <button onclick="this.blur(); slow_sync()">Slow Sync</button>
  <button onclick="localStorage.clear()">Clear localStorage</button>
</div>

</body>
</html>
