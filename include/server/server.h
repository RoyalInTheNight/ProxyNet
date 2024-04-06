#include "../socket/server_socket.h"
#include "../crypto/sha256.h"

#ifndef _WIN32
#  include <sys/un.h>
#endif // _WIN32

#include <sstream>
#include <functional>

#include <map>

class Empire {
private:
	typedef sys::ClientConnectionData       ClientCD_t;
	typedef std::vector<ClientCD_t>           client_t;
	typedef std::stringstream		       __sstream_t;
	typedef std::string 				      string_t;
	typedef std::vector<string_t> 	   	     vstring_t;
	typedef sys::SocketServer		     	  server_t;
	typedef std::function<bool(server_t*)>   handler_t;
	typedef std::map<char, handler_t> client_handler_t;
	typedef void 							    void_t;

private:
	handler_t SocketListenHandler;
	server_t   SocketServerEmpire;
	client_t     SocketClientData;
	client_handler_t CHandlerList;

	vstring_t  CID;
	vstring_t  CDB;
	vstring_t CHDB;
	client_t CHDBC;

	string_t ServerEmpire;
	string_t CommandDeskList;
	uint16_t port;

public:
	Empire();
	Empire(server_t&, handler_t&);
	Empire(server_t&, handler_t&, const string_t&);
	
	[[nodiscard]] const uint16_t GetServerPort();
	[[nodiscard]] const uint32_t GetServerAddress();
	[[nodiscard]] const uint32_t GetClientAddress(const string_t); // arg it is CID
	[[nodiscard]] const string_t GetCDL();

	string_t Addr2String(const uint32_t);
	string_t GetServerResult();

	void_t   SetServerPort(const uint16_t);
	void_t   SetHandler(handler_t&);
	void_t   SetServer(server_t&);
	void_t   SetCDL(const string_t&);
	void_t   SetCHandlerList(client_handler_t&);
	string_t ServerHandler(const string_t&);

	~Empire();
};

class EmpireDaemon {
private:
public:
};