#pragma once

#include <cstddef>
#include <cstdint>

// clang-format off
// you may set DLHTTP_NO_SOCK_INCLUDES, and include winsock/posix sockets 
// yourself.
#ifndef DLHTTP_NO_SOCK_INCLUDES
    #ifdef _WIN32
        /* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
        #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0501 /* Windows XP. */
        #endif
        #include <Ws2tcpip.h>
        #include <winsock2.h>
    #else
        /* Assume that any non-Windows platform uses POSIX-style sockets instead. */
        #include <arpa/inet.h>
        #include <netdb.h> /* Needed for getaddrinfo() and freeaddrinfo() */
        #include <sys/socket.h>
        #include <unistd.h> /* Needed for close() */
    #endif
#endif

#ifndef _WIN32
    #define SOCKET int
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
#endif
// clang-format on

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

#include <ThreadPool.h>

class ThreadPool;

namespace dlhttp {

namespace parse {

    /**
     * @brief Splits an input span by single- or multi-element delimiters,
     * and returns a new span with the resulting splits. Empty splits are
     * not removed. Delimiters are not included in the result.
     */
    std::vector<std::string> split(const std::string& input, const std::string& delim);
}

/**
 * @brief The Response class represents a generic response of any kind.
 * Since dlhttp aims to provide only the bare minimum GET capabilities,
 * this only supports string and arbitrary byte buffers.
 */
struct Response {
    uint16_t status { 200 };
    std::variant<std::string, std::vector<uint8_t>> body {};
    std::string content_type {};
};

using Handler = std::function<Response()>;
using EndpointHandlerMap = std::unordered_map<std::string, Handler>;

inline const std::string HTTP_VER = "HTTP/1.0";
inline const std::string HTTP_501 = HTTP_VER + " 501 Not Implemented\r\n";
inline const std::string HTTP_404 = HTTP_VER + " 404 Not Found\r\n";
inline const std::string HTTP_400 = HTTP_VER + " 400 Bad Request\r\n";
inline const std::string HTTP_200 = HTTP_VER + " 200 Ok\r\n";

class AsyncContext final {
public:
    AsyncContext(size_t pool_size);

    std::unique_ptr<ThreadPool> pool;
};

bool is_http(SOCKET);
void handle_http(SOCKET, AsyncContext&, const EndpointHandlerMap&);

}
