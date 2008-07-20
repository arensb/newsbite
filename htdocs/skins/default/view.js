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

collapse = toggle_pane;
expand   = toggle_pane;
