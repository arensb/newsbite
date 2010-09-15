<?
/* XXX - Should probably have separate template for page top (and
 * possibly page bottom). Should include all the CSS magic.
 */

// Give some of the skin variables shorter names
$skin_dir = $skin_vars['skin'];
$feed = &$skin_vars['feed'];
$feed_id = $feed['id'];
$items = &$skin_vars['items'];

echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: <?=$feed['title']?></title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/view.css" media="all" />
<!-- If JavaScript is turned on, slurp in the JavaScript-specific
     stylesheet
-->
<script type="text/javascript">
  document.write('<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style-js.css" media="all" />\n');
</script>
<!-- If JavaScript is turned off, slurp in the no-JavaScript-specific
     stylesheet
-->
<noscript>
  <link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style-nojs.css" media="all" />
</noscript>
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
	echo "<a href=\"$feed[url]\">", htmlspecialchars($feed['title']),
		"</a>";
?></h1>
<?if ($feed['subtitle'] != ""):?>
<div class="feed-subtitle"><?=$feed['subtitle']?></div>
<?endif?>

<?if ($feed['description'] != ""):?>
<div class="feed-description"><?=$feed['description']?></div>
<?endif?>

<ul class="feed-tools">
  <li><a href="index.php">Feed index</a></li>
<? if ($feed_id == ""): ?>
  <li><a href="update.php?id=all">Update all feeds</a></li>
<? else: ?>
  <li><a href="update.php?id=<?=$feed_id?>">Update feed</a></li>
  <li><a href="editfeed.php?id=<?=$feed_id?>">Edit feed</a></li>
  <li><a href="unsubscribe.php?id=<?=$feed_id?>">Unsubscribe from feed</a></li>
<?endif?>
</ul>

<?
/* Navigation bar. */
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
<? if (count($items) > 0):
/* XXX - Should have navigation strip:
 *  <- first [20] [21] [22] _23_ [24] [25] [26] last ->
 * Put this at top and bottom. So probably ought to have a separate
 * template for it.
 */
?>
<form name="mark-items" method="post" action="markitems.php">
<div class="button-box">
  <input type="reset" name="clearit" value="Clear changes"/>
  <input type="submit" name="mark-all" value="Mark all as read"/>
  <input type="submit" name="doit" value="Apply changes"/>
</div>
<?
/* This next field says how to mark checked items */
/* XXX - When showing read items, change to value="unread"/> */
?>
<input type="hidden" name="mark-how" value="read"/>

<?
/* List of items. Items are displayed using the separate "item"
 * template
 */
$olddate = '';
foreach ($items as $i)
{
	$date = strftime("%d %b, %Y", strtotime($i['pub_date']));
	if ($date != $olddate)
		echo "<h3 class=\"date-header\">$date</h3>";
	$the_skin->_include("item",
			    array("item" => $i));
	$olddate = $date;
}
?>

<div class="button-box">
  <input type="reset" name="clearit" value="Clear changes"/>
<?/* When displaying read items, should this say "Mark all as unread"?
   * Should there be separate "Mark all as read" and "Mark all as
   * unread" buttons?
   */
?>
  <input type="submit" name="mark-all" value="Mark all as read"/>
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

<? else: /* # articles == 0 */ ?>
<?
/* XXX - Would it be worth having a separate template for an empty feed? */
?>
<p>There are no articles to display.</p>
<? endif /* # items > 0 */ ?>

</body>
</html>