#include <chatd/net/tcp/stream.h>
#include <chatd/net/tcp/listener.h>

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define TEST_HOST "127.0.0.1"
#define TEST_PORT 21242


void *stream_thread_f(void *arg)
{
	struct tcp_stream conn;
	assert(!tcp_stream_init(&conn, TEST_HOST, TEST_PORT));

	char buff[14] = {};
	read(conn.sockfd, buff, 14);

	printf("Received from server: %s\n", buff);
	dprintf(conn.sockfd, "Hello, server!");

	tcp_stream_destroy(&conn);

	return NULL;
}

int main()
{
	struct tcp_listener listener;
	assert(!tcp_listener_init(&listener, TEST_HOST, TEST_PORT));

	for (int _fuzz = 0; _fuzz < 32; _fuzz++) {
		pthread_t stream_thread;
		struct tcp_stream remote_conn;

		pthread_create(&stream_thread, NULL, stream_thread_f, NULL);

		assert(!tcp_listener_accept(&listener, &remote_conn));

		dprintf(remote_conn.sockfd, "Hello, world!");

		char buff[15] = {};
		read(remote_conn.sockfd, buff, 15);

		printf("Received from remote: %s\n", buff);

		pthread_join(stream_thread, NULL);
		tcp_stream_destroy(&remote_conn);
	}

	tcp_listener_destroy(&listener);
}
