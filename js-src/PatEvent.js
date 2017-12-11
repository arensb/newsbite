#ifndef _PatEvent_js_
#define _PatEvent_js_
/* XXX - Currently doesn't support changing an existing DOM node. This
 * matters because we might have
 *	<div class="active">
 *	  <div class="thing">
 *	</div>
 * and we might want to attach an event to elements that match
 * ".active .thing". But if at some point we change 'class="active"'
 * to 'class="inactive"', then the inner <div> will no longer match
 * ".active .thing", and the Right Thing to do would be to remove the
 * event. But that's a hard problem, especially since there's no way
 * to list the EventListeners attached to a given node.
 *
 * Perhaps could attach to each node a list of all event handlers.
 */
/* XXX - Key bindings
 *
 * Currently, this module does not support keypress events (for that,
 * see "keybindings.js"), because that's a hairy problem.
 *	To start with, to have an Emacs-like keypress model, we need a
 * Keymap object, which maps key definitions to handlers. This is
 * pretty simple: just have a hash with keys of the form
 * <modifiers>-<keycode> (e.g, "k", or "CS-r" for Ctrl-Shift-r) that
 * map to functions. Then attach a Keymap to each DOM node. The
 * "keydown" handler then looks at the event, figures out which key
 * was pressed (with all modifiers), looks it up in the appropriate
 * Keymap, and calls the user's function.
 *
 *	What I really want is focus-follows-mouse, so that I can wave
 * the mouse over an item and press a key to collapse it. But the DOM
 * doesn't support that; some element (initially the entire window)
 * has keyboard focus, and there's no way to tell where the mouse was
 * when a given key was pressed.
 *	To do this, we'd need something like the following:
 * var mymap = new Keymap();
 * mymap.define_key("C-r", do_refresh);
 * PatEvent.bind_keymap(".foo", mymap);
 *		// Binding a key to a DOM element is a two-step process:
 *		// define the keymap, and attach the keymap to the
 *		// element(s).
 *
 * bind_keymap() needs to attach _enter and _exit event handlers to
 * all of the DOM elements that match the selector ".foo" (if they
 * don't have one already, which is a hairy problem in itself). It
 * also adds a "keymap" property to each of these DOM elements.
 *	These _enter/_exit handlers simply update a variable that says
 * which element currently has the mouse in it (and, by implication,
 * which Keymap applies).
 *	An event listener on 'window' itself (at the very top) can
 * then listen for keypresses. It checks whether a relevant element
 * has the mouse in it, look up the relevant Keymap, and call the
 * appropriate user function.
 */
var PatEvent;

// Don't include this module twice, 'cos you'll get clobbered.
PatEvent = {
	bindings:	{},	// Table of event handlers, a
				// 2-dimensional hash:
				// bindings[selector][evtype][] => handler
	};
// We also add a DOMContentLoaded listener at the end, to
// initialize stuff.
// XXX - Is there a better way to initialize it?

/* _init
 * Initialize the PatEvent-related stuff.
 */
PatEvent._init = function()
{
	/* Find a version of matchesSelector() on this browser */
	/* XXX - This ought to go elsewhere, in a 'compat' module or
	 * something.
	 */
	if (Element.prototype.matchesSelector != null)
	{
		/* Everything's okay */
	} else if (Element.prototype.mozMatchesSelector != null)
	{
		/* Mozilla */
		Element.prototype.matchesSelector =
			Element.prototype.mozMatchesSelector;
	} else if (Element.prototype.webkitMatchesSelector)
		/* Webkit */
		Element.prototype.matchesSelector =
			Element.prototype.webkitMatchesSelector;
	// XXX - else... what?

	// Chrome doesn't support DOMAttrModified. Here's a fix
	// suggested by Stack Overflow:
	// http://stackoverflow.com/questions/1882224/is-there-an-alternative-to-domattrmodified-that-will-work-in-webkit
	// "AppleWebKit" should catch both Chrome and (Mobile) Safari,
	// which are the ones I care about.
	if (navigator.userAgent.toLowerCase().indexOf('applewebkit') > -1)
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
			e.initMutationEvent("DOMAttrModified",
					    true,
					    true,
					    null,
					    prev,
					    val,
					    name,
					    1);	// XXX - See http://help.dottoro.com/ljifcdwx.php
					        // 1 - Modification
					        // 2 - Addition
					        // 3 - Removal
					        // So need to examine
					        // old value to see
					        // what kind of change
					        // this is.
			this.dispatchEvent(e);
		}
	}

	// Remove initialization listener, for cleanliness
	document.removeEventListener("DOMContentLoaded", PatEvent._init, false);
}

/* _enter_handler (see also _exit_handler)
 * It's useful to be able to say "call this handler when the mouse
 * crosses over into element X, and call this other handler when the
 * mouse exits it". The obvious way to handle this is with "mouseover"
 * and "mouseout", but those are kinda broken in Firefox.
 *
 * Let's say you have
 *	<parent>
 *	  <child1>...</child1>
 *	  <child2>...</child2>
 *	</parent>
 * and "mouseover" and "mouseout" events attached to <parent>.
 *
 * If there's no padding inside <parent>, then "mouseover" will be
 * triggered on the child, and bubble up to the parent. Furthermore,
 * when the mouse moves from <child1> to <child2>, that'll trigger a
 * "mouseout" event to say that the mouse has exited <child1>, and a
 * "mouseover" event to say that the mouse has entered <child2>. If
 * the user's handler causes visual effects, this can be annoying and
 * distracting.
 *
 * So _enter_handler() is a wrapper function: it checks to make sure
 * that the mouse is moving from outside <parent> to inside (and not
 * from one child to another), and only calls the user's intended
 * handler when it ought to.
 */
PatEvent._enter_handler = function(ev, realhandler)
{
	var node = ev.currentTarget;

	// If going from one child to another, ignore this event.
	var from_elt = ev.relatedTarget;
	var to_elt = ev.target;

	// If the "to" node isn't one of our children, we can just
	// ignore this event.
	while (to_elt != node)
	{
		if (to_elt == null)
			return;
		to_elt = to_elt.parentNode;
	}
 
	// If we get this far, the "to" node is one of our children.
	// If the "from" node is also a child, then ignore this event.
	while (from_elt != null)
	{
		if (from_elt == node)
			return;
		from_elt = from_elt.parentNode;
	}

	realhandler(ev);
}

PatEvent._exit_handler = function(ev, realhandler)
{
	var node = ev.currentTarget;

	// If going from one child to another, ignore this event.
	var from_elt = ev.target;
	var to_elt = ev.relatedTarget;

	// If the "from" node isn't one of our children, we can just
	// ignore this event.
	while (from_elt != node)
	{
		if (from_elt == null)
			return;
		from_elt = from_elt.parentNode;
	}

	// If we get this far, the "from" node is one of our children.
	// If the "to" node is also a child, then ignore this event.
	while (to_elt != null)
	{
		if (to_elt == node)
			return;
		to_elt = to_elt.parentNode;
	}

	realhandler(ev);
}

/* XXX - Ought to allow capture events as well, for completeness. But
 * for this, need to store the capture flag in this.bindings, to keep
 * track of it.
 * Currently, there's a 'capture' placeholder argument, but it's
 * ignored.
 */
PatEvent.bind_event = function(root, evtype, selector, handler, capture)
{
	/* Convenience wrappers (see comments at _enter_handler). */
	if (evtype == "_enter")
	{
		var realhandler = handler;
		evtype = "mouseover";
		handler = function(ev) {
			PatEvent._enter_handler(ev, realhandler)
		};
	} else if (evtype == "_exit")
	{
		var realhandler = handler;
		evtype = "mouseout";
		handler = function(ev) {
			PatEvent._exit_handler(ev, realhandler)
		};
	}

	// Add an entry to this.bindings (creating any missing bits
	// along the way)
	// XXX - Perhaps add another level for 'capture'?
	if (this.bindings[selector] == null)
		this.bindings[selector] = {};
	if (this.bindings[selector][evtype] == null)
		this.bindings[selector][evtype] = new Array();

	this.bindings[selector][evtype].push(handler);

	// Add a listener to this element and its children.
	this._add_binding(root, evtype, selector, handler, false);
}

PatEvent._add_binding = function(node, evtype, selector, handler, capture)
{
	// Attach the event to the node, if it matches
	try {
		if (node.matchesSelector(selector))
			node.addEventListener(evtype, handler, capture);
	} catch (e) {
		console.error("Can't attach root listener:\n%o", e);
	}

	// Attach the event to any matching child nodes
	try {
		var allnodes = node.getElementsByTagName("*");
	} catch (e) {
		// Something failed. Screw it, I'm giving up.
		return;
	}
	for (var i = 0, len = allnodes.length; i < len; i++)
	{
		var n = allnodes[i];
		if (!n.matchesSelector(selector))
			continue;
		n.addEventListener(evtype, handler, capture);
	}

	// Add a DOMNodeInserted listener to this node, so we can find
	// out if it gets any new children.
	// XXX - Chrome complains about this:
	// [Violation] Added synchronous DOM mutation listener to a 'DOMNodeInserted' event. Consider using MutationObserver to make the page more responsive.
	node.addEventListener("DOMNodeInserted",
			      PatEvent._event_node_added,
			      false);

	// XXX - Listen for DOMAttrModified events
}

/* _event_node_added
 * Handler for DOMNodeInserted event.
 * See what new nodes have been inserted, and attach the events they
 * need.
 */
PatEvent._event_node_added = function(ev)
{
	var newnode = ev.target;

	// XXX - This is probably slow as molasses
	for (var sel in PatEvent.bindings)
	{
		for (var evtype in PatEvent.bindings[sel])
		{
			for (var h in PatEvent.bindings[sel][evtype])
				PatEvent._add_binding(newnode, evtype,
					     sel,
					     PatEvent.bindings[sel][evtype][h],
					     // XXX - Need to use the correct "capture" argument.
					     false);
		}
	}
}

// Initialize stuff when the DOM is ready.
// XXX - Is there a better way to do this initialization?
document.addEventListener("DOMContentLoaded", PatEvent._init, false);
#endif	// _PatEvent_js_
