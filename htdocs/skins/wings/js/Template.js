#ifndef _Template_js_
#define _Template_js_

function Template(tmpl)
{
	/* Look for sequences of the form @VAR@. These are the variables
	 * that will be replaced in the template.
	 * split() is perfect for this, since a template is by definition
	 * a series of strings punctuated by variables to be expanded.
	 * Thus we know that the even-numbered elements (0, 2, 4, ...) are
	 * strings, and the odd-numbered ones are names of variables.
	 */
	this.template = tmpl.split(/@(\w+)@/);
}

/* Template.expand
 * Takes an array of values (as an object) and expands the template.
 * Returns the result.
 */
Template.prototype.expand = function(values)
{
	var retval = "";
	var n = this.template.length;

	for (var i = 0; i < n; i += 2)
	{
		// We're looking at a plain string. Append it to retval
		retval += this.template[i];

		// See if this string is followed by a variable name.
		// If so, expand it.
		if (i+1 < n)
		{
			retval += values[this.template[i+1]];
		}
	}

	return retval;
}

#endif	// _template_js_
