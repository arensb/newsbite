/* Create the mandatory "All" group, with ID -1 */
INSERT INTO groups (name, parent) VALUES ("All", -1);
UPDATE groups SET id=-1 WHERE id=last_insert_id();

CREATE TABLE group_members (
	member		INT		NOT NULL,
	parent		INT		NOT NULL DEFAULT -1,
	UNIQUE KEY (member, parent)
);
