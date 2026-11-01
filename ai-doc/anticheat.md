# Arena statistical anticheat

Code: `src/game/Arenacraft/anticheat/`. Table: `arena_anticheat_flag`
(characters db, `data/sql/updates/db_characters/2026_11_01_00.sql`). Built on top
of the replay system (`ai-doc/replay.md`).

Idea: don't chase injectors/signatures (those only catch mass-market cheats).
Watch what a cheater *does* that a human can't, flag the match, and let a GM
watch the replay and decide. Never auto-punishes.

## Enabling & config

Follows the replay master switch `AC_REPLAY_ENABLED`: a flag is only raised for a
match that produced a recording, so every flag is watchable. Thresholds:

| env                                        | default | meaning                                            |
|--------------------------------------------|---------|----------------------------------------------------|
| `AC_ANTICHEAT_ARENA_INTERRUPT_REACTION_MS` | `250`   | median interrupt reaction (ms into enemy cast) to flag at/below |
| `AC_ANTICHEAT_ARENA_MIN_INTERRUPT_SAMPLES` | `4`     | min interrupts before the median is judged         |

## Detectors

- **fast-interrupts**: each time a player interrupts an enemy player's cast-time
  generic spell, the reaction = how far into the cast it landed
  (`Spell::GetElapsedCastTime`, hooked in `Spell::EffectInterruptCast`). At match
  end, if the median over >= min samples is <= threshold, the player is flagged.
  A human reacting to a cast bar is a few hundred ms; a reaction/kick bot is
  near-constant and very low. Thresholds are lenient on purpose - a flag only
  asks for a look, so a false positive costs a GM one replay, while a miss lets a
  bot run.

New detectors: add a metric to `PlayerStats`, feed it from the relevant core
hook, and extend `evaluate()` (pure, unit-tested in `ArenaStats_test.cpp`).

## GM review (SEC_GAMEMASTER)

- `.arenacheck list` - open flags, newest first
- `.arenacheck view <id>` - detail + `.replay <replayId>` to watch
- `.arenacheck dismiss <id>` - mark handled (records the GM; no effect on the player)

Flags are deleted automatically when their replay is pruned (FK ON DELETE
CASCADE), so retention follows `AC_REPLAY_*`.

## Flow

`EffectInterruptCast` (map thread) -> `ArenaAnticheat::onInterrupt` collects a
sample keyed by bg instance + player. `Battleground::EndBattleground` ->
`ReplayRecorder::onArenaEnd` (saves replay, records its id) then
`ArenaAnticheat::onArenaEnd` evaluates, pulls the saved replay id
(`ReplayRecorder::takeSavedReplayId`) and inserts flags.
