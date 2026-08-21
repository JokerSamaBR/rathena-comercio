-- AutoAttack configuration schema migrated from new4th_Backup31122025.
-- Execute once in the rAthena database before using the AutoAttack item.

CREATE TABLE IF NOT EXISTS `aa_common_config` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `stopmelee` tinyint(1) NOT NULL DEFAULT 0,
  `pickup_item_config` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `prio_item_config` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `aggressive_behavior` tinyint(1) NOT NULL DEFAULT 0,
  `autositregen_conf` tinyint(1) NOT NULL DEFAULT 0,
  `autositregen_maxhp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `autositregen_minhp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `autositregen_maxsp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `autositregen_minsp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `tp_use_teleport` tinyint(1) NOT NULL DEFAULT 0,
  `tp_use_flywing` tinyint(1) NOT NULL DEFAULT 0,
  `tp_min_hp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `tp_delay_nomobmeet` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `tp_mvp` smallint(6) NOT NULL DEFAULT 0,
  `tp_miniboss` smallint(6) NOT NULL DEFAULT 0,
  `accept_party_request` tinyint(1) NOT NULL DEFAULT 1,
  `token_siegfried` tinyint(1) NOT NULL DEFAULT 1,
  `return_to_savepoint` tinyint(1) NOT NULL DEFAULT 1,
  `map_mob_selection` int(11) NOT NULL DEFAULT 0,
  `action_on_end` int(11) NOT NULL DEFAULT 0,
  `monster_surround` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `aa_items` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `type` smallint(5) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL,
  `min_hp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `min_sp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `delay` int(10) UNSIGNED NOT NULL DEFAULT 0,
  `status` int(10) UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`type`,`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `aa_mobs` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `mob_id` int(10) UNSIGNED NOT NULL,
  PRIMARY KEY (`char_id`,`mob_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `aa_skills` (
  `char_id` int(10) UNSIGNED NOT NULL,
  `type` smallint(5) UNSIGNED NOT NULL,
  `skill_id` smallint(5) UNSIGNED NOT NULL,
  `skill_lv` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  `min_hp` smallint(5) UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`type`,`skill_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
