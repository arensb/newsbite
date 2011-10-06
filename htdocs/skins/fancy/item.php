<?
/* item.php
 * Display an item.
 * This is just a fragment template. It is intended to be included in
 * another template.
 *
 * Variables:
 *	$item - The item to display
 */

$item = &$skin_vars['item'];
$item_id = $item['id'];

// Adjustments for mobile devices
switch ($skin_vars['mobile'])
{
    case 'iPhone':
    case 'iPad':
	// On iPhone/iPad, open articles in a new window.
	$title_link_attribs = ' target="new"';	# Note space
	break;
    default:
	$title_link_attribs = '';
	break;
}

// Set default title
if ($item['title'] == "" ||
    preg_match('/^\s*$/', $item['title']))
	$item['title'] = "[no title]";

/* XXX - Perhaps add a 'feed=<feed_id>', so we can have different colors
 * for different feeds.
 */
/* XXX - state="unread" in the next line will have to change when it's
 * possible to show read items.
 */
?>
<div class="item" id="item-<?=$item_id?>" state="unread">
<?
  /* This hidden field just lists the ID of an item that was displayed,
   * so that we can mark it with the "Mark all read" button.
   */
?>
  <input type="hidden" name="item-<?=$item_id?>" value=""/>

  <table class="item-header">
    <tr>
<?    if ($skin_vars['mobile'] == "iPad"):
      /* Put a second set of checkboxes on the left on the iPad, so that
       * I can hold it in either hand, and push the buttons with my thumb.
       */
?>
        <td class="icon-box button-box-left">
          <input class="mark-check" type="checkbox" name="cbX-<?=$item_id?>" value="1"/>
        </td>
<?    endif ?>
      <td class="info">
        <h3 class="item-title">
<?	// Item title, possibly with link
	if ($item['url'] == "")
		echo $item['title'];
	else
		echo "<a href=\"",
			htmlspecialchars($item['url']),
			"\"$title_link_attribs>",
			$item['title'],
			"</a>";
?>
        </h3>
<?	// Link to original site
        if ($skin_vars['feeds'] != ""): ?>
          <h3 class="feed-title">
            <a href="<?=htmlspecialchars($skin_vars['feeds'][$item['feed_id']]['url'])?>">
              <?=$skin_vars['feeds'][$item['feed_id']]['title']?>
            </a>
          </h3>
<?      endif ?>
<?	// Author
	if ($item['author'] != ""):
	/* XXX - If ever add email address:
	 * U+2709 == envelope; U+270d == writing hand
	 */
?>
	<span class="item-author">by <?=htmlspecialchars($item['author'])?></span>,&nbsp;
<?	endif ?>
	<span><?=strftime("%e %b, %Y", strtotime($item['pub_date']))?></span>
      </td>
      <td class="icon-box">
<?
/* XXX - Do something with categories */
/* category: [{$item.category}]<br/>*/
       /* XXX - When showing read items, change this to "mark as unread" */
?>
        <span class="mark-read">Mark as read:&nbsp;</span>
<?      /* "cbt": checkbox top */ ?>
        <input class="mark-check" type="checkbox" name="cbt-<?=$item_id?>" value="1"/>
      </td>
    </tr>
  </table>

<?
/* Four cases:
 * no summary, no content: not collapsible; set content to "left blank"
 * no summary,    content: not collapsible; show content.
 *    summary, no content: not collapsible; show summary, "More->" link.
 *    summary,    content:     collapsible; show content. With JS, add toggle bars.
 */
if ($item['summary'] == "")
{
	/* No summary */
	if ($item['content'] == "")
	{
		/* No summary, no content */
		$collapsible = "no";
		$which = "content";
		$content = "This space intentionally left blank";
	} else {
		/* No summary,    content */
		$collapsible = "no";
		$which = "content";
	}
} else {
	/* Summary */
	if ($item['content'] == "")
	{
		/*    summary, no content */
		$collapsible = "no";
		$which = "summary";
	} else {
		/*    summary,    content */
		$collapsible = "yes";
		$which = "content";
	}
}
?>
  <div class="content-panes show-<?=$which?>" collapsible="<?=$collapsible?>" which="<?=$which?>">
    <div class="collapse-bar upper-bar">
      &#x25b2;<?/* Upward-pointing triangle */?>
    </div>

    <div class="item-summary">
      <?=$item['summary']?>
<?    /* This is for items with floating elements in them (such as
       * tall images): make sure the image is contained within the
       * <div> and doesn't go overflowing where we don't want it.
       */
?>
      <br style="clear: both"/>
    </div>

    <div class="item-content">
      <?=$item['content']?>
      <br style="clear: both"/>
    </div>
    <div class="collapse-bar lower-bar">
      &#x25b2;<?/* Upward-pointing triangle */?>
    </div>
    <div class="expand-bar">
      &#x25bc;<?/* Downward-pointing triangle */?>
    </div>
  </div>

  <table class="item-footer">
    <tr>
<?    if ($skin_vars['mobile'] == "iPad"):
        /* Second set of checkboxes on the left on the iPad */
?>
        <td class="icon-box button-box-left">
<?        /* "cbY": checkbox bottom left */ ?>
          <input class="mark-check" type="checkbox" name="cbY-<?=$item_id?>" value="1"/>
        </td>
<?    endif ?>
      <td class="bottom-link-box">
        <ul class="bottom-links">
<?	if ($item['summary'] != "" &&
	    $item['content'] == "" &&
	    $item['url'] != ""):
?>
            <li><a href="<?=htmlspecialchars($item['url'])?>">Read more</a></li>
<?	endif;
	if (isset($item['comment_url'])): 
?>
            <li><a href="<?=htmlspecialchars($item['comment_url'])?>">Comments</a></li>
<?	endif;
	if (isset($item['comment_rss'])):
?>
            <li><a href="<?=htmlspecialchars($item['comment_rss'])?>">(feed)</a></li>
<?	endif ?>
        </ul>
      </td>
      <td class="mark-td">
<?      /* XXX - When showing read items, change this to "mark as unread" */ ?>
        Mark as read:&nbsp;
<?      /* "cbb": checkbox bottom */ ?>
        <input class="mark-check" type="checkbox" name="cbb-<?=$item_id?>" value="1"/>
      </td>
    </tr>
  </table>
</div>
