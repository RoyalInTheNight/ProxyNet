#ifndef __CLOUD
#define __CLOUD
#include <cstdint>
#include <functional>
#ifndef _WIN32

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <vector>
#include <string>

#include "../thread_pool/pool.h"
#include "../crypto/sha256.h"

#include <map>
#include <fstream>
#include <functional>

namespace CloudServerTypes {
    typedef sockaddr_in  cloud_header_t;
    typedef int32_t      cloud_socket_t;
    typedef socklen_t    cloud_sosize_t;

    typedef std::string cloud_address_t;
    typedef uint16_t       cloud_port_t;
}

namespace CloudTypes {
    typedef std::string cloud_path_t;
    typedef std::map<cloud_path_t, 
         std::vector<cloud_path_t>>
                        cloud_temp_t;
}

enum class CloudErrors : uint8_t {
    CloudSocketError     = 0xa1,
    CloudConnectionError = 0xa2,
    CloudEstabilishError = 0xa3,
    CloudUploadError     = 0xa4,
    CloudDownloadError   = 0xa5
};

class Cloud {
private:
    CloudServerTypes::cloud_header_t header_;
    CloudServerTypes::cloud_socket_t socket_;
    CloudServerTypes::cloud_sosize_t sosize_;

    CloudTypes::cloud_path_t   database_path;
    CloudTypes::cloud_temp_t   database_temp;

public:
    Cloud(const CloudServerTypes::cloud_address_t&, const CloudServerTypes::cloud_port_t);
    Cloud(const CloudServerTypes::cloud_address_t&, const std::string&);
    Cloud(const Cloud&);
    Cloud();

    void set_port(const std::string& port) { header_.sin_port = htons(std::stoi(port)); }
    void set_port(const CloudServerTypes::cloud_port_t port) { header_.sin_port = htons(port); }
    void set_address(const CloudServerTypes::cloud_address_t& address) { header_.sin_addr.s_addr = inet_addr(address.c_str()); }
    void set_header(const CloudServerTypes::cloud_header_t& header) { header_ = header; }
    void set_socket(const CloudServerTypes::cloud_socket_t socket) { socket_ = socket; }
    void set_db_path(const CloudTypes::cloud_path_t& path) { database_path = path; }
    void set_tmp_db(const CloudTypes::cloud_temp_t& temp) { database_temp = temp; }

    [[nodiscard]] CloudServerTypes::cloud_header_t get_cloud_header() const { return header_; }
    [[nodiscard]] CloudServerTypes::cloud_socket_t get_cloud_socket() const { return socket_; }
    [[nodiscard]] CloudServerTypes::cloud_address_t get_cloud_address() const { return inet_ntoa(header_.sin_addr); }
    [[nodiscard]] CloudServerTypes::cloud_port_t get_cloud_port() const { return htons(header_.sin_port); }
    [[nodiscard]] CloudTypes::cloud_path_t get_cloud_db_path() const { return database_path; }
    [[nodiscard]] CloudTypes::cloud_temp_t get_cloud_db_temp() const { return database_temp; }

    CloudErrors cloud_socket_init();
    CloudErrors cloud_connect();
    CloudErrors cloud_upload_handler(const CloudTypes::cloud_path_t&);
    CloudErrors cloud_download_handler();

    void cloud_shutdown();
    void cloud_SOS_protocol();

    ~Cloud();
};

#endif // _WIN32
#endif // __CLOUD