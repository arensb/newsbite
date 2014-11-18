/* Create the mandatory "All" group, with ID -1 */
INSERT INTO groups VALUES (-1, -1, "All");

/* Hack to make auto-increment work. See schema.sql. */
INSERT INTO groups (name) VALUES ("dummy");
DELETE FROM groups WHERE name="dummy";

CREATE TABLE group_members (
	member		INT		NOT NULL,
	parent		INT		NOT NULL DEFAULT -1
)
