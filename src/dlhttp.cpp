#include <dlhttp.h>

#include <algorithm>
#include <array>
#include <doctest/doctest.h>
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

std::vector<std::string> dlhttp::parse::split(const std::string& input, const std::string& delim) {
    std::vector<std::string> result;
    // iterator to keep track of current position inside input
    auto start = input.begin();
    while (start < input.end()) {
        // search for the tokens
        const auto iter = std::search(start, input.end(), delim.begin(), delim.end());
        if (start != input.end()) {
            // if a token was found, it's inserted into the result,
            // and the iterator is moved forwards for the next run.
            result.emplace_back(start, iter);
            start = iter + delim.size();
        } else {
            break;
        }
    }
    return result;
}

TEST_CASE("dlhttp::parse::split") {
    SUBCASE("string, single element delim") {
        std::string str = "Hello, World!";
        auto res = dlhttp::parse::split(str, ",");
        CHECK(res.size() == 2);
        std::string first(res.at(0).begin(), res.at(0).end());
        CHECK(first == "Hello");
        std::string second(res.at(1).begin(), res.at(1).end());
        CHECK(second == " World!");
    }
    SUBCASE("string, multi element delim") {
        std::string str = "Hello, World!";
        auto res = dlhttp::parse::split(str, "llo");
        CHECK(res.size() == 2);
        std::string first(res.at(0).begin(), res.at(0).end());
        CHECK(first == "He");
        std::string second(res.at(1).begin(), res.at(1).end());
        CHECK(second == ", World!");
    }
}

void dlhttp::handle_http(SOCKET sock, AsyncContext& ctx, const EndpointHandlerMap& ep_map) {
    // parse http
    // get target
    // get target handler
    // send to handler...?
    std::string request_line = "";
    const std::string crlf = "\r\n";
    while (std::search(request_line.begin(), request_line.end(), crlf.begin(), crlf.end()) == request_line.end()) {
        std::array<char, 128> buf;
        auto ret = ::recv(sock, reinterpret_cast<char*>(buf.data()), buf.size(), 0);
        if (ret == SOCKET_ERROR) {
            detail::close_socket(sock);
            throw std::runtime_error("recv() failed in handle_http");
        }
        request_line += std::string(buf.begin(), buf.begin() + ret);
    }
    auto splits = parse::split(request_line, " ");
    if (splits.size() < 3) {
        detail::close_socket(sock);
        throw std::runtime_error("invalid http request");
    }
    if (splits.at(0) != "GET") {
        ::send(sock, reinterpret_cast<const char*>(HTTP_501.data()), HTTP_501.size(), 0);
        detail::close_socket(sock);
    }
    auto task = ctx.pool->enqueue([crlf, sock, splits, &ep_map](int) {
        if (ep_map.find(splits.at(1)) != ep_map.end()) {
            Response response_data = ep_map.at(splits.at(1))();
            switch (response_data.status) {
            case dlhttp::Response::Status::NotFound_404:
                ::send(sock, reinterpret_cast<const char*>(HTTP_404.data()), HTTP_404.size(), 0);
            case dlhttp::Response::Status::Ok_200:
                // fallthrough
            default:
                ::send(sock, reinterpret_cast<const char*>(HTTP_200.data()), HTTP_200.size(), 0);
            }
            ::send(sock, reinterpret_cast<const char*>(crlf.data()), crlf.size(), 0);
            if (response_data.body.index() == 0) {
                auto str = std::get<0>(response_data.body);
                ::send(sock, reinterpret_cast<const char*>(str.c_str()), str.size(), 0);
            } else {
                auto vec = std::get<1>(response_data.body);
                ::send(sock, reinterpret_cast<const char*>(vec.data()), vec.size(), 0);
            }
        } else {
            ::send(sock, reinterpret_cast<const char*>(HTTP_404.data()), HTTP_404.size(), 0);
        }
        detail::close_socket(sock);
    },
        0);
}

dlhttp::AsyncContext::AsyncContext(size_t pool_size)
    : pool { std::make_unique<ThreadPool>(pool_size) } {
}

bool dlhttp::is_http(SOCKET sock) {
    if (sock == INVALID_SOCKET) {
        return false;
    }
    std::array<uint8_t, 4> buf;
    auto ret = recv(sock, reinterpret_cast<char*>(buf.data()), buf.size(), MSG_PEEK);
    if (ret == SOCKET_ERROR) {
        throw std::runtime_error("recv() failed");
    }
    const std::array<uint8_t, 4> expected { 'G', 'E', 'T', ' ' };
    if (ret >= 3 && std::equal(buf.begin(), buf.begin() + size_t(ret), expected.begin())) {
        return true;
    }
    return false;
}
