#include <chatd/net/tcp/stream.h>
#include <chatd/net/tcp/listener.h>

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define TEST_HOST "127.0.0.1"
#define TEST_PORT 21242


char sent_data[128][1024];
size_t sent_data_len[128];


void *stream_thread_f(void *arg)
{
	struct tcp_stream conn;
	assert(!tcp_stream_init(&conn, TEST_HOST, TEST_PORT));

	for (size_t i = 0; i < 128; i++) {
		char *line;
		size_t len;
		assert(!tcp_stream_readuntil(&conn, '\n', &line, &len));

		assert(memcmp(line, sent_data[i], sent_data_len[i]) == 0);
		assert(len == sent_data_len[i]);
		free(line);
	}

	tcp_stream_destroy(&conn);
	return NULL;
}

int main()
{
	struct tcp_listener listener;
	pthread_t stream_thread;
	srand(time(NULL));

	assert(!tcp_listener_init(&listener, TEST_HOST, TEST_PORT));

	pthread_create(&stream_thread, NULL, stream_thread_f, NULL);

	struct tcp_stream remote_conn;
	assert(!tcp_listener_accept(&listener, &remote_conn));

	for (size_t i = 0; i < 128; i++) {
		size_t len = rand() % 1023 + 1;
		for (size_t j = 0; j < len; j++) {
			char r = rand() % sizeof(char);

			if (r == '\n') {
				j--;
				continue;
			}

			sent_data[i][j] = r;
		}

		sent_data[i][len] = '\n';
		sent_data_len[i] = len;

		write(remote_conn.sockfd, sent_data[i], len + 1);
	}

	pthread_join(stream_thread, NULL);

	tcp_stream_destroy(&remote_conn);
	tcp_listener_destroy(&listener);
}
