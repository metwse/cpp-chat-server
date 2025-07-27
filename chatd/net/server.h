#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#define SERVER_BACKLOG 8

/**
 * struct server - Chat server instance
 *
 * Represents the main server structure that listens on a TCP socket.
 */
struct server {
	int sockfd;
};

enum server_result {
	SERVER_OK,
	SERVER_INVALID_HOSTNAME,
	SEVER_CANNOT_CREATE_SOCKET,
	SERVER_CANNOT_BIND_SOCKET,
	SERVER_LISTEN_FAILED,
	SERVER_UNEXPECTED,
};


/**
 * server_init() - Initializes a server for listening on a socket.
 * @srv: Pointer to the server instance to initialize
 *
 * Prepares the server to start listening on a socket by creating,
 * binding, and setting up the necessary file descriptors.
 */
enum server_result server_init(struct server *srv, const char *host,
			       uint16_t port);

/**
 * server_destroy() - Stops the server from listening and cleans up.
 * @srv: Pointer to the server instance to destroy
 *
 * Closes all open sockets, disconnects clients, and releases
 * any allocated resources used by the server.
 */
enum server_result server_destroy(struct server *srv);


#endif
