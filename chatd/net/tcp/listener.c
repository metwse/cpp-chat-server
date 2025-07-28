#include <chatd/net/tcp/listener.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/unistd.h>
#include <sys/socket.h>

#include <stdint.h>


enum tcp_listener_result tcp_listener_init(struct tcp_listener *listener,
					   const char *host,
					   uint16_t port)
{
	static int setsockopt_optval = 1;
	struct sockaddr_in srvaddr;
	struct in_addr addr;

	if (inet_pton(AF_INET, host, &addr) != 1)
		return TCP_LISTENER_INVALID_HOSTNAME;

	srvaddr.sin_addr = addr;
	srvaddr.sin_family = AF_INET;
	// sockaddr_in requires the port in network byte order. htons converts
	// the port value from host byte order (usually little-endian) to
	// network byte order (big-endian).
	srvaddr.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		return TCP_LISTENER_CANNOT_CREATE_SOCKET;

	// Allows reuse of local addresses. Makes the socket reusable
	// immediately after it is closed.
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
		   &setsockopt_optval, sizeof(setsockopt_optval));

	if (bind(sockfd, (struct sockaddr *) &srvaddr, sizeof(srvaddr)) != 0)
		return TCP_LISTENER_CANNOT_BIND_SOCKET;

	if (listen(sockfd, TCP_LISTENER_BACKLOG) != 0)
		return TCP_LISTENER_LISTEN_FAILED;

	listener->sockfd = sockfd;

	return TCP_LISTENER_OK;
}

enum tcp_listener_result tcp_listener_destroy(struct tcp_listener *listener)
{
	if (listener->sockfd == -1)
		return TCP_LISTENER_UNEXPECTED;

	close(listener->sockfd);
	listener->sockfd = -1;

	return TCP_LISTENER_OK;
}

enum tcp_listener_result tcp_listener_accept(const struct tcp_listener *listener,
					     struct tcp_stream *stream)
{

	static socklen_t len = sizeof(struct sockaddr_in);

	int sockfd = accept4(listener->sockfd,
			     (struct sockaddr *) &stream->remote_addr,
			     &len, SOCK_NONBLOCK);

	if (sockfd == -1)
		return TCP_LISTENER_ACCEPT_FAILED;

	stream->sockfd = sockfd;

	return TCP_LISTENER_OK;
}
