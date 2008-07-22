<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Load OPML</title>
<link rel="stylesheet" type="text/css" href="skins/{$skin}/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="skins/{$skin}/opml.css" media="all" />
</head>
<body>

<form enctype="multipart/form-data" action="loadopml.php" method="post">
  <!-- 100,000 chars should be enough for ~500 feeds -->
  <input type="hidden" name="max_file_size" value="100000" />
  OPML file:
  <input type="file" name="opml"/>
  <br/>
  <input type="submit" name="doit" value="Upload file"/>
</form>

</body>
</html>
