/* defer.js
 * Defines the defer() function, for browsers that don't have it.
 */
// Function.defer
// From http://www.jslab.dk/library/Function.defer
Function.prototype.defer =
  function(n,o) {
    // Get arguments as array
    var a = [];
    for(var i=2; i<arguments.length; i++)
      a.push(arguments[i]);
    var that = this;
    window.setTimeout(function(){return that.apply(o,a);},n);
  };
