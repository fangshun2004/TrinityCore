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

#ifndef PartyPackets_h__
#define PartyPackets_h__

#include "Packet.h"
#include "AuthenticationPackets.h"
#include "ObjectGuid.h"
#include "Group.h"
#include "MythicPlusPacketsCommon.h"
#include "Optional.h"

namespace WorldPackets
{
    namespace Party
    {
        class PartyCommandResult final : public ServerPacket
        {
        public:
            PartyCommandResult() : ServerPacket(SMSG_PARTY_COMMAND_RESULT, 23) { }

            WorldPacket const* Write() override;

            std::string Name;
            uint8 Command = 0u;
            uint8 Result = 0u;
            uint32 ResultData = 0u;
            ObjectGuid ResultGUID;
        };

        class PartyInviteClient final : public ClientPacket
        {
        public:
            PartyInviteClient(WorldPacket&& packet) : ClientPacket(CMSG_PARTY_INVITE, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            uint32 ProposedRoles = 0;
            std::string TargetName;
            std::string TargetRealm;
            ObjectGuid TargetGUID;
        };

        class PartyInvite final : public ServerPacket
        {
        public:
            PartyInvite() : ServerPacket(SMSG_PARTY_INVITE, 55) { }

            WorldPacket const* Write() override;

            void Initialize(Player const* inviter, int32 proposedRoles, bool canAccept);

            bool ShouldSquelch = false;
            bool AllowMultipleRoles = false;
            bool QuestSessionActive = false;
            bool IsCrossFaction = false;
            uint16 InviterCfgRealmID = 0;

            bool CanAccept = false;

            // Inviter
            Auth::VirtualRealmInfo InviterRealm;
            ObjectGuid InviterGUID;
            ObjectGuid InviterBNetAccountId;
            std::string InviterName;

            // Realm
            bool IsXRealm = false;
            bool IsXNativeRealm = false;

            // Lfg
            uint8 ProposedRoles = 0;
            uint32 LfgCompletedMask = 0;
            std::vector<uint32> LfgSlots;
        };

        class PartyInviteResponse final : public ClientPacket
        {
        public:
            PartyInviteResponse(WorldPacket&& packet) : ClientPacket(CMSG_PARTY_INVITE_RESPONSE, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            bool Accept = false;
            Optional<uint8> RolesDesired;
        };

        class PartyUninvite final : public ClientPacket
        {
        public:
            PartyUninvite(WorldPacket&& packet) : ClientPacket(CMSG_PARTY_UNINVITE, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            ObjectGuid TargetGUID;
            std::string Reason;
        };

        class GroupDecline final : public ServerPacket
        {
        public:
            GroupDecline(std::string const& name) : ServerPacket(SMSG_GROUP_DECLINE, 2 + name.size()), Name(name) { }

            WorldPacket const* Write() override;

            std::string Name;
        };

        class GroupUninvite final : public ServerPacket
        {
        public:
            GroupUninvite() : ServerPacket(SMSG_GROUP_UNINVITE, 1) { }

            WorldPacket const* Write() override;

            uint8 Reason = 0;
        };

        class RequestPartyMemberStats final : public ClientPacket
        {
        public:
            RequestPartyMemberStats(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_PARTY_MEMBER_STATS, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            ObjectGuid TargetGUID;
        };

        struct PartyMemberPhase
        {
            uint32 Flags = 0u;
            uint16 Id = 0u;
        };

        struct PartyMemberPhaseStates
        {
            uint32 PhaseShiftFlags = 0;
            ObjectGuid PersonalGUID;
            std::vector<PartyMemberPhase> List;
        };

        struct PartyMemberAuraStates
        {
            int32 SpellID = 0;
            uint16 Flags = 0;
            uint32 ActiveFlags = 0u;
            std::vector<float> Points;
        };

        struct PartyMemberPetStats
        {
            ObjectGuid GUID;
            std::string Name;
            int16 ModelId = 0;

            int32 CurrentHealth = 0;
            int32 MaxHealth = 0;

            std::vector<PartyMemberAuraStates> Auras;
        };

        struct CTROptions
        {
            uint32 ConditionalFlags = 0;
            int8 FactionGroup = 0;
            uint32 ChromieTimeExpansionMask = 0;
        };

        struct PartyMemberStats
        {
            uint16 Level = 0;
            uint16 Status = 0;

            int32 CurrentHealth = 0;
            int32 MaxHealth = 0;

            uint8 PowerType = 0u;
            uint16 CurrentPower = 0;
            uint16 MaxPower = 0;

            uint16 ZoneID = 0;
            int16 PositionX = 0;
            int16 PositionY = 0;
            int16 PositionZ = 0;

            int32 VehicleSeat = 0;

            PartyMemberPhaseStates Phases;
            std::vector<PartyMemberAuraStates> Auras;
            Optional<PartyMemberPetStats> PetStats;

            uint16 PowerDisplayID = 0;
            uint16 SpecID = 0;
            uint16 WmoGroupID = 0;
            uint32 WmoDoodadPlacementID = 0;
            int8 PartyType[2] = { };

            CTROptions ChromieTime;

            MythicPlus::DungeonScoreSummary DungeonScore;
        };

        class PartyMemberFullState final : public ServerPacket
        {
        public:
            PartyMemberFullState() : ServerPacket(SMSG_PARTY_MEMBER_FULL_STATE, 80) { }

            WorldPacket const* Write() override;
            void Initialize(Player const* player);

            bool ForEnemy = false;
            ObjectGuid MemberGuid;
            PartyMemberStats MemberStats;
        };

        class SetPartyLeader final : public ClientPacket
        {
        public:
            SetPartyLeader(WorldPacket&& packet) : ClientPacket(CMSG_SET_PARTY_LEADER, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            ObjectGuid TargetGUID;
        };

        class SetRole final : public ClientPacket
        {
        public:
            SetRole(WorldPacket&& packet) : ClientPacket(CMSG_SET_ROLE, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            ObjectGuid TargetGUID;
            uint8 Role = 0;
        };

        class RoleChangedInform final : public ServerPacket
        {
        public:
            RoleChangedInform() : ServerPacket(SMSG_ROLE_CHANGED_INFORM, 41) { }

            WorldPacket const* Write() override;

            uint8 PartyIndex = 0;
            ObjectGuid From;
            ObjectGuid ChangedUnit;
            uint8 OldRole = 0;
            uint8 NewRole = 0;
        };

        class LeaveGroup final : public ClientPacket
        {
        public:
            LeaveGroup(WorldPacket&& packet) : ClientPacket(CMSG_LEAVE_GROUP, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
        };

        class SetLootMethod final : public ClientPacket
        {
        public:
            SetLootMethod(WorldPacket&& packet) : ClientPacket(CMSG_SET_LOOT_METHOD, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            ObjectGuid LootMasterGUID;
            uint8 LootMethod = 0u;
            uint32 LootThreshold = 0u;
        };

        class MinimapPingClient final : public ClientPacket
        {
        public:
            MinimapPingClient(WorldPacket&& packet) : ClientPacket(CMSG_MINIMAP_PING, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            float PositionX = 0.f;
            float PositionY = 0.f;
        };

        class MinimapPing final : public ServerPacket
        {
        public:
            MinimapPing() : ServerPacket(SMSG_MINIMAP_PING, 24) { }

            WorldPacket const* Write() override;

            ObjectGuid Sender;
            float PositionX = 0.f;
            float PositionY = 0.f;
        };

        class UpdateRaidTarget final : public ClientPacket
        {
        public:
            UpdateRaidTarget(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_RAID_TARGET, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            ObjectGuid Target;
            int8 Symbol = 0;
        };

        class SendRaidTargetUpdateSingle final : public ServerPacket
        {
        public:
            SendRaidTargetUpdateSingle() : ServerPacket(SMSG_SEND_RAID_TARGET_UPDATE_SINGLE, 34) { }

            WorldPacket const* Write() override;

            int8 PartyIndex = 0;
            ObjectGuid Target;
            ObjectGuid ChangedBy;
            int8 Symbol = 0;
        };

        class SendRaidTargetUpdateAll final : public ServerPacket
        {
        public:
            SendRaidTargetUpdateAll() : ServerPacket(SMSG_SEND_RAID_TARGET_UPDATE_ALL, 1 + TARGET_ICONS_COUNT * (1 + 16)) { }

            WorldPacket const* Write() override;

            uint8 PartyIndex = 0;
            std::map<uint8, ObjectGuid> TargetIcons;
        };

        class ConvertRaid final : public ClientPacket
        {
        public:
            ConvertRaid(WorldPacket&& packet) : ClientPacket(CMSG_CONVERT_RAID, std::move(packet)) { }

            void Read() override;

            bool Raid = false;
        };

        class RequestPartyJoinUpdates final : public ClientPacket
        {
        public:
            RequestPartyJoinUpdates(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_PARTY_JOIN_UPDATES, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
        };

        class SetAssistantLeader final : public ClientPacket
        {
        public:
            SetAssistantLeader(WorldPacket&& packet) : ClientPacket(CMSG_SET_ASSISTANT_LEADER, std::move(packet)) { }

            void Read() override;

            ObjectGuid Target;
            Optional<uint8> PartyIndex;
            bool Apply = false;
        };

        class SetPartyAssignment final : public ClientPacket
        {
        public:
            SetPartyAssignment(WorldPacket&& packet) : ClientPacket(CMSG_SET_PARTY_ASSIGNMENT, std::move(packet)) { }

            void Read() override;
            uint8 Assignment = 0;
            Optional<uint8> PartyIndex;
            ObjectGuid Target;
            bool Set = false;
        };

        class DoReadyCheck final : public ClientPacket
        {
        public:
            DoReadyCheck(WorldPacket&& packet) : ClientPacket(CMSG_DO_READY_CHECK, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
        };

        class ReadyCheckStarted final : public ServerPacket
        {
        public:
            ReadyCheckStarted() : ServerPacket(SMSG_READY_CHECK_STARTED, 37) { }

            WorldPacket const* Write() override;

            int8 PartyIndex = 0;
            ObjectGuid PartyGUID;
            ObjectGuid InitiatorGUID;
            WorldPackets::Duration<Milliseconds> Duration;
        };

        class ReadyCheckResponseClient final : public ClientPacket
        {
        public:
            ReadyCheckResponseClient(WorldPacket&& packet) : ClientPacket(CMSG_READY_CHECK_RESPONSE, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            bool IsReady = false;
        };

        class ReadyCheckResponse final : public ServerPacket
        {
        public:
            ReadyCheckResponse() : ServerPacket(SMSG_READY_CHECK_RESPONSE, 19) { }

            WorldPacket const* Write() override;

            ObjectGuid PartyGUID;
            ObjectGuid Player;
            bool IsReady = false;
        };

        class ReadyCheckCompleted final : public ServerPacket
        {
        public:
            ReadyCheckCompleted() : ServerPacket(SMSG_READY_CHECK_COMPLETED, 17) { }

            WorldPacket const* Write() override;

            int8 PartyIndex = 0;
            ObjectGuid PartyGUID;
        };

        class RequestRaidInfo final : public ClientPacket
        {
        public:
            RequestRaidInfo(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_RAID_INFO, std::move(packet)) { }

            void Read() override { }
        };

        class OptOutOfLoot final : public ClientPacket
        {
        public:
            OptOutOfLoot(WorldPacket&& packet) : ClientPacket(CMSG_OPT_OUT_OF_LOOT, std::move(packet)) { }

            void Read() override;

            bool PassOnLoot = false;
        };

        class InitiateRolePoll final : public ClientPacket
        {
        public:
            InitiateRolePoll(WorldPacket&& packet) : ClientPacket(CMSG_INITIATE_ROLE_POLL, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
        };

        class RolePollInform final : public ServerPacket
        {
        public:
            RolePollInform() : ServerPacket(SMSG_ROLE_POLL_INFORM, 17) { }

            WorldPacket const* Write() override;

            int8 PartyIndex = 0;
            ObjectGuid From;
        };

        class GroupNewLeader final : public ServerPacket
        {
        public:
            GroupNewLeader() : ServerPacket(SMSG_GROUP_NEW_LEADER, 14) { }

            WorldPacket const* Write() override;

            int8 PartyIndex = 0;
            std::string Name;
        };

        struct PartyPlayerInfo
        {
            ObjectGuid GUID;
            std::string Name;
            std::string VoiceStateID;   // same as bgs.protocol.club.v1.MemberVoiceState.id
            uint8 Class = 0u;
            uint8 Subgroup = 0u;
            uint8 Flags = 0u;
            uint8 RolesAssigned = 0u;
            uint8 FactionGroup = 0u;
            bool FromSocialQueue = false;
            bool VoiceChatSilenced = false;
            bool Connected = false;
        };

        struct PartyLFGInfo
        {
            uint8 MyFlags = 0;
            uint32 Slot = 0;
            uint8 BootCount = 0;
            uint32 MyRandomSlot = 0;
            bool Aborted = false;
            uint8 MyPartialClear = 0;
            float MyGearDiff = 0.0f;
            uint8 MyStrangerCount = 0;
            uint8 MyKickVoteCount = 0;
            bool MyFirstReward = false;
        };

        struct PartyLootSettings
        {
            uint8 Method = 0u;
            ObjectGuid LootMaster;
            uint8 Threshold = 0u;
        };

        struct PartyDifficultySettings
        {
            uint32 DungeonDifficultyID = 0u;
            uint32 RaidDifficultyID = 0u;
            uint32 LegacyRaidDifficultyID = 0u;
        };

        class PartyUpdate final : public ServerPacket
        {
        public:
            PartyUpdate() : ServerPacket(SMSG_PARTY_UPDATE, 200) { }

            WorldPacket const* Write() override;

            uint16 PartyFlags = 0;
            uint8 PartyIndex = 0;
            uint8 PartyType = 0;

            ObjectGuid PartyGUID;
            ObjectGuid LeaderGUID;
            uint8 LeaderFactionGroup = 0;

            int32 MyIndex = 0;
            int32 SequenceNum = 0;

            RestrictPingsTo PingRestriction = RestrictPingsTo::None;

            std::vector<PartyPlayerInfo> PlayerList;

            Optional<PartyLFGInfo> LfgInfos;
            Optional<PartyLootSettings> LootSettings;
            Optional<PartyDifficultySettings> DifficultySettings;
        };

        class SetEveryoneIsAssistant final : public ClientPacket
        {
        public:
            SetEveryoneIsAssistant(WorldPacket&& packet) : ClientPacket(CMSG_SET_EVERYONE_IS_ASSISTANT, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            bool EveryoneIsAssistant = false;
        };

        class ChangeSubGroup final : public ClientPacket
        {
        public:
            ChangeSubGroup(WorldPacket&& packet) : ClientPacket(CMSG_CHANGE_SUB_GROUP, std::move(packet)) { }

            void Read() override;

            ObjectGuid TargetGUID;
            Optional<uint8> PartyIndex;
            uint8 NewSubGroup = 0u;
        };

        class SwapSubGroups final : public ClientPacket
        {
        public:
            SwapSubGroups(WorldPacket&& packet) : ClientPacket(CMSG_SWAP_SUB_GROUPS, std::move(packet)) { }

            void Read() override;

            ObjectGuid FirstTarget;
            ObjectGuid SecondTarget;
            Optional<uint8> PartyIndex;
        };

        class ClearRaidMarker final : public ClientPacket
        {
        public:
            ClearRaidMarker(WorldPacket&& packet) : ClientPacket(CMSG_CLEAR_RAID_MARKER, std::move(packet)) { }

            void Read() override;

            uint8 MarkerId = 0u;
        };

        class RaidMarkersChanged final : public ServerPacket
        {
        public:
            RaidMarkersChanged() : ServerPacket(SMSG_RAID_MARKERS_CHANGED, 6) { }

            WorldPacket const* Write() override;

            uint8 PartyIndex = 0;
            uint32 ActiveMarkers = 0u;

            std::vector<RaidMarker const*> RaidMarkers;
        };

        class PartyKillLog final : public ServerPacket
        {
        public:
            PartyKillLog() : ServerPacket(SMSG_PARTY_KILL_LOG, 2 * 16) { }

            WorldPacket const* Write() override;

            ObjectGuid Player;
            ObjectGuid Victim;
        };

        class GroupDestroyed final : public ServerPacket
        {
        public:
            GroupDestroyed() : ServerPacket(SMSG_GROUP_DESTROYED, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class BroadcastSummonCast final : public ServerPacket
        {
        public:
            BroadcastSummonCast() : ServerPacket(SMSG_BROADCAST_SUMMON_CAST, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid Target;
        };

        class BroadcastSummonResponse final : public ServerPacket
        {
        public:
            BroadcastSummonResponse() : ServerPacket(SMSG_BROADCAST_SUMMON_RESPONSE, 16 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid Target;
            bool Accepted = false;
        };

        class SetRestrictPingsToAssistants final : public ClientPacket
        {
        public:
            explicit SetRestrictPingsToAssistants(WorldPacket&& packet) : ClientPacket(CMSG_SET_RESTRICT_PINGS_TO_ASSISTANTS, std::move(packet)) { }

            void Read() override;

            Optional<uint8> PartyIndex;
            RestrictPingsTo RestrictTo = RestrictPingsTo::None;
        };

        class SendPingUnit final : public ClientPacket
        {
        public:
            explicit SendPingUnit(WorldPacket&& packet) : ClientPacket(CMSG_SEND_PING_UNIT, std::move(packet)) { }

            void Read() override;

            ObjectGuid SenderGUID;
            ObjectGuid TargetGUID;
            PingSubjectType Type = PingSubjectType::Max;
            uint32 PinFrameID = 0;
            Duration<Milliseconds, int32> PingDuration;
            Optional<uint32> CreatureID;
            Optional<uint32> SpellOverrideNameID;
        };

        class ReceivePingUnit final : public ServerPacket
        {
        public:
            ReceivePingUnit() : ServerPacket(SMSG_RECEIVE_PING_UNIT, 16 + 16 + 1 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid SenderGUID;
            ObjectGuid TargetGUID;
            PingSubjectType Type = PingSubjectType::Max;
            uint32 PinFrameID = 0;
            Duration<Milliseconds, int32> PingDuration;
            Optional<uint32> CreatureID;
            Optional<uint32> SpellOverrideNameID;
        };

        class SendPingWorldPoint final : public ClientPacket
        {
        public:
            explicit SendPingWorldPoint(WorldPacket&& packet) : ClientPacket(CMSG_SEND_PING_WORLD_POINT, std::move(packet)) { }

            void Read() override;

            ObjectGuid SenderGUID;
            uint32 MapID = 0;
            TaggedPosition<Position::XYZ> Point;
            PingSubjectType Type = PingSubjectType::Max;
            uint32 PinFrameID = 0;
            ObjectGuid Transport;
            Duration<Milliseconds, int32> PingDuration;
        };

        class ReceivePingWorldPoint final : public ServerPacket
        {
        public:
            ReceivePingWorldPoint() : ServerPacket(SMSG_RECEIVE_PING_WORLD_POINT, 16 + 4 + 4 * 3 + 1 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid SenderGUID;
            uint32 MapID = 0;
            TaggedPosition<Position::XYZ> Point;
            PingSubjectType Type = PingSubjectType::Max;
            uint32 PinFrameID = 0;
            Duration<Milliseconds, int32> PingDuration;
            ObjectGuid Transport;
        };

        class CancelPingPin final : public ServerPacket
        {
        public:
            CancelPingPin() : ServerPacket(SMSG_CANCEL_PING_PIN, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid SenderGUID;
            uint32 PinFrameID = 0;
        };
    }
}

#endif // PartyPackets_h__
