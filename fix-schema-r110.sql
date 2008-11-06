ALTER TABLE	items
ADD COLUMN	is_read BOOLEAN
AFTER		state;

UPDATE		items
SET		is_read = IF(state = 'read' OR state = 'deleted',
			TRUE, FALSE);

ALTER TABLE	items
DROP COLUMN	state;
