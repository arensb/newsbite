DELIMITER //
DROP FUNCTION IF EXISTS `get_option`;
CREATE FUNCTION `get_option`
    (fid INT,
     opt CHAR(64))
    RETURNS	CHAR(255)
READS SQL DATA
DETERMINISTIC
BEGIN
	SET @retval = NULL;

	# Get the option value for this particular feed.
	SELECT	`value` into @retval
	FROM	`feed_options`
	WHERE	`feed_id` = fid
	  AND	`name` = opt;

	IF @retval IS NOT NULL THEN
	   RETURN @retval;
	END IF;

	SELECT	`value` into @retval
	FROM	`feed_options`
	WHERE	`feed_id` = -1
	  AND	`name` = opt;

	RETURN @retval;
END //
DELIMITER ;
