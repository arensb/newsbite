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

		/* Pick skin */
		// XXX - Input checking: the cookie is untrusted user
		// input. Make sure it matches /^\w+$/ (or something
		// like that). In particular, since it's used as a
		// pathname, don't want it to get out of 'skins'
		// directory.
		if (isset($_COOKIE['skin']) &&
		    is_dir("skins/$_COOKIE[skin]"))
			$this->skin = $_COOKIE['skin'];
		$this->assign('skin', $this->skin);

		$this->compile_id	= "{$this->skin}-skin";
		$this->template_dir	= "skins/{$this->skin}";
		$this->compile_dir	= SMARTY_PATH . "templates_c";
		$this->config_dir	= SMARTY_PATH . "configs";
	}
}
?>
