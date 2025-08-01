/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
Name: quest_commandscript
%Complete: 100
Comment: All quest related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "DisableMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RBAC.h"
#include "ReputationMgr.h"
#include "World.h"

using namespace Trinity::ChatCommands;

class quest_commandscript : public CommandScript
{
public:
    quest_commandscript() : CommandScript("quest_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable objectiveCommandTable =
        {
            { "complete", HandleQuestObjectiveComplete, rbac::RBAC_PERM_COMMAND_QUEST_OBJECTIVE_COMPLETE, Console::No }
        };
        static ChatCommandTable questCommandTable =
        {
            { "add",        HandleQuestAdd,         rbac::RBAC_PERM_COMMAND_QUEST_ADD,      Console::No },
            { "complete",   HandleQuestComplete,    rbac::RBAC_PERM_COMMAND_QUEST_COMPLETE, Console::No },
            { "objective",  objectiveCommandTable },
            { "remove",     HandleQuestRemove,      rbac::RBAC_PERM_COMMAND_QUEST_REMOVE,   Console::No },
            { "reward",     HandleQuestReward,      rbac::RBAC_PERM_COMMAND_QUEST_REWARD,   Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "quest", questCommandTable }
        };
        return commandTable;
    }

    static bool HandleQuestAdd(ChatHandler* handler, Quest const* quest)
    {
        Player* player = handler->getSelectedPlayerOrSelf();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, quest->GetQuestId(), nullptr))
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, quest->GetQuestId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check item starting quest (it can work incorrectly if added without item in inventory)
        ItemTemplateContainer const& itc = sObjectMgr->GetItemTemplateStore();
        auto itr = std::find_if(std::begin(itc), std::end(itc), [quest](ItemTemplateContainer::value_type const& value)
        {
            return value.second.GetStartQuest() == quest->GetQuestId();
        });

        if (itr != std::end(itc))
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_STARTFROMITEM, quest->GetQuestId(), itr->first);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->IsActiveQuest(quest->GetQuestId()))
            return false;

        // ok, normal (creature/GO starting) quest
        if (player->CanAddQuest(quest, true))
            player->AddQuestAndCheckCompletion(quest, nullptr);

        return true;
    }

    static bool HandleQuestRemove(ChatHandler* handler, Quest const* quest)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        QuestStatus oldStatus = player->GetQuestStatus(quest->GetQuestId());

        if (oldStatus != QUEST_STATUS_NONE)
        {
            player->RemoveActiveQuest(quest->GetQuestId(), false);

            // remove all quest entries for 'entry' from quest log
            if (oldStatus != QUEST_STATUS_REWARDED)
            {
                // we ignore unequippable quest items in this case, its' still be equipped
                player->TakeQuestSourceItem(quest->GetQuestId(), false);

                if (quest->HasFlag(QUEST_FLAGS_FLAGS_PVP))
                {
                    player->pvpInfo.IsHostile = player->pvpInfo.IsInHostileArea || player->HasPvPForcingQuest();
                    player->UpdatePvPState();
                }
            }
            player->RemoveRewardedQuest(quest->GetQuestId());
            player->DespawnPersonalSummonsForQuest(quest->GetQuestId());

            sScriptMgr->OnQuestStatusChange(player, quest->GetQuestId());
            sScriptMgr->OnQuestStatusChange(player, quest, oldStatus, QUEST_STATUS_NONE);

            handler->SendSysMessage(LANG_COMMAND_QUEST_REMOVED);
            return true;
        }
        else
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, quest->GetQuestId());
            handler->SetSentErrorMessage(true);
            return false;
        }
    }

    static void CompleteObjective(Player* player, QuestObjective const& obj)
    {
        switch (obj.Type)
        {
            case QUEST_OBJECTIVE_ITEM:
            {
                uint32 curItemCount = player->GetItemCount(obj.ObjectID, true);
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, obj.ObjectID, obj.Amount - curItemCount);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = player->StoreNewItem(dest, obj.ObjectID, true);
                    player->SendNewItem(item, obj.Amount - curItemCount, true, false);
                }
                break;
            }
            case QUEST_OBJECTIVE_CURRENCY:
            {
                player->ModifyCurrency(obj.ObjectID, obj.Amount, CurrencyGainSource::Cheat);
                break;
            }
            case QUEST_OBJECTIVE_MIN_REPUTATION:
            {
                uint32 curRep = player->GetReputationMgr().GetReputation(obj.ObjectID);
                if (curRep < uint32(obj.Amount))
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(obj.ObjectID))
                        player->GetReputationMgr().SetReputation(factionEntry, obj.Amount);
                break;
            }
            case QUEST_OBJECTIVE_MAX_REPUTATION:
            {
                uint32 curRep = player->GetReputationMgr().GetReputation(obj.ObjectID);
                if (curRep > uint32(obj.Amount))
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(obj.ObjectID))
                        player->GetReputationMgr().SetReputation(factionEntry, obj.Amount);
                break;
            }
            case QUEST_OBJECTIVE_MONEY:
            {
                player->ModifyMoney(obj.Amount);
                break;
            }
            case QUEST_OBJECTIVE_PROGRESS_BAR:
                // do nothing
                break;
            default:
                player->UpdateQuestObjectiveProgress(static_cast<QuestObjectiveType>(obj.Type), obj.ObjectID, obj.Amount);
                break;
        }
    }

    static bool HandleQuestComplete(ChatHandler* handler, Quest const* quest)
    {
        Player* player = handler->getSelectedPlayerOrSelf();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // If player doesn't have the quest
        if ((player->GetQuestStatus(quest->GetQuestId()) == QUEST_STATUS_NONE && !quest->HasFlag(QUEST_FLAGS_TRACKING_EVENT))
            || DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, quest->GetQuestId(), nullptr))
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, quest->GetQuestId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        for (QuestObjective const& obj : quest->Objectives)
            CompleteObjective(player, obj);

        if (sWorld->getBoolConfig(CONFIG_QUEST_ENABLE_QUEST_TRACKER)) // check if Quest Tracker is enabled
        {
            // prepare Quest Tracker datas
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_QUEST_TRACK_GM_COMPLETE);
            stmt->setUInt32(0, quest->GetQuestId());
            stmt->setUInt64(1, player->GetGUID().GetCounter());

            // add to Quest Tracker
            CharacterDatabase.Execute(stmt);
        }

        player->CompleteQuest(quest->GetQuestId());
        return true;
    }

    static bool HandleQuestObjectiveComplete(ChatHandler* handler, uint32 objectiveId)
    {
        Player* player = handler->getSelectedPlayerOrSelf();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        QuestObjective const* obj = sObjectMgr->GetQuestObjective(objectiveId);
        if (!obj)
        {
            handler->SendSysMessage(LANG_COMMAND_QUEST_OBJECTIVE_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CompleteObjective(player, *obj);
        return true;
    }

    static bool HandleQuestReward(ChatHandler* handler, Quest const* quest)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // If player doesn't have the quest
        if (player->GetQuestStatus(quest->GetQuestId()) != QUEST_STATUS_COMPLETE
            || DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, quest->GetQuestId(), nullptr))
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, quest->GetQuestId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->RewardQuest(quest, LootItemType::Item, 0, player);
        return true;
    }
};

void AddSC_quest_commandscript()
{
    new quest_commandscript();
}
