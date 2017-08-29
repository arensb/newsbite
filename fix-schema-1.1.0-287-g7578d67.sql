/* After an OS upgrade, MyISAM has decided to stop accepting UPDATE()
 * statements. I think because of triggers. See
 * https://bugs.mysql.com/bug.php?id=70879
 */
ALTER TABLE options ENGINE=InnoDB;
ALTER TABLE feed_options ENGINE=InnoDB;
ALTER TABLE groups ENGINE=InnoDB;
ALTER TABLE group_members ENGINE=InnoDB;
ALTER TABLE feeds ENGINE=InnoDB;
ALTER TABLE items ENGINE=InnoDB;
ALTER TABLE counts ENGINE=InnoDB;
