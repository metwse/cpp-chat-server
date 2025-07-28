#ifndef TCP_STREAM_H
#define TCP_STREAM_H

#include <netinet/in.h>
#include <stdint.h>

/**
 * struct tcp_stream - TCP/IP stream.
 */
struct tcp_stream {
	int sockfd;
	struct sockaddr_in remote_addr;
};

enum tcp_stream_result {
	TCP_STREAM_OK,
	TCP_STREAM_INVALID_HOSTNAME,
	TCP_STREAM_CANNOT_CREATE_SOCKET,
	TCP_STREAM_CANNOT_BIND_SOCKET,
	TCP_STREAM_CONNECT_FAILED,
	TCP_STREAM_UNEXPECTED,
};


/**
 * tcp_stream_inti() - Connects to given host and port.
 */
enum tcp_stream_result tcp_stream_init(struct tcp_stream *,
				       const char *host,
				       uint16_t port);

/**
 * tcp_stream_destroy() - Disconnects the tcp_stream from socket.
 */
enum tcp_stream_result tcp_stream_destroy(struct tcp_stream *);

#endif
