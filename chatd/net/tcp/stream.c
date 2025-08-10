#include <chatd/net/tcp/stream.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/unistd.h>


enum tcp_stream_result tcp_stream_readuntil(struct tcp_stream *stream,
					    char c, char **out, size_t *len)
{
	for (size_t i = 0; i < stream->buff_len; i++) {
		if (stream->buff[i] == c) {
			*out = malloc(sizeof(char) * (i + 1));
			if (*out == NULL)
				return TCP_STREAM_NOMEM;
			*len = i;

			memcpy(*out, stream->buff, i * sizeof(char));
			(*out)[i] = '\0';

			if (stream->buff_len > 0) {
				for (size_t j = i + 1; j < stream->buff_len; j++)
					stream->buff[j - i - 1] =
						stream->buff[j];

				stream->buff_len -= i + 1;
			}

			return TCP_STREAM_OK;
		}
	}

	char buff[TCP_STREAM_BUFF_CAP];
	char *outbuff = stream->buff;
	*len = stream->buff_len;

	stream->buff = NULL;
	stream->buff_len = 0;

	while (true) {
		size_t read_bytes = read(stream->sockfd,
					 buff, TCP_STREAM_BUFF_CAP);
		if (read_bytes <= 0)
			break;

		outbuff = realloc(outbuff, *len + read_bytes);
		if (outbuff == NULL)
			return TCP_STREAM_NOMEM;

		bool found = false;
		for (size_t i = 0; i < read_bytes; i++) {
			if (found == true) {
				stream->buff[stream->buff_len++] = buff[i];
			} else {
				if (buff[i] == c) {
					found = true;
					stream->buff = malloc(sizeof(char) *
							      TCP_STREAM_BUFF_CAP);
					if (stream->buff == NULL)
						return TCP_STREAM_NOMEM;
				} else {
					outbuff[(*len)++] = buff[i];
				}
			}
		}

		if (found) {
			if (*len == 0) {
				free(outbuff);
				*out = NULL;
			} else {
				*out = realloc(outbuff, *len + 1);
				(*out)[*len] = '\0';
			}

			return TCP_STREAM_OK;
		}
	}

	free(outbuff);
	return TCP_STREAM_READE;
}

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

	stream->buff = malloc(sizeof(char) * TCP_STREAM_BUFF_CAP);
	if (stream->buff == NULL)
		return TCP_STREAM_NOMEM;

	stream->buff_len = 0;

	return TCP_STREAM_OK;
}

enum tcp_stream_result tcp_stream_destroy(struct tcp_stream *stream)
{
	if (stream->buff) {
		free(stream->buff);
		stream->buff = NULL;
	}

	if (stream->sockfd == -1)
		return TCP_STREAM_UNEXPECTED;

	shutdown(stream->sockfd, SHUT_RD);
	close(stream->sockfd);
	stream->sockfd = -1;

	return TCP_STREAM_OK;
}
