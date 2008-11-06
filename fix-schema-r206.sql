ALTER TABLE	feeds
ADD COLUMN	active BOOLEAN
AFTER		image;

UPDATE		feeds
SET		active = TRUE;
