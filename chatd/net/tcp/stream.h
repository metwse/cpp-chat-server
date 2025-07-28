#ifndef TCP_STREAM_H
#define TCP_STREAM_H

#include <netinet/in.h>

/**
 * struct tcp_stream - TCP/IP stream.
 */
struct tcp_stream {
	int sockfd;
	struct sockaddr_in remote_addr;
};

enum tcp_stream_result {
	TCP_STREAM_OK,
	TCP_STREAM_UNEXPECTED,
};


/**
 * tcp_stream_destroy() - Disconnects the tcp_stream from socket.
 */
enum tcp_stream_result tcp_stream_destroy(struct tcp_stream *);

#endif
