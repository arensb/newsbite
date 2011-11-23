<?
/* json.inc
 * Compatibility functions for hosts that don't have the JSON module.
 */

/* json_encode
 * Helper function for jsonify: takes one argument, and returns its
 * representation as a JSON string.
 */
function json_encode($it)
{

	/* At this point, $it is the thing we're converting. Decide
	 * how to handle it, depending on whether $it is a string, an
	 * assoc, or an array.
	 */
	$retval = "";
	if (!isset($it))
		$retval = "null";
	elseif (is_numeric($it))
		$retval = &$it;
	elseif (is_bool($it))
		$retval = ($it ? "true" : "false");
	elseif (is_string($it))
		/* Escape chars in $it */
		$retval = '"' .
			addcslashes($it, "\\\n\r\"") .
			'"';
	elseif (is_array($it))
	{
		if (array_key_exists("0", $it))
		{
			/* Output a JavaScript array */
			// We need to do the silly dance with $it[0]
			// to avoid having an extra comma in the
			// output.
			$retval = '[' . json_encode($it[0]);
			for ($i = 1; $i < count($it); $i++)
				$retval .= ',' . json_encode($it[$i]);
			$retval .= ']';
		} else {
			/* Output a JavaScript object */
			$keys = array();
			$retval = '{';
			foreach ($it as $key => $value)
				$keys[] = json_encode($key) . ':' . json_encode($value);
			$retval .= implode(',', $keys);
			$retval .= '}';
		}
	} else
		return FALSE;

	return $retval;
}
?>
