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
      <td class="icon-box">\
        <span class="mark-read">Mark as read:&nbsp;</span>\
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
      <td class="mark-td">\
        Mark as read:&nbsp;\
        <input class="mark-check" type="checkbox" name="cbb-@id@" value="1"/>\
      </td>\
    </tr>\
  </table>\
</div>';
</script>
<title>NewsBite: <?=htmlspecialchars($feed['title'])?></title>
<!-- <link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" /> -->
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/view.css" media="all" />
<?
if (isset($mobile_css))
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"skins/$skin_dir/$mobile_css\" media=\"screen\" />\n";
?>	  
<script type="text/javascript" src="skins/<?=$skin_dir?>/view.js"></script>
</head>
<body id="view-body">

<?if ($feed['image'] != ""): ?>
<img class="feed-icon" src="<?=$feed['image']?>"/>
<?endif?>

<h1><?
# Feed title
if ($feed['url'] == "")
	echo htmlspecialchars($feed['title']);
else
	echo "<a href=\"",
		htmlspecialchars($feed['url']),
		"\">",
		htmlspecialchars($feed['title']),
		"</a>";
?></h1>
<?if ($feed['subtitle'] != ""):?>
<div class="feed-subtitle"><?=htmlspecialchars($feed['subtitle'])?></div>
<?endif?>

<?if ($feed['description'] != ""):
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

<?
/* Navigation bar. **/
/* XXX - Should probably be a template, since it should be identical to
 * the one below.
 */
?>
<div class="navbar">
  <span class="earlier">
    <a href="<?=$skin_vars['prev_link']?>"><?=$skin_vars['prev_link_text']?></a>
  </span>
  <span class="later">
    <a href="<?=$skin_vars['next_link']?>"><?=$skin_vars['next_link_text']?></a>
  </span>
<?
  /* If the only things inside the navbar span are the links, Firefox
   * and Safari render the navbar as a box of height 0, with the links
   * dangling underneath. But if we put something, inside it, it will
   * have the height of the contents.
   * One problem: if the links are taller than expected, they'll still
   * overflow the navbar div. CSS positioning sucks.
   */
?>
  &nbsp;
</div>

<!-- List of items -->
<?if (count($items) > 0):
/* XXX - Should have navigation strip:
 *  <- first [20] [21] [22] _23_ [24] [25] [26] last ->
 * Put this at top and bottom. So probably ought to have a separate
 * template for it.
 */
?>
<form name="mark-items" method="post" action="markitems.php">
<div class="button-box">
  <input type="submit" name="doit" value="Apply changes"/>
</div>
<?
/* This next field says how to mark checked items */
/* XXX - When showing read items, change to value="unread"/> */
?>
<input type="hidden" name="mark-how" value="read"/>

<div id="itemlist"><img src="skins/<?=$skin_dir?>/Ajax-loader.gif"/></div>

<?
/* List of items. Items are displayed using the separate "item"
 * template
 */
foreach ($items as $i)
{
	$the_skin->_include("item",
			    array("item" => $i));
} 
?>

<div class="button-box">
<!--
  <input type="reset" name="clearit" value="Clear changes"/>
-->
<?
  /* When displaying read items, should this say "Mark all as unread"?
   * Should there be separate "Mark all as read" and "Mark all as
   * unread" buttons?
   */
?>
<!--
  <input type="submit" name="mark-all" value="Mark all as read"/>
-->
  <input type="submit" name="doit" value="Apply changes"/>
</div>
</form>

<?
/* Navigation bar. */
/* XXX - Should probably be a template, since it should be identical to
 * the one above.
 */
?>
<div class="navbar">
  <span class="earlier">
    <a href="<?=$skin_vars['prev_link']?>"><?=$skin_vars['prev_link_text']?></a>
  </span>
  <span class="later">
    <a href="<?=$skin_vars['next_link']?>"><?=$skin_vars['next_link_text']?></a>
  </span>
</div>

<?else:	/* count($items) > 0 */
/* XXX - Would it be worth having a separate template for an empty feed? */
?>
<p>There are no articles to display.</p>
<?endif /* count($items) */ ?>

<p>User <?=$skin_vars['auth_user']?>, session expires <?=$skin_vars['auth_expiration']?>.</p>
<!-- <div id="debug" style="clear:both"></div> -->

</body>
</html>
