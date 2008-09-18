{* item.tpl
 * Display an item.
 * This is just a fragment template. It is intended to be included in
 * another template.
 *
 * Variables:
 *	$item - The item to display
 *}
{strip}
{* XXX - Perhaps add a 'feed=<feed_id>', so we can have different colors
 * for different feeds.
 *}
{* XXX - state="unread" in the next line will have to change when it's
 * possible to show read items.
 *}
<div class="item" id="item-{$item.id}" state="unread">
  {* This hidden field just lists the ID of an item that was displayed,
   * so that we can mark it with the "Mark all read" button.
   *}
  <input type="hidden" name="item-{$item.id}" value=""/>

  <table class="item-header">
    <tr>
      <td class="info">
        <h3 class="item-title">
          {if $item.url != ""}
            <a href="{$item.url}">
          {/if}
          {if ($item.title == "")}
            [no title]
          {else}
            {$item.title}
          {/if}
          {if $item.url != ""}
            </a>
          {/if}
        </h3>
        {if ($items != "")}
          <h3 class="feed-title">
            <a href="{$feeds[$item.feed_id].url}">
              {$feeds[$item.feed_id].title}
            </a>
          </h3>
        {/if}
        {if $item.author != ""}
          {* XXX - If ever add email address:
           * U+2709 == envelope; U+270d == writing hand
           *}
          <p class="item-author">by {$item.author|escape}</p>
        {/if}
        <span>{$item.pub_date|date_format:"%e %b, %Y"}</span>
      </td>
      <td class="icon-box">
{* XXX - Do something with categories *}
{* category: [{$item.category}]<br/>*}
       {* XXX - When showing read items, change this to "mark as unread" *}
        Mark as read:&nbsp;
        {* "cbt": checkbox top *}
        <input class="mark-check" type="checkbox" name="cbt-{$item.id}" value="1" onclick="javascript:mark_item(this)"/>
      </td>
    </tr>
  </table>

{* Four cases:
 * no summary, no content: not collapsible; set content to "left blank"
 * no summary,    content: not collapsible; show content.
 *    summary, no content: not collapsible; show summary, "More->" link.
 *    summary,    content:     collapsible; show content. With JS, add toggle bars.
 *}
  {if $item.summary == ""}
    {* No summary *}
    {if $item.content == ""}
      {* No summary, no content *}
      {assign var="collapsible" value="no"}
      {assign var="which" value="content"}
      {assign var="content" value="This space intentionally left blank"}
    {else}
      {* No summary,    content *}
      {assign var="collapsible" value="no"}
      {assign var="which" value="content"}
    {/if}
  {else}
    {* Summary *}
    {if $item.content == ""}
      {*    summary, no content *}
      {assign var="collapsible" value="no"}
      {assign var="which" value="summary"}
    {else}
      {*    summary,    content *}
      {assign var="collapsible" value="yes"}
      {assign var="which" value="content"}
    {/if}
  {/if}
  <div class="content-panes" collapsible="{$collapsible}" which="{$which}">
    <div class="collapse-bar"
         onclick="javascript:toggle_pane(this)">
      &#x25b2;{* Upward-pointing triangle *}
    </div>

    <div class="item-summary">
      {$item.summary}
      {* This is for items with floating elements in them (such as
       * tall images): make sure the image is contained within the
       * <div> and doesn't go overflowing where we don't want it.
       *}
      <br style="clear: both"/>
    </div>

    <div class="item-content">
      {$item.content}
      <br style="clear: both"/>
    </div>
    <div class="collapse-bar"
         onclick="javascript:toggle_pane(this)">
      &#x25b2;{* Upward-pointing triangle *}
    </div>
    <div class="expand-bar"
         onclick="javascript:toggle_pane(this)">
      &#x25bc;{* Downward-pointing triangle *}
    </div>
  </div>

  <table class="item-footer">
    <tr>
      <td class="bottom-link-box">
        <ul class="bottom-links">
          {if $item.summary != "" && $item.content == "" && $item.url != ""}
            <li><a href="{$item.url}">Read more</a></li>
          {/if}
          {if (isset($item.comment_url))}
            <li><a href="{$item.comment_url}">Comments</a></li>
          {/if}
          {if (isset($item.comment_rss))}
            <li><a href="{$item.comment_rss}">(feed)</a></li>
          {/if}
        </ul>
      </td>
      <td class="mark-td">
        {* XXX - When showing read items, change this to "mark as unread" *}
        Mark as read:&nbsp;
        {* "cbb": checkbox bottom *}
        <input class="mark-check" type="checkbox" name="cbb-{$item.id}" value="1" onclick="javascript:mark_item(this)"/>
      </td>
    </tr>
  </table>
</div>
{/strip}
