#ifndef _Events_js_
#define _Events_js_
/* Events.js
 * Interface for binding events to handlers, with selectors.
 */
/* XXX - Bleah. This doesn't work. Redo.
 */
/* XXX - Potential problem (or security hole, or something):
 * If we have bind_event("click", ".foo", some_handler)
 * and a post includes <div class="foo">...</div>
 * then some_handler() will be invoked when the user clicks on that
 * div. Ought to have some way of setting up forbidden areas that will
 * not have handlers attached to them. Perhaps have
 *	dont_bind(".item .summary");
 *	dont_bind(".item .content");
 * to say not to attach any listeners to matching DOM nodes (or their
 * children).
 */
document.addEventListener("DOMContentLoaded", init_Events, false);

var event_bindings = {};	// Table of event handlers, a
				// 2-dimensional hash:
				// event_bindings[evtype][selector] => handler

/* init_Events
 * Initialize the Event-related stuff.
 */
/* XXX - Should this be made a constructor of some kind, or invoked
 * automatically?
 */
function init_Events()
{
	/* Find a version of matchesSelector() on this browser */
	if (Element.prototype.matchesSelector != null)
		/* Everything's okay */
		;
	else if (Element.prototype.mozMatchesSelector != null)
	{
		Element.prototype.matchesSelector =
			Element.prototype.mozMatchesSelector;
	} else if (Element.prototype.webkitMatchesSelector)
		Element.prototype.matchesSelector =
			Element.prototype.webkitMatchesSelector;
	// XXX - else... what?

	// Chrome doesn't support DOMAttrModified. Here's a fix
	// suggested by Stack Overflow:
	// http://stackoverflow.com/questions/1882224/is-there-an-alternative-to-domattrmodified-that-will-work-in-webkit
	if (navigator.userAgent.toLowerCase().indexOf('chrome') > -1)
	{
		// Replace the old Element.setAttribute() method with
		// a wrapper that'll invoke the old method, then
		// trigger an event.
		Element.prototype._setAttribute =
			Element.prototype.setAttribute;
		Element.prototype.setAttribute = function(name, val)
		{ 
			var e = document.createEvent("MutationEvent"); 
			var prev = this.getAttribute(name); 
			this._setAttribute(name, val);
			/* XXX - What's that 2 (attrChangeArg)?
			 * Firefox uses 1.
			 */
			e.initMutationEvent("DOMAttrModified",
					    true,
					    true,
					    null,
					    prev,
					    val,
					    name,
					    2);
			this.dispatchEvent(e);
		}
	}
}

/* bind_event
 * Bind an event of type 'evtype' (e.g., "click", "keydown",
 * "mouseover", etc.), to elements that match the given 'selector'
 * (e.g., "table.mystuff div *" or ".clickable") to the event handler
 * 'handler'.
 * 'handler' will be called with one argument: the event.
 */
function bind_event(evtype, selector, handler)
{
	if (event_bindings[evtype] == null)
	{
		// We don't have a handler for event 'evtype'. Create
		// one.
		event_bindings[evtype] = {
		};
		event_bindings[evtype][selector] = handler;
		document.addEventListener(evtype,
			  function(ev) {
				  return _event_handler(ev, event_bindings[evtype])
			  },
			  false);
	}
}

/* _event_handler
 * 'ev' is the event that occurred. 'table' is the subtable of
 * 'event_bindings' for that particular event.
 */
/* XXX - Should it be possible to bind multiple handlers to an
 * element? that is, if we have
 *	bind_event("click", ".foo", do_foo);
 *	bind_event("click", ".bar", do_bar);
 * and
 *	<button class="foo bar">
 * should both do_foo and do_bar be triggered?
 *
 * Or, perhaps more plausibly, if the selectors are ".container *" and
 * ".element" (where .element is inside .container). Should both
 * handlers be triggered?
 * Ideally, ought to call the more specific one, and allow it to
 * either preventDefault() or bubble up to the less-specific one, but
 * that seems a huge wheel to reinvent.
 */
function _event_handler(ev, table)
{
	var node = ev.target;	// Node that the event occurred on

	/* Find the first selector that matches the node */
	for (var sel in table)
	{
//msg_add("checking selector "+sel);
		if (node.matchesSelector(sel))
{//msg_add("match: "+sel);
			// Found a handler. Call it.
			return table[sel](ev);
}
	}
	return null;
}
#endif	// _Events_js_
