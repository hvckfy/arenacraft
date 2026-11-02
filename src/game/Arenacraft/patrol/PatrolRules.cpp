#include "PatrolRules.hpp"

#include <algorithm>
#include <cctype>

namespace arenacraft::patrol
{
std::optional<Category> parseCategory(std::string_view text)
{
  std::string lower;
  for (char c : text)
    if (c != ' ' && c != '-' && c != '_')
      lower += char(std::tolower(static_cast<unsigned char>(c)));

  for (CategoryInfo const& entry : Categories)
    if (lower == entry.key)
      return entry.category;

  if (lower == "cheat" || lower == "hack" || lower == "hacks" || lower == "speedhack")
    return Category::Cheating;
  if (lower == "bot" || lower == "botting" || lower == "autokick" || lower == "autofake" || lower == "script" ||
      lower == "scripts")
    return Category::Automation;
  if (lower == "grief" || lower == "afk" || lower == "throwing" || lower == "feeding")
    return Category::Griefing;
  if (lower == "wintrade" || lower == "wintrades")
    return Category::WinTrading;
  return std::nullopt;
}

CategoryInfo const& info(Category category) { return Categories[std::size_t(category)]; }

std::string labels(uint8_t mask)
{
  std::string out;
  for (CategoryInfo const& entry : Categories)
  {
    if (!(mask & bit(entry.category)))
      continue;
    if (!out.empty())
      out += ", ";
    out += entry.label;
  }
  return out;
}

uint8_t guiltyCategories(std::span<uint8_t const> yesMasks)
{
  if (yesMasks.size() < VerdictsPerCase)
    return 0;

  uint8_t guilty = 0;
  for (CategoryInfo const& entry : Categories)
  {
    std::size_t const yes =
        std::count_if(yesMasks.begin(), yesMasks.end(), [&](uint8_t mask) { return mask & bit(entry.category); });
    if (yes >= YesVotesToBan)
      guilty |= bit(entry.category);
  }
  return guilty;
}
} // namespace arenacraft::patrol
