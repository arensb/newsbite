# get_option function
# Get the value of option `opt` for feed with ID `fid`, and return it.
# If it's not explicitly set for the feed, get the default.
# If there's no default, return NULL.

DROP FUNCTION IF EXISTS `get_option`;
DELIMITER //
CREATE FUNCTION `get_option`
    (fid INT,
     opt CHAR(64))
    RETURNS	CHAR(255)
BEGIN
	SET @retval = NULL;

	# Get the option value for this particular feed.
	SELECT	`value` into @retval
	FROM	`feed_options`
	WHERE	`feed_id` = fid;

	IF @retval IS NOT NULL THEN
	   RETURN @retval;
	END IF;

	SELECT	`value` into @retval
	FROM	`feed_options`
	WHERE	`feed_id` = 0;

	RETURN @retval;
END //
DELIMITER ;
