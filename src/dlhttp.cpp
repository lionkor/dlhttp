#include <dlhttp.h>

#include <algorithm>
#include <array>
#include <exception>
#include <stdexcept>

namespace dlhttp::detail {

int close_socket(SOCKET sock) {
    int status = 0;

#ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0) {
        status = closesocket(sock);
    }
#else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0) {
        status = close(sock);
    }
#endif
    return status;
}

}

dlhttp::AsyncContext::AsyncContext()
    : m_thread { &dlhttp::AsyncContext::thread_main, this } {
}

void dlhttp::AsyncContext::thread_main() {
    
}

bool dlhttp::is_http(SOCKET sock) {
    if (sock == INVALID_SOCKET) {
        return false;
    }
    std::array<uint8_t, 4> buf;
    auto ret = recv(sock, buf.data(), buf.size(), MSG_PEEK);
    if (ret == SOCKET_ERROR) {
        throw std::runtime_error("recv() failed");
    }
    const std::array<uint8_t, 4> expected { 'G', 'E', 'T', ' ' };
    if (ret >= 3 && std::equal(buf.begin(), buf.begin() + size_t(ret), expected.begin())) {
        return true;
    }
    return false;
}
