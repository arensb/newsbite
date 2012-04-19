/* We don't use ttl for anything */
ALTER TABLE	feeds
DROP COLUMN	ttl;

/* Add modification time to items */
ALTER TABLE	items
ADD COLUMN	mtime TIMESTAMP
AFTER		is_read;

UPDATE		items
SET		mtime = greatest(pub_date, last_update);
