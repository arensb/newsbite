<?
// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
$feed = &$skin_vars['feed'];
$feed_id = $feed['id'];
$items = &$skin_vars['items'];

/* Figure out which mobile device, if any, we're on */
switch ($skin_vars['mobile'])
{
    case 'iPhone':
	$mobile_css = "iphone.css";
	break;
    case 'iPad':
	$mobile_css = "ipad.css";
	break;
    case 'Android':
	$mobile_css = "android.css";
	break;
    default:
	break;
}

# Tell the client to cache this for a week
$tomorrow = new DateTime("now + 1 week");
header("Expires: " . $tomorrow->format("D, d M Y H:i:s T"));

echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<? /* Include a device-specific viewport, if necessary */
if ($skin_vars['mobile'] == "iPhone")
	echo '<meta name="viewport" content="width = device-width, initial-scale=0.5" />', "\n";
elseif ($skin_vars['mobile'] == "Android")
	echo '<meta name="viewport" content="width = device-width, initial-scale=1.0" />', "\n";
?>
<script type="text/javascript">
// Set various useful variables to pass on to scripts
// The parameters we were given were the feed ID, and the start offset.
var mobile = "<?=$skin_vars['mobile']?>";
var feed = <?=jsonify($skin_vars['feed'])?>;
var start_offset = <?=jsonify($skin_vars['start'])?>;

// XXX - This template needs an awful lot of work. See item.php
// XXX - Don't show left checkboxes on non-iPads.
var item_tmpl_text = '<div class="item" id="item-@id@">\
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
</div>';
</script>
<title>NewsBite: <?=htmlspecialchars($feed['title'])?></title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/view.css" media="all" />
<?
if (isset($mobile_css))
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"skins/$skin_dir/$mobile_css\" media=\"screen\" />\n";
?>	  
<script type="text/javascript" src="skins/<?=$skin_dir?>/view.js"></script>
</head>
<body id="view-body">

<?if (isset($feed['image']) && $feed['image'] != ""): ?>
<img class="feed-icon" src="<?=$feed['image']?>"/>
<?endif?>

<h1><?
# Feed title
if (!isset($feed['url']) || $feed['url'] == "")
	echo htmlspecialchars($feed['title']);
else
	echo "<a href=\"",
		htmlspecialchars($feed['url']),
		"\">",
		htmlspecialchars($feed['title']),
		"</a>";
?></h1>
<?if (isset($feed['subtitle']) && $feed['subtitle'] != ""):?>
<div class="feed-subtitle"><?=htmlspecialchars($feed['subtitle'])?></div>
<?endif?>

<?if (isset($feed['description']) && $feed['description'] != ""):
	$description = $feed['description'];
	run_hooks("clean-html", array(&$description))
?>
<div class="feed-description"><?=$description?></div>
<?endif?>
<?
// XXX - Debugging
#echo "feed: [<pre>"; print_r($feed); echo "</pre>]<br/>\n";
/*echo "skin vars:<br/>\n";
foreach ($skin_vars as $k => $v)
{
	echo "[", $k, "]<br/>\n";
}*/
?>

<ul class="feed-tools">
  <li><a href="index.php">Feed index</a></li>
<?if ($feed_id == ""):?>
  <li><a href="update.php?id=all">Update all feeds</a></li>
<?else:?>
  <li><a href="update.php?id=<?=$feed_id?>">Update feed</a></li>
  <li><a href="editfeed.php?id=<?=$feed_id?>">Edit feed</a></li>
  <li><a href="unsubscribe.php?id=<?=$feed_id?>">Unsubscribe from feed</a></li>
<?endif?>
</ul>
<button onclick="localStorage.clear()">Clear localStorage</button>

<!-- List of items -->
<?
/* XXX - Should have navigation strip:
 *  <- first [20] [21] [22] _23_ [24] [25] [26] last ->
 * Put this at top and bottom. So probably ought to have a separate
 * template for it.
 */
?>
<div class="button-box">
  <button onclick="refresh()">Refresh</button>
  <button onclick="sync()">Sync</button>
</div>

<div id="itemlist"><img src="skins/<?=$skin_dir?>/Ajax-loader.gif"/></div>

<div class="button-box">
  <button onclick="refresh()">Refresh</button>
  <button onclick="sync()">Sync</button>
</div>

<p>User <?=$skin_vars['auth_user']?>, session expires <?=$skin_vars['auth_expiration']?>.</p>

</body>
</html>
