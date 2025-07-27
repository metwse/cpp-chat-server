#include <chatd/net/server.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdint.h>


enum server_result server_init(struct server *srv, const char *host,
			       uint16_t port)
{
	struct sockaddr_in srvaddr;
	struct in_addr addr;

	if (inet_pton(AF_INET, host, &addr) != 1)
		return SERVER_INVALID_HOSTNAME;

	srvaddr.sin_addr = addr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		return SEVER_CANNOT_CREATE_SOCKET;

	if (bind(sockfd, (struct sockaddr *) &srvaddr, sizeof(srvaddr)) != 0)
		return SERVER_CANNOT_BIND_SOCKET;

	if (listen(sockfd, SERVER_BACKLOG) != 0)
		return SERVER_LISTEN_FAILED;

	srv->sockfd = sockfd;

	return SERVER_OK;
}

enum server_result server_destroy(struct server *srv)
{
	if (srv->sockfd == -1)
		return SERVER_UNEXPECTED;

	shutdown(srv->sockfd, SHUT_RD);
	srv->sockfd = -1;

	return SERVER_OK;
}
