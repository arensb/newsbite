{* item.tpl
 * Display an item.
 * This is just a fragment template. It is intended to be included in
 * another template.
 *
 * Variables:
 *	$item - The item to display
 *}
{strip}
<div class="item" id="item_{$item.guid}" style="border: 1px solid black">
  <div class="item_header">
    <a class="item_title" href="{$item.url}">
      {if ($item.title == "")}
        [no title]
      {else}
        {$item.title}
      {/if}
    </a><br/>
    {if isset($item.author)}
      by {$item.author}<br/>
    {/if}

    {if ($item.category != "")}
{* XXX - Do something with categories *}
{* category: [{$item.category}]<br/>*}
{* XXX - There should be a box of icons, like /. categories *}
    {/if}
{* guid: [{$item.guid}]<br/> *}
    {$item.pub_date|date_format:"%c"}
    &nbsp; (updated {$item.last_update|date_format:"%c"})
{* XXX - Do something with the state *}
    &nbsp; state: [{$item.state}]
{* Note that there are two groups of radio buttons per item: one group
 * at the top, and another at the bottom. These all have the same
 * radio group name: "state_{id}". Otherwise, confusion can arise if
 * the item is marked as read at the top, and unread at the bottom.
 * The values are "na", "ua", "ra" at the top (for new, unread, read)
 * and "nb", "ub", and "rb" at the bottom. The "a" and "b" are just
 * there because w3.org says that all the radio buttons in a group
 * should have different values.
 *}
    <br/>
    (New: <input type="radio" name="state_{$item.id}" value="na" />
     Unread: <input type="radio" name="state_{$item.id}" value="ua" />
     Read:<input type="radio" name="state_{$item.id}" value="ra" />
    )
    <br/>
  </div>{* item_header *}

{* XXX - If JavaScript is turned on, should have selectable tabs for the
 * summary and full content.
 *}
  {if ($item.summary != "")}
{*    <h5>Summary:</h5>*}
    <div class="item_summary">
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
    <div class="item_content">{$item.content}</div>
  {/if}

  <div class="item_footer">
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
    (New: <input type="radio" name="state_{$item.id}" value="nb" />
     Unread: <input type="radio" name="state_{$item.id}" value="ub" />
     Read:<input type="radio" name="state_{$item.id}" value="rb" />
    )
  </div>
</div>
{/strip}
