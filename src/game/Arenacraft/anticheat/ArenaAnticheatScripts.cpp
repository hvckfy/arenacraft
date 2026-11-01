/*
 * Arena statistical anticheat: config/startup hook and the GM review commands.
 *
 *   .arenacheck list              open flags (newest first)
 *   .arenacheck view <flagId>     one flag's detail + the replay to watch
 *   .arenacheck dismiss <flagId>  mark a flag handled (no effect on the player)
 *
 * Flags never punish. A GM reviews the match with `.replay <replayId>` and acts
 * with the normal tools (.ban, .mute, ...) if warranted.
 */
#include "ArenaAnticheatScripts.hpp"

#include "ArenaAnticheat.hpp"
#include "Chat.h"
#include "CommandScript.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "ReplayConfig.hpp"
#include "WorldScript.h"

#include <sstream>

using namespace Acore::ChatCommands;

namespace arenacraft::anticheat
{
namespace
{
constexpr uint32 ListLimit = 20;

class ArenaAnticheatWorldScript : public WorldScript
{
public:
  ArenaAnticheatWorldScript()
      : WorldScript("arenacraft::anticheat::ArenaAnticheatWorldScript",
                    {WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_STARTUP})
  {
  }

  void OnAfterConfigLoad(bool /*reload*/) override { ArenaAnticheat::instance().loadConfig(); }
  void OnStartup() override { ArenaAnticheat::instance().loadConfig(); }
};

class ArenaAnticheatCommandScript : public CommandScript
{
public:
  ArenaAnticheatCommandScript() : CommandScript("arenacraft::anticheat::ArenaAnticheatCommandScript") {}

  ChatCommandTable GetCommands() const override
  {
    static ChatCommandTable table = {
        {"list", HandleList, SEC_GAMEMASTER, Console::Yes},
        {"view", HandleView, SEC_GAMEMASTER, Console::Yes},
        {"dismiss", HandleDismiss, SEC_GAMEMASTER, Console::Yes},
    };
    static ChatCommandTable commandTable = {{"arenacheck", table}};
    return commandTable;
  }

  static bool HandleList(ChatHandler* handler)
  {
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ARENA_ACHECK_FLAGS);
    stmt->SetData(0, ListLimit);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
    {
      handler->SendSysMessage("No open arena anticheat flags.");
      return true;
    }

    handler->SendSysMessage("Open arena flags (newest first) - .arenacheck view <id>:");
    do
    {
      Field* f = result->Fetch();
      handler->PSendSysMessage("#{} {} - {} (replay #{})", f[0].Get<uint32>(), f[2].Get<std::string>(),
                               f[4].Get<std::string>(), f[1].Get<uint32>());
    } while (result->NextRow());
    return true;
  }

  static bool HandleView(ChatHandler* handler, uint32 flagId)
  {
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ARENA_ACHECK_FLAG);
    stmt->SetData(0, flagId);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    if (!result)
    {
      handler->SendErrorMessage("No such flag.");
      return false;
    }

    Field* f = result->Fetch();
    handler->PSendSysMessage("Flag #{}: {} [{}]", flagId, f[2].Get<std::string>(), f[3].Get<std::string>());
    handler->PSendSysMessage("  {}", f[4].Get<std::string>());
    handler->PSendSysMessage("  status: {}, raised {}", f[6].Get<std::string>(), f[5].Get<std::string>());
    handler->PSendSysMessage("  watch the match: .replay {}", f[1].Get<uint32>());
    return true;
  }

  static bool HandleDismiss(ChatHandler* handler, uint32 flagId)
  {
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ARENA_ACHECK_FLAG);
    std::string const who = handler->GetSession() ? handler->GetSession()->GetPlayerName() : "Console";
    stmt->SetData(0, who);
    stmt->SetData(1, flagId);
    CharacterDatabase.Execute(stmt);
    handler->PSendSysMessage("Flag #{} dismissed.", flagId);
    return true;
  }
};
} // namespace

void AddArenaAnticheatScripts()
{
  new ArenaAnticheatWorldScript();
  new ArenaAnticheatCommandScript();
}
} // namespace arenacraft::anticheat
