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

#ifndef TRINITYCORE_BLACK_MARKET_PACKETS_H
#define TRINITYCORE_BLACK_MARKET_PACKETS_H

#include "Packet.h"
#include "ItemPacketsCommon.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace BlackMarket
    {
        struct BlackMarketItem
        {
            int32 MarketID = 0;
            int32 SellerNPC = 0;
            Item::ItemInstance Item;
            int32 Quantity = 0;
            uint64 MinBid = 0;
            uint64 MinIncrement = 0;
            uint64 CurrentBid = 0;
            int32 SecondsRemaining = 0;
            int32 NumBids = 0;
            bool HighBid = false;
        };

        class BlackMarketOpen final : public ClientPacket
        {
        public:
            explicit BlackMarketOpen(WorldPacket&& packet) : ClientPacket(CMSG_BLACK_MARKET_OPEN, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
        };

        class BlackMarketRequestItems final : public ClientPacket
        {
        public:
            explicit BlackMarketRequestItems(WorldPacket&& packet) : ClientPacket(CMSG_BLACK_MARKET_REQUEST_ITEMS, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
            Timestamp<> LastUpdateID;
        };

        class BlackMarketRequestItemsResult final : public ServerPacket
        {
        public:
            explicit BlackMarketRequestItemsResult() : ServerPacket(SMSG_BLACK_MARKET_REQUEST_ITEMS_RESULT, 4) { }

            WorldPacket const* Write() override;

            Timestamp<> LastUpdateID;
            std::vector<BlackMarketItem> Items;
        };

        class BlackMarketBidOnItem final : public ClientPacket
        {
        public:
            explicit BlackMarketBidOnItem(WorldPacket&& packet) : ClientPacket(CMSG_BLACK_MARKET_BID_ON_ITEM, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
            int32 MarketID = 0;
            Item::ItemInstance Item;
            uint64 BidAmount = 0;
        };

        class BlackMarketBidOnItemResult final : public ServerPacket
        {
        public:
            explicit BlackMarketBidOnItemResult() : ServerPacket(SMSG_BLACK_MARKET_BID_ON_ITEM_RESULT, 4 + 76 + 4) { }

            WorldPacket const* Write() override;

            int32 MarketID = 0;
            Item::ItemInstance Item;
            int32 Result = 0;
        };

        class BlackMarketOutbid final : public ServerPacket
        {
        public:
            explicit BlackMarketOutbid() : ServerPacket(SMSG_BLACK_MARKET_OUTBID, 4 + 76 + 4) { }

            WorldPacket const* Write() override;

            int32 MarketID = 0;
            Item::ItemInstance Item;
            int32 RandomPropertiesID = 0;
        };

        class BlackMarketWon final : public ServerPacket
        {
        public:
            explicit BlackMarketWon() : ServerPacket(SMSG_BLACK_MARKET_WON, 4 + 76 + 4) { }

            WorldPacket const* Write() override;

            int32 MarketID = 0;
            Item::ItemInstance Item;
            int32 RandomPropertiesID = 0;
        };
    }
}

#endif // TRINITYCORE_BLACK_MARKET_PACKETS_H
