#pragma once

#include "ArenaStats.hpp"

#include <cstdint>
#include <mutex>
#include <unordered_map>

class Battleground;
class Player;

namespace arenacraft::anticheat
{
// Collects per-match arena signals and, at match end, turns anything inhuman
// into a flag for GM review (arena_anticheat_flag). It never punishes - a flag
// just points a GM at the replay of a suspicious match.
//
// Gated by the replay system's master switch: flags are only produced for
// matches that got a recording, so every flag is watchable with `.replay`.
//
// Interrupt samples arrive from Spell::EffectInterruptCast (map threads); match
// end from Battleground::EndBattleground (map thread). Hence the lock.
class ArenaAnticheat
{
public:
  static ArenaAnticheat& instance();

  // World thread; reads config thresholds.
  void loadConfig();

  // A player landed an interrupt on an enemy player's cast-time spell
  // `reactionMs` into it. Only meaningful in rated arenas.
  void onInterrupt(Player* interrupter, Player* victim, uint32_t reactionMs);

  // Match ended: evaluate collected stats, store flags, drop the match.
  void onArenaEnd(Battleground* bg);

  [[nodiscard]] Thresholds const& thresholds() const { return _thresholds; }

private:
  ArenaAnticheat() = default;

  std::mutex _lock;
  // bg instance id -> (player guid -> stats)
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, PlayerStats>> _matches;
  Thresholds                                                              _thresholds;
};
} // namespace arenacraft::anticheat
