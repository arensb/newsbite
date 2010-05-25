CREATE INDEX `last_update` ON items (last_update);
CREATE INDEX `is_read` ON items (is_read);
UPDATE	`feeds`
  SET	last_update =
	(SELECT	IFNULL(MAX(items.pub_date),
			NOW())
	 FROM	items
	 WHERE	items.feed_id = feeds.id);
