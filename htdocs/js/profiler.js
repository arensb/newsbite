/* profiler.js
 * See end for an example.
 */

/* Profiler class
 * Records when functions are entered/left, and can produce a report
 * of where time was spent.
 */
function Profiler()
{
	this.stamps = [];
	this.t0 = new Date().getTime();	// Record when started
}

// Recorded events are tuples of the form [type, msg, time],
// where
// type is a type of event:
//	0: passing a breakpoint
//	1: entering a function
//	2: exiting a function
// msg is a message; for entering and exiting functions, this is the
//	name of the function.
// time is the time at which the event was recorded, in milliseconds
//	since the epoch.

Profiler.prototype.record = function(str)
{
	this.stamps.push([0,
			  str,
			  new Date().getTime()
			  ]);
}

Profiler.prototype.enter = function(funcname)
{
	this.stamps.push([1,
			  funcname,
			  new Date().getTime()
			  ]);
}

Profiler.prototype.leave = function(funcname)
{
	this.stamps.push([2,
			  funcname,
			  new Date().getTime()
			  ]);
}

var spaces = "                                                                                                    ";
Profiler.prototype.report = function()
{
	var box = document.getElementById("profiler");
	if (box == undefined)
		return;

	var totals = {};
	var context = [];

	for (var i = 0; i < this.stamps.length; i++)
	{
		var event = this.stamps[i];
			// [0]: type
			// [1]: function/breakpoint name
			// [2]: timestamp
		var msg = "";

		msg += spaces.substr(0, context.length*4) + msg;
			// Indentation

		switch (event[0])
		{
		    case 1:
			msg += "Enter " + event[1];

			// Push a tuple of the form {funcname, time}
			// onto the context stack.
			context.push([event[1], event[2]]);
			break;
		    case 2:
			msg += "Exit " + event[1];
			// Hopefully the last item on the context stack
			// is from when this function was called
			if (context[context.length-1][0] != event[1])
			{
				box.innerHTML += "*** Expected " + event[1] + ", got " + context[context.length-1] + "\n";
			}
			var starttime = context[context.length-1][1];
			var totaltime = event[2] - starttime;
			if (totals[event[1]] == undefined)
				totals[event[1]] = 0;
			totals[event[1]] += totaltime;
			msg += " (" + totaltime + " ms)";
			context.pop();
			break;
		    case 0:
		    default:
			msg += event[1];

			var frame = context[context.length-1];
			var lasttime = frame[2];
			if (lasttime == undefined)
				lasttime = frame[1];
			frame[2] = event[2];
			var totaltime = event[2] - lasttime;
			msg += " (" + totaltime + " ms)";

			break;
		}

		box.innerHTML += (event[2]-this.t0)/1000 + ": " + msg + "\n";
	}
	box.innerHTML += "\n** Totals **\n";
	for (var i in totals)
	{
		box.innerHTML += i + ": " + totals[i] + "\n";
	}
}

// Profiler.register
// Replace the named function with a wrapper that records when the
// function was called, calls it, records when the function exited,
// and returns the original function's return value.
// XXX - For now, we assume that all functions are global
Profiler.prototype.register = function(funcname)
{
	var oldfunc = window[funcname];
	var prototyper = this;

	window[funcname] = function()
	{
		prototyper.enter(funcname);
		var retval = oldfunc.apply(this, arguments);
		prototyper.leave(funcname);
		return retval;
	}
}

Profiler.prototype.register_object = function(obj)
{
	var prototyper = this;
	var obj_name = obj.toString();
		// XXX - Ought to get the name of the class, but AFAIK
		// the only way to do that is to parse obj.constructor
		// and get the "function ClassName" part.

	for (var f in obj)
	{
		try {
			if (typeof(obj[f]) != "function")
				continue;
		} catch(e) {
			continue;
		}

		var old_method = obj[f];
		var method_name = obj_name + "." + f;

		obj[f] = function()
		{
			prototyper.enter(method_name);
			var retval = old_method.apply(this, arguments);
			prototyper.leave(method_name);
			return retval;
		}
	}
}

Profiler.prototype.register_class = function(theclass)
{
	var prototyper = this;
	var obj_name = theclass.toString();

	for (var f in theclass.prototype)
	{
		try {
			if (typeof(theclass.prototype[f]) != "function")
				continue;
		} catch (e) {
			continue;
		}

		var old_method = theclass.prototype[f];
		var method_name = obj_name + "." + f;

		theclass.prototype[f] = function()
		{
			prototyper.enter(method_name);
			var retval = old_method.apply(this, arguments);
			prototyper.leave(method_name);
			return retval;
		}
	}
}

// Example:
//var p = new Profiler();

//// Mark all global functions for profiling.
//for (f in window)
//{
//	if (typeof(window[f]) == "function")
//	{
//		p.register(f);
//	}
//}

//// Or in a function:
//p.record("before request.send");
//	request.send(req_data);
//p.record("after request.send");
