#include <chatd/net/tcp/stream.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/unistd.h>


enum tcp_stream_result tcp_stream_init(struct tcp_stream *stream,
				       const char *host,
				       uint16_t port)
{
	struct in_addr addr;

	if (inet_pton(AF_INET, host, &addr) != 1)
		return TCP_STREAM_INVALID_HOSTNAME;

	stream->remote_addr.sin_addr = addr;
	stream->remote_addr.sin_family = AF_INET;
	stream->remote_addr.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		return TCP_STREAM_CANNOT_CREATE_SOCKET;

	if (connect(sockfd,
		    (struct sockaddr *) &stream->remote_addr,
		    sizeof(stream->remote_addr)) != 0)
		return TCP_STREAM_CONNECT_FAILED;

	stream->sockfd = sockfd;

	return TCP_STREAM_OK;
}

enum tcp_stream_result tcp_stream_destroy(struct tcp_stream *stream)
{
	if (stream->sockfd == -1)
		return TCP_STREAM_UNEXPECTED;

	shutdown(stream->sockfd, SHUT_RD);
	close(stream->sockfd);
	stream->sockfd = -1;

	return TCP_STREAM_OK;
}
