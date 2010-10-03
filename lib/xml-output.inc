<?php
/* xml-output.inc
 * Functions for sending XML to the client.
 * Rarely if ever used; moved to their own file to make common.inc
 * smaller and faster.
 */

/* xmlify
 * Convert the argument to an XML document, and return it.
 */
function xmlify($arg, $name="contents")
{
	$retval = '<'.'?xml version="1.0" encoding="utf-8"?'.">\n";
	// XXX - For now, assume the argument is an assoc.
	$retval .= "<$name>\n" . xmlify1($arg) . "</$name>\n";
	return $retval;
}

function xmlify1($it)
{
	if (!isset($it))
		;
	elseif (is_numeric($it))
		$retval .= $it;
	elseif (is_bool($it))
		$retval .= ($it ? "true" : "false");
	elseif (is_string($it))
		$retval .= '<![CDATA[' .
			htmlspecialchars($it) .		// XXX - Escape chars
			']]>';
	elseif (is_array($it))
	{
		if (array_key_exists("0", $it))
		{
			/* Output an array */
			foreach ($it as $i)
			{
#echo "xmlifying [$i]\n";
				$retval .= "<item>\n" .
					xmlify1($i) .
					"</item>\n";
#echo "retval == [$retval]\n";
			}
		} else {
			/* Output a series of tags */
			// XXX
			foreach ($it as $key => $value)
			{
//				if (!isset($value))
//					continue;
				$retval .= "<$key>" .
					xmlify1($value) .
					"</$key>\n";
			}
		}
	} else
		return FALSE;

	return $retval;
}

/* print_xml
 * Like xmlify(), this converts $arg to XML, but it doesn't bother
 * converting it to a string; it just prints the result.
 */
function print_xml($arg, $name="contents")
{
	echo  '<','?xml version="1.0" encoding="utf-8"?',">\n";
	// XXX - For now, assume the argument is an assoc.
	echo "<$name>\n";
	print_xml1($arg);
	echo "</$name>\n";
}

function print_xml1($it)
{
	if (!isset($it))
		return TRUE;
	if (is_numeric($it))
	{
		echo $i;
		return TRUE;
	}
	if (is_bool($it))
	{
		echo ($it ? "true" : "false");
		return TRUE;
	}
	if (is_string($it))
	{
		echo '<![CDATA[',
			htmlspecialchars($it),		// XXX - Escape chars
			']]>';
		return TRUE;
	}
	if (is_array($it))
	{
		if (array_key_exists("0", $it))
		{
			/* Output an array */
			foreach ($it as $i)
			{
				echo "<item>\n";
				print_xml1($i);
				echo "</item>\n";
			}
		} else {
			/* Output a series of tags */
			foreach ($it as $key => $value)
			{
				if (!isset($value))
					continue;
				echo "<$key>";
				print_xml1($value);
				echo "</$key>\n";
			}
		}
		return TRUE;
	}
	return FALSE;
}
?>
