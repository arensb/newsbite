<?php
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Error</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/abort.css" media="all" />
</head>
<body id="abort-body">

<h1>Error</h1>

<p><?=htmlspecialchars($skin_vars['message'])?></p>

<hr/>

<ul>
  <li><a href="index.php">Feed index</a></li>
</ul>

</body>
</html>
