#include "ArenaStats.hpp"

#include <algorithm>

namespace arenacraft::anticheat
{
uint32_t medianReaction(std::vector<InterruptSample> const& samples)
{
  if (samples.empty())
    return 0;

  std::vector<uint32_t> values;
  values.reserve(samples.size());
  for (InterruptSample const& s : samples)
    values.push_back(s.reactionMs);
  std::sort(values.begin(), values.end());

  std::size_t const mid = values.size() / 2;
  if (values.size() % 2 == 1)
    return values[mid];
  return (values[mid - 1] + values[mid] + 1) / 2; // round half up
}

std::optional<Flag> evaluate(PlayerStats const& stats, Thresholds const& thresholds)
{
  if (stats.interrupts.size() >= thresholds.minInterruptSamples)
  {
    uint32_t const median = medianReaction(stats.interrupts);
    if (median <= thresholds.interruptReactionMs)
      return Flag{stats.guid, stats.name, "fast-interrupts",
                  "median " + std::to_string(median) + "ms over " + std::to_string(stats.interrupts.size()) +
                      " interrupts"};
  }

  return std::nullopt;
}
} // namespace arenacraft::anticheat
