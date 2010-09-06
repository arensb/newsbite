<?php
/* feed-title.tpl
 * This template displays the feed title or nickname, plus the article
 * counts. This template is used inside the main feed display. The
 * reason it's a separate template is so that the Ajax code can update
 * the article count when it updates a feed. Since the title can
 * change as well, it's easiest to just shove the whole thing into a
 * separate template.
 */
// Feed title
$feed = $skin_vars['feed'];

if ($feed['nickname'] != "")
	$title = htmlspecialchars($feed['nickname']);
elseif ($feed['title'] != "")
	$title = htmlspecialchars($feed['title']);
else
	$title = "[no&nbsp;title]";

$counts = $skin_vars['counts'];
if (!is_int($counts['unread']+0))
	$counts['unread'] = 0;
if (!is_int($counts['read']+0))
	$counts['read'] = 0;
?>
<a href="view.php?id=<?=$feed['id']?>"><?=$title?></a>:&nbsp;
<?
/* Number of unread/read items in the feed */
/* XXX - These should be links, to show only read items, or all
 * items.
 */
?>
<?=$counts['unread']?> unread /
<?=$counts['read']?> read
<? /* Links to places related to the feed */ ?>
&nbsp;(<a href="<?=$feed['url']?>">site</a>)
&nbsp;(<a href="<?=$feed['feed_url']?>">RSS</a>)
