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
    using SOCKET = int;
    static constexpr SOCKET INVALID_SOCKET = -1;
    static constexpr ssize_t SOCKET_ERROR = -1;
#endif

// clang-format on

namespace dlhttp {

bool is_http(SOCKET);

}
