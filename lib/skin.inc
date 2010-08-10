<?php
/* skin.inc
 * Skin class. Provides a wrapper around Smarty, for handling skins.
 */
require_once("config.inc");
require_once(SMARTY_DIR . "Smarty.class.php");

class Skin extends Smarty
{
	public $skin = DEFAULT_SKIN;

	function __construct()
	{
		parent::__construct();

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

		$this->compile_dir	= SMARTY_PATH . "templates_c";
#		$this->config_dir	= SMARTY_PATH . "configs";
		$this->vars		= array();
		$this->register_function('jsonify', 'smarty_function_jsonify');
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

		$this->assign('skin', $this->skin);
		return true;		// Success
	}
# Methods:
# new()
# assign("var_name", $value)
# display("somepage.tpl")
# fetch("subpage.tpl")

	/* XXX - Temporary glue functions, for transition between
	 * Smarty and nont-Smarty.
	 */
	function assign($var, $value)
	{
		$this->vars[$var] = $value;
		parent::assign($var, $value);
	}

	function display($tmpl)
	{
		// XXX - See if $tmpl =~ s/tpl/php/ exists; if so,
		// include() it. Otherwise, use Smarty.
#		$fname = "skins/$this->skin/$tmpl";
		$fname = "skins/$this->skin/" .
			str_replace(".tpl", ".php", $tmpl);
echo "fname: [$fname]<br/>\n";
		if (file_exists($fname))
			include($fname);
		else
			parent::display($tmpl);
	}
}
?>
