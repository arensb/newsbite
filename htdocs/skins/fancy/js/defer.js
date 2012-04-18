/* defer.js
 * Defines the defer() function, for browsers that don't have it.
 */
#ifndef _defer_js_
#define _defer_js_
if (!("defer" in Function.prototype))
{
	/* Function.defer
	 * Put off executing a function until later, when the
	 * JavaScript engine is idle.
	 *
	 * From http://www.jslab.dk/library/Function.defer
	 */
	Function.prototype.defer =
		function(n,o) {
			// Get arguments as array
			var a = [];
			for(var i=1; i<arguments.length; i++)
				a.push(arguments[i]);
			var that = this;
console.debug("defer("+n+", "+o+")");
console.debug(a);
			window.setTimeout(function(){return that.apply(o,a);},n);
		};
}
#endif	// _defer_js_
