DROP TABLE IF EXISTS `net`.`fa`;
CREATE TABLE  `net`.`fa` (
  `device` int(10) unsigned NOT NULL,
  `channel` int(10) unsigned NOT NULL,
  `sensor` int(10) unsigned NOT NULL,
  `section` int(10) unsigned NOT NULL,
  `status` int(10) unsigned NOT NULL,
  PRIMARY KEY (`device`,`channel`,`sensor`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;