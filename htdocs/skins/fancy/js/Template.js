#ifndef _Template_js_
#define _Template_js_

function Template(tmpl)
{
	this.template = tmpl.replace(/>\s+</g, "><");
			// Text of the template. Try to strip out
			// extraneous whitespace.
}

/* Template.expand
 * Takes an array of values (as an object) and expands the template.
 * Returns the result.
 */
Template.prototype.expand = function(values)
{
try {
	return this.template.replace(/@(\w+)@/g,
		       function(dummy, match) {
			       if (values[match] == undefined)
				       return "";
			       return values[match];
		       });
} catch (e) {
console.error("Error in Template.expand: "+e);
console.trace();
}
}

#endif	// _template_js_
