#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

// Pure patrol rules: categories, questions and the verdict tally. No game world, unit-tested.
namespace arenacraft::patrol
{
enum class Category : uint8_t
{
  Cheating   = 0,
  Automation = 1,
  Griefing   = 2,
  WinTrading = 3,
};
inline constexpr std::size_t CategoryCount = 4;

struct CategoryInfo
{
  Category         category;
  std::string_view key;      // used in commands (.replay report ... cheating)
  std::string_view label;    // shown in messages and ban reasons
  std::string_view question; // asked to the patroller
};

inline constexpr std::array<CategoryInfo, CategoryCount> Categories = {{
    {Category::Cheating, "cheating", "Cheating", "Cheating: speed, teleport, fly or wall-walk hacks?"},
    {Category::Automation, "automation", "Automation",
     "Automation: autokick, autofake, auto-interrupt, bots or rotation scripts?"},
    {Category::Griefing, "griefing", "Griefing", "Griefing: throwing the match on purpose, AFK or feeding?"},
    {Category::WinTrading, "wintrading", "Win trading", "Win trading: an arranged result, letting the enemy win?"},
}};

inline constexpr uint8_t bit(Category category) { return uint8_t(1u << uint8_t(category)); }

// Verdicts needed before a case is decided, and "yes" votes in one category that ban.
inline constexpr std::size_t VerdictsPerCase = 5;
inline constexpr std::size_t YesVotesToBan   = 4;

// Accepts the keys above plus common aliases ("cheat", "bot", "autokick", "grief", "wintrade", ...),
// case-insensitive.
std::optional<Category> parseCategory(std::string_view text);

CategoryInfo const& info(Category category);

// Comma-separated labels of the categories in `mask` ("Cheating, Automation").
std::string labels(uint8_t mask);

// Categories that reached YesVotesToBan among the given verdicts (each a yes-mask). Returns 0 until at least
// VerdictsPerCase verdicts are in, so a case is never decided early.
uint8_t guiltyCategories(std::span<uint8_t const> yesMasks);
} // namespace arenacraft::patrol
