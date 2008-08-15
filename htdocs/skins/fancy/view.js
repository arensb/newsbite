debug_window = null;

function debug(str)
{
return;
	if (debug_window == null)
	{
		debug_window = window.open("",
					"Debugging Window",
					"height=400,width=600,scrollbars,menubar");
	}
	var body = debug_window.document.childNodes[1].childNodes[1];
	body.innerHTML += str + "<br/>\n";
}

function clrdebug()
{
	if (debug_window == null)
		return;
	var body = debug_window.document.childNodes[1].childNodes[1];
	body.innerHTML = "";
}

// XXX - This function shouldn't be replicated. Consolidate.
/* createXMLHttpRequest
 * Create a new XMLHttpRequest object, hopefully in a
 * browser-independent manner.
 */
function createXMLHttpRequest()
{
	var request = false;

	/* Firefox, Safari, etc. */
	if (window.XMLHttpRequest)
	{
		if (typeof XMLHttpRequest != 'undefined')
		{
			try {
				request = new XMLHttpRequest();
			} catch (e) {
				request = false;
				debug("Error allocating new XMLHttpRequest\n");
			}
		}
	} else if (window.ActiveXObject)
	{
		/* IE */
		/* Create a new ActiveX XMLHTTP object */
		try {
			request = new ActiveXObject('Msxml2.XMLHTTP');
		} catch (e) {
			request = false;
			debug("Error allocating ActiveX XMLHTTP\n");
		}
	}
	return request;
}

/* toggle-pane
 * Intended to be called from within
 * <div content-panes>
 *   <div {expand-bar|collapse-bar}>
 * The <div content-panes> is expected to have one <div item-summary>
 * and one <div item-content>.
 *
 * This function toggles the state of the <div content-panes>: if it
 * used to display the summary, it should now display the content, and
 * vice-versa.
 */
function toggle_pane(node)
{
	var my_pane;		// Pane containing the calling element
	var sib_class;	 	// Class of sibling we're looking for

	var container = node.parentNode;

	/* Go up until we find the <div content-panes> that contains
	 * both the <div item-summary> and the <div item-content>.
	 */
	while (container && (container.className != "content-panes"))
		;
	if (container == null)
		/* Something's wrong. Abort */
		return;

	/* Set the "which" attribute on the pane container. CSS does
	 * the rest: there are different rules for displaying expanded
	 * and collapsed articles.
	 */

	cont_state = container.getAttribute("which");
	if (cont_state == "summary")
		container.setAttribute("which", "content");
	else
		container.setAttribute("which", "summary");
}

/* load_articles
 * Load the articles that will be seen in this view.
 */
function load_articles()
{
debug(items.length + " items");
for (var i = 0; i < items.length; i++)
{
	debug("item " + i + ": [" + items[i] + "]");
}
//var item = document.getElementById("item-" + items[0]);

// XXX - Initialize cache: get the next 25, 50, 100, whatever items.
// Don't display until another is marked as read.

return;
	var request = createXMLHttpRequest();
	if (!request)
	{
		debug("Can't allocate XMLHttpRequest");
	}

	var err;

	// reqobj: an object encapsulating everything we want to keep
	// track of during this operation
	var reqobj = {
		request:	request,
		last_off:	0
		};

	request.open('GET',
		'view.php?id=' + feed_id + '&o=json',
		true);
	request.onreadystatechange = function() { parse_response(reqobj) };
	request.send('');

	return false;
}

function parse_response(req)
{
	debug("parse_response readyState: " + req.request.readyState);
	switch (req.request.readyState)
	{
	    case 0:		// Uninitialized
	    case 1:		// Loading
	    case 2:		// Loaded
		return;
	    case 3:		// Got partial text
		debug("Got some text. Len " + req.request.responseText.length);
		return;
	    case 4:		// Got all text
		break;
	}

	// XXX - Do something intelligent with the response text.
}
