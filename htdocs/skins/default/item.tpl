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
      {* This hidden field just lists the ID of an item that was displayed,
       * so that we can mark it with the "Mark all read" button.
       *}
      <input type="hidden" name="item-{$item.id}" value=""/>
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
        {if ($items != "")}
          <h3 class="feed-title">
            <a href="{$feeds[$item.feed_id].url}">
              {$feeds[$item.feed_id].title}
            </a>
          </h3>
        {/if}
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
{* Four cases:
 * no summary, no content: link to real page.
 *    summary, no content: show summary, "More->" link.
 * no summary,    content: show content.
 *    summary,    content: show content. With JS, add toggle bars.
 *}
{* XXX - Should <div content-panes> be outside everything, so that we
 * can hide _all_ of the item text (but still show the header, grayed
 * out), when marking an item as read?
 *}
  {if ($item.summary == "")}
    {if ($item.content == "")}
      {* No summary, no content *}
      {if ($item.url == "")}
        {* In practice it doesn't look as if there are any items with no
         * summary, no content, and no URL.
         *}
        <div class="item-empty">This space intentionally left blank.</div>
      {else}
        <div class="item-empty"><a href="{$item.url}">Read on</a></div>
      {/if}
    {else}
      {* No summary, content *}
      <div class="item-content">
        {$item.content}
        <br style="clear: both"/>
      </div>
    {/if}
  {else}
    {if ($item.content == "")}
      {* Summary, no content *}
      <div class="item-summary">
        {$item.summary}
        {* This is for items with floating elements in them (such as
         * tall images): make sure the image is contained within the
         * <div> and doesn't go overflowing where we don't want it.
         *}
        <br style="clear: both"/>
      </div>
      {if ($item.url != "")}
        {* Link to the full item *}
        <div><a href="{$item.url}">Read more</a></div>
      {/if}
    {else}
      {* Summary, content *}
      {* XXX - Perhaps new articles should be expanded by default, and
       * unread ones should be collapsed by default.
       *}
      {* XXX - Perhaps a better way to do switching, and which allows
       * switching panes better:
       * <div class="content-panes" which="summary">
       * or
       * <div class="content-panes" which="content">
       *
       * Then, in "style.css":
       *
       * .content-panes[which="summary"] .item-summary,
       * .content-panes[which="content"] .item-content {
       * 	display:		block;
       * }
       * 
       * .content-panes[which="summary"] .item-content,
       * .content-panes[which="content"] .item-summary {
       * 	display:		none;
       * }
       *
       * and the function that toggles between the two can simply use
       * getAttribute("which") and setAttribute("which", "summary");
       *}
      <div class="content-panes">
        <div class="item-summary">
          {$item.summary}
          {* This is for items with floating elements in them (such as
           * tall images): make sure the image is contained within the
           * <div> and doesn't go overflowing where we don't want it.
           *}
          <br style="clear: both"/>
          {* XXX - expand-bar and collapse-bar have the same style.
           * Come up with a name for both, so as not to duplicate.
           *}
          <div class="expand-bar"
               onclick="javascript:expand(this)">
            vvv Expand vvv
          </div>
        </div>
        <div class="item-content">
          <div class="collapse-bar"
               onclick="javascript:collapse(this)">
            ^^^ Collapse ^^^
          </div>
          {$item.content}
          <br style="clear: both"/>
          <div class="collapse-bar"
               onclick="javascript:collapse(this)">
            ^^^ Collapse ^^^
          </div>
        </div>
      </div>
    {/if}
  {/if}{* item.summary == "" *}

  <div class="item-footer">
    {if (isset($item.comment_url))}
      <a href="{$item.comment_url}">Comments</a>
      {if (isset($item.comment_rss))}
        &nbsp;
        <a href="{$item.comment_rss}">(feed)</a>
      {/if}
      <br/>
    {/if}
    &nbsp;
    (New: <input type="radio" name="state-{$item.id}" value="nb" />
     Unread: <input type="radio" name="state-{$item.id}" value="ub" />
     Read:<input type="radio" name="state-{$item.id}" value="rb" />
    )
  </div>
</div>
{/strip}
