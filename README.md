# dlhttp

A stub http implementation to provide simple GET responses over a TCP socket. Server services running on consumer machines often rely on port-forwarding, and thus using more than one port can be a pain. UDP and TCP, for example for game servers, can already use the same port, but if http is required alongside game TCP, this is non-trivial to build.

dlhttp solves this issue, by providing an intermediate layer through which all new TCP connections run. This means that dlhttp acts between an `accept()` and the services main TCP handling code.

To do this, it performs a highly optimized check to test for http, and either acts on the http request, or hands the socket right back.

This code is AGPL-3.0 licensed, see the "LICENSE" file for more information. To come to an agreement for commercial usage, contact the author (@lionkor, development(at)kortlepel.com).

## usage

```cpp
// RAII will handle threads, etc.
dlhttp::AsyncContext ctx{};

dlhttp::EndpointHandlerMap handlers {
	{ "/", []() { return {200, "this is a dlhttp server :^)"} } },
	{ "/info", []() { return {200, "i'm online!"} } },
};

// accept loop
while (...) {
	int client_sock = accept(server_sock, nullptr, nullptr)
	
	if (dlhttp::is_http(client_sock)) {
		dlhttp::handle_http_async(ctx, handlers);
	}
}
```