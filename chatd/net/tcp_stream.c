#include <chatd/net/tcp_stream.h>

#include <sys/socket.h>
#include <sys/unistd.h>


enum tcp_stream_result tcp_stream_destroy(struct tcp_stream *stream)
{
	if (stream->sockfd == -1)
		return TCP_STREAM_UNEXPECTED;

	shutdown(stream->sockfd, SHUT_RD);
	close(stream->sockfd);
	stream->sockfd = -1;

	return TCP_STREAM_OK;
}
