#include "ArenaAnticheat.hpp"

#include "Battleground.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Player.h"
#include "ReplayConfig.hpp"
#include "ReplayRecorder.hpp"

namespace arenacraft::anticheat
{
ArenaAnticheat& ArenaAnticheat::instance()
{
  static ArenaAnticheat anticheat;
  return anticheat;
}

void ArenaAnticheat::loadConfig()
{
  _thresholds.interruptReactionMs = sConfigMgr->GetOption<uint32>("Anticheat.Arena.InterruptReactionMs", 250, false);
  _thresholds.minInterruptSamples = sConfigMgr->GetOption<uint32>("Anticheat.Arena.MinInterruptSamples", 4, false);

  LOG_INFO("server.loading",
           "Arena anticheat: {} (interrupt median <= {}ms over {} samples flags a match). Follows Replay.Enabled.",
           replay::config().enabled ? "on" : "off", _thresholds.interruptReactionMs, _thresholds.minInterruptSamples);
}

void ArenaAnticheat::onInterrupt(Player* interrupter, Player* victim, uint32_t reactionMs)
{
  if (!replay::config().enabled || !interrupter || !victim)
    return;

  Battleground* bg = interrupter->GetBattleground();
  if (!bg || !bg->isArena() || victim->GetBattleground() != bg)
    return;
  // only enemy casts; teammates can't be interrupted in arena anyway, but guard
  if (interrupter->GetBgTeamId() == victim->GetBgTeamId())
    return;

  std::lock_guard<std::mutex> guard(_lock);
  PlayerStats&                stats = _matches[bg->GetInstanceID()][interrupter->GetGUID().GetCounter()];
  if (stats.guid == 0)
  {
    stats.guid = interrupter->GetGUID().GetCounter();
    stats.name = interrupter->GetName();
  }
  stats.interrupts.push_back({reactionMs});
}

void ArenaAnticheat::onArenaEnd(Battleground* bg)
{
  if (!bg)
    return;

  std::unordered_map<uint32_t, PlayerStats> players;
  {
    std::lock_guard<std::mutex> guard(_lock);
    auto                        itr = _matches.find(bg->GetInstanceID());
    if (itr == _matches.end())
      return;
    players = std::move(itr->second);
    _matches.erase(itr);
  }

  // A flag is only useful with a replay to watch; skip if the match was not
  // recorded (bracket disabled, or too short to keep).
  std::optional<uint32_t> const replayId = replay::ReplayRecorder::instance().takeSavedReplayId(bg->GetInstanceID());
  if (!replayId)
    return;

  CharacterDatabaseTransaction trans   = CharacterDatabase.BeginTransaction();
  bool                         anyFlag = false;
  for (auto const& [guid, stats] : players)
  {
    std::optional<Flag> const flag = evaluate(stats, _thresholds);
    if (!flag)
      continue;

    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ARENA_ACHECK_FLAG);
    stmt->SetData(0, *replayId);
    stmt->SetData(1, flag->guid);
    stmt->SetData(2, flag->name);
    stmt->SetData(3, flag->reason);
    stmt->SetData(4, flag->detail);
    trans->Append(stmt);
    anyFlag = true;

    LOG_INFO("bg.arena", "Arena anticheat: flagged {} ({}) - {} [replay #{}]", flag->name, flag->guid, flag->detail,
             *replayId);
  }

  if (anyFlag)
    CharacterDatabase.CommitTransaction(trans);
}
} // namespace arenacraft::anticheat
