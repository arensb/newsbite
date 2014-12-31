<?php
echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";

// Give some of the skin variables shorter names
global $skin_dir;
$skin_dir = $skin_vars['skin'];

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Editing groups</title>
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/<?=$skin_dir?>/editgroups.css" media="all" />
<!-- Template for tree of groups -->
<template id="groupentry">
  <li id="group_@GID@" class="group-entry">
    <label id="groupname_@GID@">@GROUPNAME@</label>
    <button class="edit-group-button">Edit</button>
    <button class="delete-group-button">Delete</button>
    <div class="child-groups" id="children_@GID@"></div>
  </li>
</template>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="skins/<?=$skin_dir?>/group.js"></script>
</head>
<body id="edit-group">

<? /* XXX - Links to get back to interesting places, like feed list */ ?>
<h1>Groups</h1>

<form name="edit-groups" method="post" action="group.php">
<input type="hidden" name="command" value="<?=$skin_vars['command']?>"/>
<ul id="group-tree"></ul>
<hr/>

<input type="reset" value="Clear changes"/>
<input type="submit" name="change" value="Apply changes"/>
</form>

<h2>Add a group</h2>
<!-- Yeah, maybe it's just easier to have a separate form for adding groups.
' -->
<!-- <form name="add-group" method="post" action="groups.php"> -->
<form id="add-group-form" name="add-group">
  <input name="command" type="hidden" value="add"/>
  Group name: <input name="name" type="text" size="20"/><br/>
<!-- XXX - parent -->
  <input name="parent" type="hidden" value="-1"/>
  <input name="add" type="submit" value="Add group"/>
</form>

</body>
</html>
