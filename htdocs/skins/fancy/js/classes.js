/* classes.js
 * Functions for manipulating elements' class lists.
 * Useful for triggering CSS changes.
 *
 * All of these functions take a DOM element with an attribute of the
 * form class="foo bar baz", and add/remove/manipulate the class list.
 */
#ifndef _classes_js_
#define _classes_js_

/* Initialize the class-manipulation convenience functions.
 *
 * If the browser supports classList, we want to use that. If not, use
 * compatibility wrappers.
 *
 * Unfortunately, I haven't found a way to figure out, at load time,
 * whether a given browser has classList or not. In Firefox, can use
 *	if ("classList" in Element.prototype)
 *		// Have classList
 *	else
 *		// Don't have classList
 * But this doesn't seem to work with Chrome or mobile Safari (it
 * returns false, even in browsers that support classList). The only
 * method that seems to work is to create an Element (or examine an
 * existing one) and see if it has classList. So instead of doing
 * initialization the obvious way, we add a DOMContentLoaded handler
 * (that gets run when document.body exists), then see if
 * document.body has classList.
 */
/* XXX - Can this be made to look more object-ish? Perhaps rewrite the
 * code that uses these functions to use the classList API, and create
 * a compatibility object for those browsers that don't support this.
 * See
 * https://github.com/eligrey/classList.js/blob/master/classList.js
 * for a compatibility shim.
 */
var is_in_class;
var add_class;
var remove_class;
var replace_class;
var toggle_class;

document.addEventListener("DOMContentLoaded", function() {
	// NB: this function unregisters itself at the end.

	// See if we have Element.classList, which makes all of the
	// following much simpler. See
	// https://developer.mozilla.org/en/DOM/element.classList
	var have_classList = (document.body.classList != undefined);

	/* is_in_class
	 * Returns true iff 'cls' is in the class list of 'elem'.
	 */
	is_in_class = have_classList ?
		// Use Element.classList.contains()
		function(elem, cls)
		{
			return elem.classList.contains(cls);
		}
	:
		// Backward-compatibility
		function(elem, cls)
		{
			var class_str = elem.className;

			if (class_str == undefined)
				// No class set
				return false;

			return class_str.match(new RegExp('(^|\\s)'+cls+'($|\\s)'));
		}
	;

	/* add_class
	 * Make sure 'cls' is on the class list of 'elem', adding it if
	 * necessary.
	 */
	add_class = have_classList ?
		function(elem, cls)
		{
			return elem.classList.add(cls);
		}
	:
		// Backward-compatibility
		function(elem, cls)
		{
			var class_str = elem.className;

			if (class_str == undefined)
				// No class set
				return false;

			var classes = class_str.split(" ");
			// Split class string into a list

			for (var i in classes)
			{
				if (classes[i] == cls)
					// Element already has this class
					return true;
			}

			// Need to add the class
			elem.className = classes.concat([cls]).join(" ");

			return true;	// Success
		}
	;

	/* remove_class
	 * Remove 'cls' from the class list of 'elem'.
	 */
	remove_class = have_classList ?
		function(elem, cls)
		{
			return elem.classList.remove(cls);
		}
	:
		function(elem, cls)
		{
			var class_str = elem.className;

		if (class_str == undefined)
			// No class set
			return false;

			var classes = class_str.split(" ");
			// Split class string into a list
			var new_classes = new Array();
			// The new class list

			for (var i in classes)
			{
				if (classes[i] == cls)
					// Skip over the class we're removing
					continue;
				new_classes.push(classes[i]);
				// Remember this other class
			}

			// The new class list is whatever we're left with.
			elem.className = new_classes.join(" ");

			return true;	// Success
		}
	;

	/* replace_class
	 * Replaces 'old_class' with 'new_class' in the class list of 'elem'.
	 * This is logically equivalent to
	 *	remove_class(elem, old_class);
	 *	add_class(elem, new_class);
	 * but is more efficient, since it combines the two.
	 */
	replace_class = have_classList ?
		function(elem, old_class, new_class)
		{
			elem.classList.remove(old_class);
			elem.classList.add(new_class);
		}
	:
		function(elem, old_class, new_class)
		{
			var class_str = elem.className;

			if (class_str == undefined)
				// No class set
				return false;

			var classes = class_str.split(" ");
			// Split class string into a list
			var new_classes = new Array();
			var seen_new_class = false;
			// Have we seen the new class already?

			for (var i in classes)
			{
				if (classes[i] == old_class)
					// Don't add old_class to new_classes
					continue;
				if (classes[i] == new_class)
					// Note that we've seen new_class on
					// the way
					seen_new_class = true;
				new_classes.push(classes[i]);
			}

			// If new_class is already on the class list, don't
			// add it a second time.
			if (!seen_new_class)
				new_classes.push(new_class);

			// The new class list is whatever we're left with.
			elem.className = new_classes.join(" ");

			return true;	// Success
		}
	;

	/* toggle_class
	 * If 'elem' has 'classA', replace that with 'classB', and vice-versa.
	 * This is logically equivalent
	 */
	// XXX - Use classList 
	toggle_class = have_classList ?
		function(elem, classA, classB)
		{
			if (elem.classList.contains(classA))
			{
				elem.classList.remove(classA);
				elem.classList.add(classB);
			} else if (elem.classList.contains(classB))
			{
				elem.classList.remove(classB);
				elem.classList.add(classA);
			}
			return true;		// Assume success
		}
	:
		function(elem, classA, classB)
		{
			var class_str = elem.className;

			if (class_str == undefined)
				// No class set
				return false;

			var classes = class_str.split(" ");
			// Split class string into a list
			var new_classes = new Array();
			var toggled = false;

			for (var i in classes)
			{
				if (classes[i] == classA)
				{
					// We've found classA
					if (toggled)
						// We've already seen classA
						// or classB. Don't add this
						// one a second time.
						continue;

					// We're toggling from classA to
					// classB, so put classB on the new
					// list of classes.
					new_classes.push(classB);
					toggled = true;
					// Remember that we've done this
					continue;
				}
				if (classes[i] == classB)
				{
					// We've found classB
					if (toggled)
						// We've already seen classA
						// or classB. Don't add this
						// one a second time.
						continue;

					// We're toggling from classB to
					// classA, so put classA on the new
					// list of classes.
					new_classes.push(classA);
					toggled = true;
					// Remember that we've done this
					continue;
				}
				new_classes.push(classes[i]);
			}

			// The new class list is whatever we're left with.
			elem.className = new_classes.join(" ");

			return true;	// Success
		}
	;

	// Remove this function from the DOMContentLoaded event
	// listener list. Might save space. Seems tidy.
	document.removeEventListener("DOMContentLoaded",
				     arguments.callee,	// self-reference
				     false);
}, false);	// End of DOMContentLoaded handler

#endif	// _classes_js_
