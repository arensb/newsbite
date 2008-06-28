function collapse(node)
{
	var my_pane;		// Pane containing the calling element
	var sib_class;	 	// Class of sibling we're looking for

	var container = node.parentNode;

	/* Go up until we find the <div content-panes> that contains
	 * both the <div item-summary> and the <div item-content>.
	 */
	while (container && (container.className != "content-panes"))
	{
		/* On the way down, see what kind of node we're inside:
		 * "item-summary" or "item-content".
		 */
		if (my_pane == null &&
		    (container.className == "item-summary"))
		{
			my_pane = container;
			sib_class = "item-content";
		} else if (my_pane == null && 
			   (container.className == "item-content"))
		{
			my_pane = container;
			sib_class = "item-summary";
		}

		container = container.parentNode;
	}
	if (container == null)
		/* Something's wrong. Abort */
		return;

	/* Find the sibling <div item-content> */
	for (var sib = container.firstChild;
	     sib != null;
	     sib = sib.nextSibling)
	{
		if (sib == my_pane)
		{
			sib.style.display = "none";
			continue;
		}
		if (sib.className == sib_class)
		{
			sib.style.display = "block";
		}
	}
}

expand = collapse;
