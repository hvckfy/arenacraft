#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace arenacraft::anticheat
{
// Tunables for the arena statistical anticheat. Loaded from config
// (ArenaAnticheat.cpp); defaults are deliberately lenient - a flag only asks a
// GM to watch the replay, it never punishes, so a few false positives are fine
// and missing a blatant bot is worse.
struct Thresholds
{
  // A player is flagged when the MEDIAN of their interrupt-reaction samples is
  // at or below this, over at least `minInterruptSamples` samples. Reaction is
  // how far into an enemy's cast (ms) the interrupt landed - a human reacting
  // to a cast bar is a few hundred ms, a reaction bot is near-constant and very
  // low.
  uint32_t interruptReactionMs = 250;
  uint32_t minInterruptSamples = 4;

  bool operator==(Thresholds const&) const = default;
};

// One interrupt a player landed on an enemy player's (cast-time) spell.
struct InterruptSample
{
  uint32_t reactionMs = 0;

  bool operator==(InterruptSample const&) const = default;
};

// Per-player, per-match collected signals.
struct PlayerStats
{
  uint32_t                     guid = 0;
  std::string                  name;
  std::vector<InterruptSample> interrupts;
};

// A produced suspicion. Stored in arena_anticheat_flag for GM review; `detail`
// is a short human-readable metric summary.
struct Flag
{
  uint32_t    guid = 0;
  std::string name;
  std::string reason; // short slug, e.g. "fast-interrupts"
  std::string detail; // e.g. "median 80ms over 6 interrupts"

  bool operator==(Flag const&) const = default;
};

// Pure evaluation: given one player's match stats and the thresholds, decide
// whether to raise a flag. Unit-tested (ArenaStats_test.cpp); no engine deps so
// the detection rules can change without touching capture/DB code.
std::optional<Flag> evaluate(PlayerStats const& stats, Thresholds const& thresholds);

// Median of a sample list, rounded. Returns 0 for an empty list.
uint32_t medianReaction(std::vector<InterruptSample> const& samples);
} // namespace arenacraft::anticheat
