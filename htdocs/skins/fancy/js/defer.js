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
	 *
	 * defer(Number n, Object o, [Mixed a2...an])
	 * Defer execution of the function for 'n' milliseconds.
	 * If object 'o' is supplied, then the function is executed as
	 * a method on the object. Any additional argumetns 'a2'...'an'
	 * are passed to the function.
	 *
	 * To call 'foo(a,b,c)' in 300 ms:
	 *	foo.defer(300, null, a, b, c);
	 * To call myObj.theMethod(d,e,f) in 300 ms:
	 *	myObj.theMethod.defer(300, myObj, d, e, f);
	 */
	Function.prototype.defer =
		function(n,o) {
			// Get arguments as array
			var a = [];

			// The 2, below, comes from the fact that we
			// want to strip off the 'n' and 'o' arguments
			// we were given, an keep only the "proper"
			// arguments intended for the target function.
			for(var i=2; i<arguments.length; i++)
				a.push(arguments[i]);
			var that = this;
			window.setTimeout(function(){return that.apply(o,a);},n);
		};
}
#endif	// _defer_js_
