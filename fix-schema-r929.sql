/* We don't use ttl for anything */
ALTER TABLE	feeds
DROP COLUMN	ttl;

/* Add modification time to items */
ALTER TABLE	items
ADD COLUMN	mtime TIMESTAMP
AFTER		is_read;

UPDATE		items
SET		mtime = LEAST(NOW(),
			      GREATEST(pub_date, last_update));
			/* Initialize mtime to the later of pub_date
			 * or last_update. But if that's in the future,
			 * set it to now().
			 */
