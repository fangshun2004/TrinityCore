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

#include "WorldSocketMgr.h"
#include "Config.h"
#include "NetworkThread.h"
#include "ScriptMgr.h"
#include "WorldSocket.h"
#include <boost/system/error_code.hpp>

class WorldSocketThread : public Trinity::Net::NetworkThread<WorldSocket>
{
public:
    void SocketAdded(std::shared_ptr<WorldSocket> const& sock) override
    {
        sock->SetSendBufferSize(sWorldSocketMgr.GetApplicationSendBufferSize());
        sScriptMgr->OnSocketOpen(sock);
    }

    void SocketRemoved(std::shared_ptr<WorldSocket>const& sock) override
    {
        sScriptMgr->OnSocketClose(sock);
    }
};

WorldSocketMgr::WorldSocketMgr() : BaseSocketMgr(), _socketSystemSendBufferSize(-1), _socketApplicationSendBufferSize(65536), _tcpNoDelay(true)
{
}

WorldSocketMgr::~WorldSocketMgr() = default;

WorldSocketMgr& WorldSocketMgr::Instance()
{
    static WorldSocketMgr instance;
    return instance;
}

bool WorldSocketMgr::StartNetwork(Trinity::Asio::IoContext& ioContext, std::string const& bindIp, uint16 port, int threadCount)
{
    _tcpNoDelay = sConfigMgr->GetBoolDefault("Network.TcpNodelay", true);

    int const max_connections = TRINITY_MAX_LISTEN_CONNECTIONS;
    TC_LOG_DEBUG("misc", "Max allowed socket connections {}", max_connections);

    // -1 means use default
    _socketSystemSendBufferSize = sConfigMgr->GetIntDefault("Network.OutKBuff", -1);

    _socketApplicationSendBufferSize = sConfigMgr->GetIntDefault("Network.OutUBuff", 65536);

    if (_socketApplicationSendBufferSize <= 0)
    {
        TC_LOG_ERROR("misc", "Network.OutUBuff is wrong in your config file");
        return false;
    }

    if (!BaseSocketMgr::StartNetwork(ioContext, bindIp, port, threadCount))
        return false;

    _acceptor->AsyncAccept([this](Trinity::Net::IoContextTcpSocket&& sock, uint32 threadIndex)
    {
        OnSocketOpen(std::move(sock), threadIndex);
    });

    sScriptMgr->OnNetworkStart();
    return true;
}

void WorldSocketMgr::StopNetwork()
{
    BaseSocketMgr::StopNetwork();

    sScriptMgr->OnNetworkStop();
}

void WorldSocketMgr::OnSocketOpen(Trinity::Net::IoContextTcpSocket&& sock, uint32 threadIndex)
{
    // set some options here
    if (_socketSystemSendBufferSize >= 0)
    {
        boost::system::error_code err;
        sock.set_option(boost::asio::socket_base::send_buffer_size(_socketSystemSendBufferSize), err);
        if (err && err != boost::system::errc::not_supported)
        {
            TC_LOG_ERROR("misc", "WorldSocketMgr::OnSocketOpen sock.set_option(boost::asio::socket_base::send_buffer_size) err = {}", err.message());
            return;
        }
    }

    // Set TCP_NODELAY.
    if (_tcpNoDelay)
    {
        boost::system::error_code err;
        sock.set_option(boost::asio::ip::tcp::no_delay(true), err);
        if (err)
        {
            TC_LOG_ERROR("misc", "WorldSocketMgr::OnSocketOpen sock.set_option(boost::asio::ip::tcp::no_delay) err = {}", err.message());
            return;
        }
    }

    BaseSocketMgr::OnSocketOpen(std::move(sock), threadIndex);
}

Trinity::Net::NetworkThread<WorldSocket>* WorldSocketMgr::CreateThreads() const
{
    return new WorldSocketThread[GetNetworkThreadCount()];
}
