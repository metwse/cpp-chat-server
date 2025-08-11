#ifndef TCP_STREAM_H
#define TCP_STREAM_H

#include <netinet/in.h>
#include <stdint.h>

#define TCP_STREAM_BUFF_CAP 128


/**
 * struct tcp_stream - TCP/IP stream.
 */
struct tcp_stream {
	int sockfd;
	struct sockaddr_in remote_addr;
	char *buff;
	size_t buff_len;
};

enum tcp_stream_result {
	TCP_STREAM_OK,
	TCP_STREAM_INVALID_HOSTNAME,
	TCP_STREAM_CANNOT_CREATE_SOCKET,
	TCP_STREAM_CANNOT_BIND_SOCKET,
	TCP_STREAM_CONNECT_FAILED,
	TCP_STREAM_UNEXPECTED,
	TCP_STREAM_NOMEM,
	TCP_STREAM_READE,
	TCP_STREAM_WRITEE,
	TCP_STREAM_TERMINATED_LINE,
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

/**
 * tcp_stream_readuntil() - Reads until given char from TCP stream.
 *
 * Ensures the outbuffer null-terminated.
 */
enum tcp_stream_result tcp_stream_readuntil(struct tcp_stream *,
					    char c, char **out, size_t *len);

/**
 * tcp_stream_write() - Writes given buffer to socket.
 *
 * Ensures the outbuffer null-terminated.
 */
enum tcp_stream_result tcp_stream_write(struct tcp_stream *,
					const char *buff, size_t len);

#endif
