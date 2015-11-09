-- Use utf8mb4 instead of utf8, since utf8 only supports characters
-- encoded as three bytes, whereas utf8mb4 goes all the way up to
-- four, and thus supports emoji.
ALTER TABLE options DEFAULT CHARACTER SET utf8mb4;
ALTER TABLE options CONVERT TO CHARACTER SET utf8mb4;

ALTER TABLE feed_options DEFAULT CHARACTER SET utf8mb4;
ALTER TABLE feed_options CONVERT TO CHARACTER SET utf8mb4;

ALTER TABLE groups DEFAULT CHARACTER SET utf8mb4;
ALTER TABLE groups CONVERT TO CHARACTER SET utf8mb4;

ALTER TABLE group_members DEFAULT CHARACTER SET utf8mb4;
ALTER TABLE group_members CONVERT TO CHARACTER SET utf8mb4;

ALTER TABLE feeds DEFAULT CHARACTER SET utf8mb4;
ALTER TABLE feeds CONVERT TO CHARACTER SET utf8mb4;

ALTER TABLE items DEFAULT CHARACTER SET utf8mb4;
ALTER TABLE items CONVERT TO CHARACTER SET utf8mb4;

ALTER TABLE counts DEFAULT CHARACTER SET utf8mb4;
ALTER TABLE counts CONVERT TO CHARACTER SET utf8mb4;
