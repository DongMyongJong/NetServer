DROP TABLE IF EXISTS `net`.`fa_sensor`;
CREATE TABLE  `net`.`fa_sensor` (
  `device` int(10) unsigned NOT NULL,
  `channel` int(10) unsigned NOT NULL,
  `section` int(10) unsigned NOT NULL,
  `sensor` int(10) unsigned NOT NULL,
  `status` int(10) unsigned NOT NULL,
  PRIMARY KEY (`device`,`channel`,`sensor`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `net`.`fa_device`;
CREATE TABLE  `net`.`fa_device` (
  `device` int(10) unsigned NOT NULL,
  `status` int(10) unsigned NOT NULL,
  PRIMARY KEY (`device`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;