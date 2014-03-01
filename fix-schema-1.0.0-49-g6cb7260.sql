CREATE TABLE counts (
	feed_id		INT		NOT NULL,
	total		INT,
	num_read	INT,
	PRIMARY KEY(feed_id)
)
DEFAULT CHARSET=utf8;

/* trig_add_feed
 * Trigger to add a row to `counts` when we add a new feed.
 */
CREATE TRIGGER trig_add_feed
AFTER INSERT ON feeds
FOR EACH ROW
	INSERT INTO counts
	SET	feed_id = NEW.id,
		total = 0,
		num_read = 0;

/* trig_drop_feed
 * Trigger: when we delete a feed, delete its row in `counts`.
 */
CREATE TRIGGER trig_drop_feed
AFTER DELETE ON feeds
FOR EACH ROW
	DELETE FROM counts
	WHERE	feed_id = OLD.id;

# Populate the `counts` table with initial data
INSERT INTO counts (feed_id, total, num_read)
SELECT
	feed_id,
	COUNT(*) AS total,
	SUM(IF(is_read,1,0)) AS num_read
FROM	items
GROUP BY feed_id;
