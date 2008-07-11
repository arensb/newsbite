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
		$this->compile_id	= "{$this->skin}-skin";
		$this->template_dir	= "skins/{$this->skin}";
		$this->compile_dir	= SMARTY_PATH . "templates_c";
		$this->cache_dir	= SMARTY_PATH . "cache";
		$this->config_dir	= SMARTY_PATH . "configs";
	}

	function display($template)
	{
		$this->assign('skin', $this->skin);
		parent::display($template);
	}
}
?>
