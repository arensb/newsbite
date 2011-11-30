/* keybindings.js
 * Implement keybindings.
 */
#ifndef _keybindings_js_
#define _keybindings_js_

/* XXX - Module: keybindings.js */
/* keytab
 * This is the main table for mapping keystrokes to functions.
 * It's actually a 5-dimensional array, with the first four dimensions
 * being booleans, keyed on the various modifier flags: Ctrl, Shift,
 * Meta, and Alt. The fifth dimension is the keycode found in key
 * events.
 */
var keytab = [];
for (var ctrl = 0; ctrl <= 1; ctrl++)
{
	keytab[ctrl] = [];
	for (var shift = 0; shift <= 1; shift++)
	{
		keytab[ctrl][shift] = [];
		for (var meta = 0; meta <= 1; meta++)
		{
			keytab[ctrl][shift][meta] = [];
			for (var alt = 0; alt <= 1; alt++)
			{
				keytab[ctrl][shift][meta][alt] = [];
			}
		}
	}
}

/* bind_key
 * Similar to define-key in Emacs. 'key' is a human-readable string
 * defining a key combination, and 'func' is a function to call when
 * that key is pressed.
 *
 * 'key' can be a letter, with optional modifiers:
 *	x		The letter 'x'
 *	X		Shift-X
 *	S-x		Shift-X
 *	M-x		Meta-X
 *	C-x		Ctrl-X
 *	A-x		Alt-X
 * Modifiers may be combined:
 *	M-S-x		Meta-Shift-X
 *	A-C-M-S-x	Alt-Ctrl-Meta-Shift-X
 * Unfortunately, order matters.
 */
function bind_key(key, func)
{
	var matches;

	/* Extract the key definition */
	matches = /^(A-)?(C-)?(M-)?(S-)?(.)/.exec(key);
		// XXX - Error-checking

	var alt   = (matches[1] ? true : false);
	var ctrl  = (matches[2] ? true : false);
	var meta  = (matches[3] ? true : false);
	var shift = (matches[4] ? true : false);
	var ltr   = matches[5];

	if (ltr.toLowerCase() != ltr)
		// Special case: "S-x" and "X" are the same thing.
		shift = true;

	/* Bind the key to the function */
	keytab[ctrl+0][shift+0][meta+0][alt+0][ltr.toUpperCase().charCodeAt()] = func;
}

/* Handle keys */
function handle_key(evt)
{
// evt: object KeyboardEvent
// originalTarget
// target
// currentTarget
// type (keyup)
// eventPhase (3)
// which (74 == ASCII J)
// ctrlKey (false)
// shiftKey (false)
// keyCode (74)
// metaKey (false)
// altKey (false)
// view (object Window)

	var func = keytab[evt.ctrlKey+0][evt.shiftKey+0][evt.metaKey+0][evt.altKey+0][evt.keyCode];
	if (func != undefined)
	{
		func(evt);
		// XXX - Should this also evt.prevent_default()?
		return;
	}
}

#endif	// _keybindings_js_
