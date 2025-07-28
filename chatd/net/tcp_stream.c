#include <chatd/net/tcp_stream.h>


enum tcp_stream_result tcp_stream_destroy(struct tcp_stream *stream)
{
	if (stream->sockfd == -1)
		return TCP_STREAM_UNEXPECTED;

	shutdown(stream->sockfd, SHUT_RD);
	stream->sockfd = -1;

	return TCP_STREAM_OK;
}
