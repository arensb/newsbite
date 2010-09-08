<?php
/* skin.inc
 * Skin class. Displays a page using a given skin.
 */
require_once("config.inc");

class Skin
{
	public $skin = DEFAULT_SKIN;

	function __construct()
	{
		$this->vars		= array();

		/* Pick skin.
		 * For security, make sure the value of the cookie is
		 * sensible (/^\w+$/) and that it's a directory
		 * underneath "skins".
		 */
		if (isset($_GET['skin']))
			$this->setskin($_GET['skin']);
		elseif (isset($_COOKIE['skin']))
			$this->setskin($_COOKIE['skin']);
		else
			$this->setskin($this->skin);
	}

	/* setskin
	 * Set the skin value. This identifies a directory underneath
	 * "skins/" which contains the templates for the skin.
	 */
	function setskin($newskin)
	{
		if (preg_match('/^\w+$/', $newskin) &&
		    is_dir("skins/$newskin"))
		{
			$this->skin = $newskin;
			$this->compile_id	= "{$this->skin}-skin";
			$this->template_dir	= "skins/{$this->skin}";
		} else
			return false;	// Failure

		$this->vars['skin'] = $newskin;
		$this->assign('skin', $this->skin);
		return true;		// Success
	}
# Methods:
# new()
# assign("var_name", $value)
# display("somepage.tpl")
# fetch("subpage.tpl")

	/* XXX - Temporary glue functions, for transition between
	 * Smarty and non-Smarty.
	 */
	function assign($var, $value)
	{
		$this->vars[$var] = $value;
#		parent::assign($var, $value);
	}

	function display($tmpl)
	{
		// XXX - See if $tmpl =~ s/tpl/php/ exists; if so,
		// include() it. Otherwise, use Smarty.
		$fname = "skins/$this->skin/" .
			str_replace(".tpl", ".php", $tmpl);
		if (file_exists($fname))
		{
			global $the_skin;
			global $skin_vars;
			$the_skin = $this;
			$skin_vars = $this->vars;
			include($fname);
		} else
			parent::display($tmpl);
	}

	function _include($tmpl, $more_vars)
	{
#echo "Inside _include($tmpl)<br/>\n";
		// XXX - See if $tmpl =~ s/tpl/php/ exists; if so,
		// include() it as a string. Otherwise, use Smarty.
		$fname = "skins/$this->skin/" .
			str_replace(".tpl", ".php", $tmpl);
#echo "fname (", getcwd(), "): [$fname]<br/>\n";
		if (file_exists($fname))
		{
#echo "Yes, it exists<br/>\n";
			global $the_skin;
			global $skin_vars;
			$the_skin = $this;
			$old_skin_vars = $skin_vars;
					// Save current skin variables

			$skin_vars = array_merge($this->vars, $more_vars);
#echo "Including [$fname]<br/>\n";
//			ob_start();		// Turn on output buffering
			include($fname);
//			$retval = ob_get_contents();
//						// Get output buffer
//			ob_end_clean();		// End output buffering
			$skin_vars = $old_skin_vars;
					// Restore skin variables
//			return $retval;
		} else
			parent::_include($tmpl);
	}
};
?>
