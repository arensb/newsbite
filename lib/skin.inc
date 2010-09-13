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
	}

	function display($tmpl)
	{
		$fname = "skins/$this->skin/$tmpl.php";
		if (file_exists($fname))
		{
			global $the_skin;
			global $skin_vars;
			$the_skin = $this;
			$skin_vars = $this->vars;
			include($fname);
		} else
			abort("Can't find template skins/$this->skin/$tmpl");
	}

	function _include($tmpl, $more_vars)
	{
		$fname = "skins/$this->skin/$tmpl.php";
		if (file_exists($fname))
		{
			global $the_skin;
			global $skin_vars;
			$the_skin = $this;
			$old_skin_vars = $skin_vars;
					// Save current skin variables

			$skin_vars = array_merge($this->vars, $more_vars);
			include($fname);
			$skin_vars = $old_skin_vars;
					// Restore skin variables
		} else
			abort("Can't find template skins/$this->skin/$tmpl");
	}

	function fetch($tmpl, $more_vars = NULL)
	{
		$fname = "skins/$this->skin/$tmpl.php";

		if (file_exists($fname))
		{
			global $the_skin;
			global $skin_vars;
			$the_skin = $this;
			$old_skin_vars = $skin_vars;
					// Save current skin variables

			if (isset($more_vars))
				$skin_vars = array_merge($this->vars, $more_vars);
			else
				$skin_vars = $this->vars;

			ob_start();
			include($fname);
			$retval = ob_get_contents();
			ob_end_clean();
			return $retval;
		} else
			return NULL;
	}
};
?>
