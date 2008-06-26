{* item.tpl
 * Display an item.
 * This is just a fragment template. It is intended to be included in
 * another template.
 *
 * Variables:
 *	$item - The item to display
 *}
{strip}
<div class="item" id="item-{$item.guid}" style="border: 1px solid black">
  <table class="item-header">
    <tr>
      <td class="date-box">
      {* XXX - This should probably include the year somewhere *}
        <ul>
          <li class="day">{$item.pub_date|date_format:"%e"}</li>
          <li class="mon">{$item.pub_date|date_format:"%b"}</li>
          <li class="time">{$item.pub_date|date_format:"%H:%M:%S"}</li>
        </ul>
      </td>
      <td class="info">
        <h3 class="item-title">
          <a href="{$item.url}">
            {if ($item.title == "")}
              [no title]
            {else}
              {$item.title}
            {/if}
          </a>
        </h3>
      </td>
      <td class="icon-box">
{* XXX - Do something with categories *}
{* category: [{$item.category}]<br/>*}
        <ul>
{* Note that there are two groups of radio buttons per item: one group
 * at the top, and another at the bottom. These all have the same
 * radio group name: "state_{id}". Otherwise, confusion can arise if
 * the item is marked as read at the top, and unread at the bottom.
 * The values are "na", "ua", "ra" at the top (for new, unread, read)
 * and "nb", "ub", and "rb" at the bottom. The "a" and "b" are just
 * there because w3.org says that all the radio buttons in a group
 * should have different values.
 *}
{* XXX - There should really be one largish button, to mark the item
 * as read. Currently too hard to click on the correct one for the
 * most common action.
 *}
          <li>New <input type="radio" name="state-{$item.id}" value="na"/></li>
          <li>Unread <input type="radio" name="state-{$item.id}" value="ua"/></li>
          <li>Read <input type="radio" name="state-{$item.id}" value="ra"/></li>
        </ul>
      </td>
    </tr>
  </table>
{* XXX - If JavaScript is turned on, should have selectable tabs for the
 * summary and full content.
 *}
{* XXX - Four cases:
 * no summary, no content: link to real page.
 *    summary, no content: show summary, "More->" link.
 * no summary,    content: show content.
 *    summary,    content: see below:
 *
 * In the case where there's both a summary and a content, behavior
 * should be different depending on whether JS is on or off: when off,
 * show the content.
 * (XXX - However, this causes a problem with feeds like jesusandmo,
 * which can have different text for summary and content.)
 *
 * If JS is on, show the summary, with an "expand" button. Clicking on
 * it hides the summary and displays the content, with a "collapse"
 * button (that reverts to the previous state, of course).
 *}
  {if ($item.summary != "")}
{*    <h5>Summary:</h5>*}
    <div class="item-summary">
      {$item.summary}

      {* This is for items with floating elements in them (such as
       * tall images): make sure the image is contained within the
       * <div> and doesn't go overflowing where we don't want it.
       *}
      <br style="clear: both"/>
    </div>
  {/if}

  {if ($item.content != "")}
{*    <h5>Content:</h5>*}
    <div class="item-content">{$item.content}</div>
  {/if}

  <div class="item-footer">
    {if (isset($item.comment_url))}
      <a href="{$item.comment_url}">Comments</a>
      {if (isset($item.comment_rss))}
        &nbsp;
        <a href="{$item.comment_rss}">(feed)</a>
      {/if}
      <br/>
    {/if}
{* XXX - Control buttons to mark as read and whatnot. *}
    &nbsp;
    (New: <input type="radio" name="state-{$item.id}" value="nb" />
     Unread: <input type="radio" name="state-{$item.id}" value="ub" />
     Read:<input type="radio" name="state-{$item.id}" value="rb" />
    )
  </div>
</div>
{/strip}
