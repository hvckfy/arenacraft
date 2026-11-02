#include <doctest/doctest.h>

#include "PatrolRules.hpp"

#include <vector>

using namespace arenacraft::patrol;

TEST_CASE("patrol categories parse from keys and aliases")
{
  CHECK(parseCategory("cheating") == Category::Cheating);
  CHECK(parseCategory("Cheat") == Category::Cheating);
  CHECK(parseCategory("autokick") == Category::Automation);
  CHECK(parseCategory("BOT") == Category::Automation);
  CHECK(parseCategory("grief") == Category::Griefing);
  CHECK(parseCategory("win-trading") == Category::WinTrading);
  CHECK(parseCategory("wintrade") == Category::WinTrading);
  CHECK_FALSE(parseCategory("rude").has_value());
  CHECK_FALSE(parseCategory("").has_value());
}

TEST_CASE("patrol labels list the categories of a mask")
{
  CHECK(labels(0) == "");
  CHECK(labels(bit(Category::Automation)) == "Automation");
  CHECK(labels(bit(Category::Cheating) | bit(Category::WinTrading)) == "Cheating, Win trading");
}

TEST_CASE("a case is only decided once five verdicts are in")
{
  uint8_t const              yes = bit(Category::Automation);
  std::vector<uint8_t> const four{yes, yes, yes, yes};
  CHECK(guiltyCategories(four) == 0);
}

TEST_CASE("four of five yes votes in one category ban for that category")
{
  uint8_t const              a = bit(Category::Automation);
  uint8_t const              c = bit(Category::Cheating);
  std::vector<uint8_t> const verdicts{a, a | c, a, a, c};
  CHECK(guiltyCategories(verdicts) == a);
}

TEST_CASE("three of five is not enough")
{
  uint8_t const              a = bit(Category::Automation);
  std::vector<uint8_t> const verdicts{a, a, a, 0, 0};
  CHECK(guiltyCategories(verdicts) == 0);
}

TEST_CASE("votes are counted per category, not summed across categories")
{
  uint8_t const              a = bit(Category::Automation);
  uint8_t const              c = bit(Category::Cheating);
  std::vector<uint8_t> const verdicts{a, a, c, c, a | c};
  CHECK(guiltyCategories(verdicts) == 0);
}
