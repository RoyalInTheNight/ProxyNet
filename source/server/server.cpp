#include "../../include/server/server.h"

Server::Server() {
	this->serverPort = 4445;
	this->threadMode = sys::ThreadMode::single_thread;
	this->socketType = sys::SocketType::tcp_socket;

	server.setPort(this->serverPort);
	server.setThreadType(this->threadMode);
	server.setType(this->socketType);
}

Server::Server(const uint16_t getPort)
	: serverPort{ getPort }, 
	  threadMode{ sys::ThreadMode::single_thread }, 
	  socketType{ sys::SocketType::tcp_socket    } {
	server.setPort(this->serverPort);
	server.setThreadType(this->threadMode);
	server.setType(this->socketType);
}

void Server::setPort(const uint16_t getPort) {
	this->serverPort = getPort;

	server.setPort(this->serverPort);
}

void Server::setThreadMode(const sys::ThreadMode& getThread) {
	this->threadMode = getThread;

	server.setThreadType(this->threadMode);
}

void Server::setSocketType(const sys::SocketType& getSocketType) {
	this->socketType = getSocketType;

	server.setType(this->socketType);
}

bool Server::sendAll(const std::string& getData) {
	bool sendAllStatus = server.sendAll(getData);


	return sendAllStatus;
}

bool Server::sendByCID(const uint32_t CID, const std::string& getData) {
	if (CID < 0 || CID >= server.getClients().size())
		return false;

	bool sendByCIDStatus = server.sendBy(server.getCID().at(CID), getData);

	return sendByCIDStatus;
}

bool Server::serverHandlerEnable() {
	if (!server.socketInit())
		return false;

	bool handlerStatus = false;

	std::thread([&]() -> void {
		while ((handlerStatus = server.socketListenConnection()) == true);
	}).detach();

	while (handlerStatus);

	return false;
}