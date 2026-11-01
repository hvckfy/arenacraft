-- Arena statistical anticheat flags (src/game/Arenacraft/anticheat). One row per
-- suspicious player per match, raised for GM review only - never an automatic
-- punishment. `replay_id` points at the arena_replay recording so a GM can watch
-- the match with `.replay <id>`; follows the replay system's master switch.
CREATE TABLE IF NOT EXISTS `arena_anticheat_flag` (
  `id`         int unsigned NOT NULL AUTO_INCREMENT,
  `replay_id`  int unsigned NOT NULL,
  `guid`       int unsigned NOT NULL COMMENT 'flagged character guid',
  `name`       varchar(12)  NOT NULL,
  `reason`     varchar(32)  NOT NULL COMMENT 'detector slug, e.g. fast-interrupts',
  `detail`     varchar(128) NOT NULL COMMENT 'human-readable metric summary',
  `created_at` datetime     NOT NULL,
  `status`     enum('open','dismissed') NOT NULL DEFAULT 'open',
  `handled_by` varchar(24)  DEFAULT NULL COMMENT 'GM who dismissed it',
  `handled_at` datetime     DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_status` (`status`),
  KEY `idx_guid` (`guid`),
  KEY `idx_replay` (`replay_id`),
  CONSTRAINT `fk_arena_acheck_replay` FOREIGN KEY (`replay_id`) REFERENCES `arena_replay` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Arena anticheat flags for GM review';
