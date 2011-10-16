#ifndef _Template_js_
#define _Template_js_

function Template(tmpl)
{
	this.template = tmpl;	// Text of the template
}

/* Template.expand
 * Takes an array of values (as an object) and expands the template.
 * Returns the result.
 */
Template.prototype.expand = function(values)
{
	return this.template.replace(/@(\w+)@/g,
		       function(dummy, match) {
			       if (values[match] == undefined)
				       return "";
			       return values[match];
		       });
}

#endif	// _template_js_