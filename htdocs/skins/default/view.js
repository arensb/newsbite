/* collapse
 * Intended to be called from within
 * <div content-panes>
 *   <div {item-summary|item-content}>
 *     <div {expand-bar|collapse-bar}>
 * The <div content-panes> is expected to have one <div item-summary>
 * and one <div item-content>.
 *
 * This function makes the current pane invisible, and the other one
 * visible.
 */
// XXX - Actually, this is misnamed, since it implements both collapse()
// and expand(). Perhaps it should be toggle_pane() or some such.
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

expand = collapse;
