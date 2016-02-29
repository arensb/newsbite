<?php
/* loadopml.php
 * Load an OPML subscription list.
 */
$out_fmt = "html";
require_once("common.inc");
require_once("database.inc");

if (!isset($_FILES['opml']) || $_FILES['opml'] == ""):
	// Prompt for an OPML file
########################################
	echo '<', '?xml version="1.0" encoding="UTF-8"?', ">\n";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>NewsBite: Load OPML</title>
<link rel="stylesheet" type="text/css" href="css/style.css" media="all" />
<link rel="stylesheet" type="text/css" href="css/opml.css" media="all" />
<script type="text/javascript" src="js/jquery.js"></script>
<meta name="theme-color" content="#8080c0" />
</head>
<body>
<h1>Upload OPML File</h1>

<p>Select an OPML file to upload.</p>

<input type="file" name="opml-file" id="opml-file"/>

<script type="text/javascript">
// XXX - Move this code to a .js file.

// XXX - Something to let the user know what's going on, and when an
// OPML file has been successfully uploaded.

// upload_opml_file
// Take a File objects, reads the corresponding file, and sends a POST
// REST request.
function upload_opml_file(f)
{
	// send_opml_rest
	// Perform the actual REST call to send the file.
	function send_opml_rest(ev)
	{
		console.log("Inside send_opml_rest ", ev);
		var contents = ev.target.result;
		//console.debug("file contents: ", contents);

		// XXX - Is it worth checking that this is a proper
		// OPML file, or at least an XML file, or at least has
		// a reasonable header?

		// XXX - Add a library for this sort of thing.
		var req = new XMLHttpRequest();
		req.open("POST", "w1/opml", true);
		// XXX - Why is "PUT" not allowed?
		req.setRequestHeader("Content-Type",
				     "text/xml; charset=UTF-8");
		req.send(contents);
			// XXX - Error-checking.
	}

	if (!f instanceof File)
	{
		console.error("upload_opml_file was not given a proper file: ", f);
		// XXX - Alert the user?
		return;
	}
	var reader = new FileReader();
	reader.onload = send_opml_rest;
	reader.readAsText(f);
}

$(document).ready(function(){
	$("#opml-file").on("change", function(ev) {
		// When the user uploads a file, send it to the server.
		upload_opml_file(ev.target.files[0]);
	});
});
</script>
</body>
</html>
<?php
########################################
	exit(0);
endif;
?>
