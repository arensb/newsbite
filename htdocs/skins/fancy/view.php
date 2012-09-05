<?
// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];

# Tell the client to cache this for a week
$tomorrow = new DateTime("now + 1 week");
header("Expires: " . $tomorrow->format("D, d M Y H:i:s T"));

echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<script type="text/javascript">
// Set various useful variables to pass on to scripts
var skin_dir = "<?=$skin_dir?>";	// Needed because we need to be able
					// find the CSS file.

var page_top_tmpl_text = '<img class="feed-icon" src="@image@"/>\
\
<!-- Shouldn\'t be a link if there\'s no URL -->\
<h1><a href="@url@">@title@</a></h1>\
<div class="feed-subtitle">@subtitle@</div>\
\
<div class="feed-description">@description@</div>\
\
<ul class="feed-tools">\
  <li><a href="index.php">Feed index</a></li>\
  <li><a href="update.php?id=@id@">Update feed</a></li>\
  <li><a href="editfeed.php?id=@id@">Edit feed</a></li>\
  <li><a href="unsubscribe.php?id=@id@">Unsubscribe from feed</a></li>\
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
          <li><a href="@comment_url@">Comments</a></li>\
          <li><a href="@comment_rss@">(feed)</a></li>\
        </ul>\
      </td>\
      <td class="mark-td button-box-right">\
        <label>Mark as read:&nbsp;</label>\
        <input class="mark-check" type="checkbox" name="cbb-@id@" value="1"/>\
      </td>\
    </tr>\
  </table>\
</article>';
</script>
<title>NewsBite</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/view.css" media="all" />
<script type="text/javascript" src="skins/<?=$skin_dir?>/view.js"></script>
</head>
<body id="view-body">

<div id="page-top">Feed information goes here</div>

<div class="button-box">
  <button onclick="localStorage.clear()">Clear localStorage</button>
  <button onclick="this.blur(); slow_sync()">Slow Sync</button>
</div>

<div id="itemlist"><img src="skins/<?=$skin_dir?>/Ajax-loader.gif"/></div>

<div class="button-box">
  <button onclick="this.blur(); slow_sync()">Slow Sync</button>
</div>

<p>User <?=$skin_vars['auth_user']?>, session expires <?=$skin_vars['auth_expiration']?>.</p>

</body>
</html>
