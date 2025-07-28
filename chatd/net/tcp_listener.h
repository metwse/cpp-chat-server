#ifndef TCP_LISTENER_H
#define TCP_LISTENER_H

#include <chatd/net/tcp_stream.h>
#include <stdint.h>

#define TCP_LISTENER_BACKLOG 8

/**
 * struct tcp_listener - TCP/IP listener.
 */
struct tcp_listener {
	int sockfd;
};

enum tcp_listener_result {
	TCP_LISTENER_OK,
	TCP_LISTENER_INVALID_HOSTNAME,
	TCP_LISTENER_CANNOT_CREATE_SOCKET,
	TCP_LISTENER_CANNOT_BIND_SOCKET,
	TCP_LISTENER_LISTEN_FAILED,
	TCP_LISTENER_ACCEPT_FAILED,
	TCP_LISTENER_UNEXPECTED,
};


/**
 * tcp_listener_init() - Initializes a TCP listener socket.
 * @listener: Pointer to a tcp_listener struct to be initialized.
 * @host: IP address (as a string) to bind the listener to.
 * @port: Port number to listen on.
 *
 * Creates and sets up a TCP socket for listening on the given IP address
 * and port. The function also enables SO_REUSEADDR to allow quick reuse of
 * the socket after it is closed. The listener is bound to the address and
 * starts listening with a defined backlog.
 *
 * Return:
 * * TCP_LISTENER_OK - on success
 * * TCP_LISTENER_INVALID_HOSTNAME - if the IP address is invalid
 * * TCP_LISTENER_CANNOT_CREATE_SOCKET - if socket() fails
 * * TCP_LISTENER_CANNOT_BIND_SOCKET - if bind() fails
 * * TCP_LISTENER_LISTEN_FAILED - if listen() fails
 */
enum tcp_listener_result tcp_listener_init(struct tcp_listener *listener,
					   const char *host,
					   uint16_t port);

/**
 * tcp_listener_destroy() - Disconnects the tcp_listener from socket.
 *
 * Closes all open sockets, disconnects clients, and releases
 * any allocated resources used by the tcp_listener.
 */
enum tcp_listener_result tcp_listener_destroy(struct tcp_listener *);

/**
 * tcp_listener_accept() - Accepts an incoming TCP connection
 * @listener: Pointer to the TCP listener instance
 * @stream: Pointer to a stream that will be initialized upon accepting a
 *          connection
 *
 * Blocks until an incoming connection is received on the listener socket.
 * If successful, initializes the provided @stream with the accepted
 * connection's socket descriptor and peer address.
 */
enum tcp_listener_result tcp_listener_accept(const struct tcp_listener *listener,
					     struct tcp_stream *stream);

#endif
