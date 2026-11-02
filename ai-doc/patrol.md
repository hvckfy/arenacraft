# Patrol

Players review reported arena replays and decide on bans, without knowing who they are judging.
Code: `src/game/Arenacraft/patrol/` (`PatrolRules` = pure rules + tests, `Patrol` = service, `PatrolScripts` =
commands). Tables: `patrol_case`, `patrol_report`, `patrol_verdict` (characters db,
`data/sql/updates/db_characters/2026_11_02_02.sql`). Needs replays (`AC_REPLAY_ENABLED`, see `replay.md`).

## Flow

1. **Report** - `.replay report <replay id> <name> <category>`. Only a participant of that match may report, only
   another participant, at most 10 reports per account per day. One case per (replay, suspect); more reports add
   categories to it. A closed case can't be reported again.
2. **Accept** - `.patrol accept` picks a random open case the account has not judged, did not report and did not
   play in. Requires 50 solo queue season games (staff accounts exempt) and the usual replay-watching conditions.
   The case's replay is played; the replay id is never shown.
3. **Review** - every participant is renamed: the suspect is `Suspect`, the others `Player1`..`Player5`. The camera
   follows the suspect (`.patrol camera` frees it). `.replay leave` ends the replay early.
4. **Verdict** - when the replay ends the patroller answers one yes/no question per category
   (`.patrol yes` / `.patrol no`). `.patrol skip` passes on the case instead (recorded, never offered again, not
   counted).
5. **Decision** - after 5 verdicts the case closes. A category with 4+ "yes" bans the suspect's account for 7 days
   (`BanMgr`, author `Patrol`, reason `Patrol: <categories>`); otherwise the case is cleared.

Categories (`PatrolRules.hpp`): cheating (speed/teleport/fly/wall-walk hacks), automation (autokick, autofake,
auto-interrupt, bots, rotation scripts), griefing (throwing, AFK, feeding), win trading. Aliases such as `bot`,
`autokick`, `grief`, `wintrade` are accepted by the report command.

Staff: `.patrol cases` lists open cases with their verdict and report counts.

## Anonymity

- `WorldSession::SendNameQueryOpcode` answers with the alias for a reviewer (`patrol::AliasFor`); on replay start the
  aliases are also pushed unsolicited so names the client already cached are overwritten, and the real names are
  pushed back when the review ends.
- Guild names are not answered while reviewing (`HandleGuildQueryOpcode`). Pet names are unknown anyway (the recorded
  pets do not exist on the server).
- `.replay list` and `.replay <id>` are refused while reviewing.
- Not hidden: class, race and appearance; GUIDs (an addon that already knows a character's GUID could tell who it
  is - hiding that needs GUID rewriting in the recorded packets).

## Limits

- The replay is what the recording participant's client saw (see `replay.md`): a suspect stealthed to that client is
  not visible.
- Reviews live in memory: a logout drops an unfinished review (no verdict). A replay that fails to start keeps the
  review; `.patrol accept` retries it.
- Retention (`AC_REPLAY_KEEP_DAYS` / `MAX_TOTAL` / `MAX_PER_PLAYER`) skips recordings with an open case.
- Eligibility uses the soloq team's season games, so it resets with the season.
