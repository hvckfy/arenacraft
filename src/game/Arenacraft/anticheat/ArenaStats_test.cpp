#include <doctest/doctest.h>

#include "ArenaStats.hpp"

using namespace arenacraft::anticheat;

namespace
{
PlayerStats withInterrupts(std::vector<uint32_t> const& reactions)
{
  PlayerStats stats;
  stats.guid = 7;
  stats.name = "Botty";
  for (uint32_t r : reactions)
    stats.interrupts.push_back({r});
  return stats;
}
} // namespace

TEST_CASE("medianReaction")
{
  CHECK(medianReaction({}) == 0);
  CHECK(medianReaction({{100}}) == 100);
  CHECK(medianReaction({{100}, {200}, {300}}) == 200);
  CHECK(medianReaction({{100}, {200}}) == 150);
  CHECK(medianReaction({{80}, {90}, {100}, {110}}) == 95);
}

TEST_CASE("evaluate flags inhumanly fast, consistent interrupts")
{
  Thresholds t; // 250ms median, 4 samples

  SUBCASE("a reaction bot is flagged")
  {
    auto flag = evaluate(withInterrupts({60, 70, 80, 90, 75}), t);
    REQUIRE(flag);
    CHECK(flag->reason == "fast-interrupts");
    CHECK(flag->guid == 7);
    CHECK(flag->detail == "median 75ms over 5 interrupts");
  }

  SUBCASE("human reaction times are not flagged")
  {
    CHECK_FALSE(evaluate(withInterrupts({320, 410, 280, 500, 390}), t));
  }

  SUBCASE("too few samples never flags, however fast") { CHECK_FALSE(evaluate(withInterrupts({10, 10, 10}), t)); }

  SUBCASE("right at the threshold flags")
  {
    CHECK(evaluate(withInterrupts({250, 250, 250, 250}), t));
    CHECK_FALSE(evaluate(withInterrupts({251, 251, 251, 251}), t));
  }

  SUBCASE("no samples") { CHECK_FALSE(evaluate(withInterrupts({}), t)); }
}
