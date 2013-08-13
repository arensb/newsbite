/* keybindings.js
 * Implement keybindings.
 */
#ifndef _keybindings_js_
#define _keybindings_js_
#define NEW_KEYTAB	1	/* Try the new one-dimensional keytab */

/* keytab
 * This is the main table for mapping keystrokes to functions.
 * It's actually a 5-dimensional array, with the first four dimensions
 * being booleans, keyed on the various modifier flags: Ctrl, Shift,
 * Meta, and Alt. The fifth dimension is the keycode found in key
 * events.
 */
#if NEW_KEYTAB
var keytab = {};
#else	// NEW_KEYTAB
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
#endif	// NEW_KEYTAB

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
#if NEW_KEYTAB
	var keystr =
		String.fromCharCode((ctrl  + 0 << 3) |
				    (shift + 0 << 2) |
				    (meta  + 0 << 1) |
				    (alt   + 0)) +
		"-" +
		ltr.toUpperCase();
//console.log("set keystr: ["+keystr+"]");
	keytab[keystr] = func;
#else	// NEW_KEYTAB
	keytab[ctrl+0][shift+0][meta+0][alt+0][ltr.toUpperCase().charCodeAt()] = func;
#endif	// NEW_KEYTAB
}

/* Handle keys */
/* XXX - According to Chrome's profile tool, this is the function
 * where we spend the most time. I wonder if the 5-dimensionality of
 * the keytab array is to blame (perhaps especially because four of
 * those dimensions are just binary).
 *
 * Maybe it'd be better to just have a one-dimensional array keyed on
 * either "C-S-123" for "ctrl-shift-keycode(123)", or
 * "<modifier>-<keycode>" or "<modifier>-<char>".
 *
 * <modifier> can be a four-character sequence representing ctrl,
 * shift, meta, and alt respectively, and indicating whether they're
 * on or off. Thus, the binding for "C-S-x" could be stored under
 * "1100-120" (ctrl yes, shift yes, meta no, alt no, charcode 120).
 *
 * Or, since the modifiers are binary, the lead character could be the
 * one with charCode 0x0c == 12.
 *
 * OTOH, I'm not convinced that it makes a whole lot of difference.
 */
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
	/* *Sigh* Keyboard event handling is apparently a mess.
	 * There's evt.keyCode, which says which key was pressed, and
	 * there's evt.charCode, which says which character that is,
	 * if that makes sense (e.g., "Shift" or "F12" doesn't
	 * correspond to a character). On top of which, keyCodes are
	 * inconsistent across browsers, and even versions of
	 * browsers, especially for things other than ASCII letters.
	 *
	 * According to
	 * https://developer.mozilla.org/en-US/docs/Web/API/event.keyCode
	 * either charCode or keyCode is set, never both; and only the
	 * "keypress" event (not "keydown" or "keyup" event) has
	 * charCode set.
	 *
	 * However, "evt.which" is supposed to give either the
	 * charCode or keyCode, as appropriate, and appears to exist
	 * in FireFox, Chrome, and Safari.
	 */

#if NEW_KEYTAB
	var keystr =
		String.fromCharCode((evt.ctrlKey  + 0 << 3) |
				    (evt.shiftKey + 0 << 2) |
				    (evt.metaKey  + 0 << 1) |
				    (evt.altKey   + 0)) +
		"-" +
		String.fromCharCode(evt.which).toUpperCase();
//console.log("get keystr: ["+keystr+"]");
	var func = keytab[keystr];
#else	// NEW_KEYTAB
	var func = keytab[evt.ctrlKey+0][evt.shiftKey+0][evt.metaKey+0][evt.altKey+0][evt.keyCode];
#endif	// NEW_KEYTAB

	if (func != undefined)
	{
		func(evt);
		return;
	}
}

#endif	// _keybindings_js_
