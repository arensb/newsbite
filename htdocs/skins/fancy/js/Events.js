#ifndef _Events_js_
#define _Events_js_
/* Events.js
 * Interface for binding events to handlers, with selectors.
 */
document.addEventListener("DOMContentLoaded", init_Events, false);

var event_bindings = {};	// Table of event handlers, a
				// 2-dimensional hash:
				// event_bindings[evtype][selector] => handler

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
				  _event_handler(ev, event_bindings[evtype])
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
		if (node.matchesSelector(sel))
			// Found a handler. Call it.
			return table[sel](ev);
	}
	return null;
}
#endif	// _Events_js_
