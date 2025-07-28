#ifndef ASYNCIO_H
#define ASYNCIO_H

#include <chatd/net/tcp/stream.h>
#include <chatd/net/tcp/listener.h>
#include <chatd/collections/vec.h>
#include <pthread.h>
#include <stdbool.h>

/**
 * struct tcp_connection - Represents a single TCP connection
 * @is_active: Indicates whether the connection is still active
 * @write_lock: Mutex to guard access to the write queue
 * @write_queue: Queue of pending write operations
 * @sockfd: File descriptor of the socket
 *
 * Represents the internal state of a single client TCP connection. Write
 * operations are queued and guarded by a mutex.
 */
struct tcp_connection {
	bool is_active;
	pthread_mutex_t write_lock;
	// TODO: Implement vec_dequeue
	struct vec write_queue;
	size_t sockfd;
};

/**
 * struct tcp_data - Represents a data buffer received or to be sent over TCP
 * @len: Length of the data in bytes
 * @data: Pointer to the data buffer
 *
 * Used to handle on_data event.
 */
struct tcp_data {
	size_t len;
	char *data;
};

/**
 * tcp_on_connection - Callback for when a new TCP connection is accepted
 * @conn: Pointer to the accepted connection
 *e@state: User-defined context
 */
typedef void (*tcp_on_connection)(struct tcp_connection *conn,
				  void *state);

/**
 * tcp_on_data - Callback for when data is available to read
 * @conn: Pointer to the TCP connection with data
 * @state: User-defined context
 */
typedef void (*tcp_on_data)(struct tcp_connection *conn,
			    struct tcp_data data,
			    void *state);


/**
 * tcp_serve() - Runs the TCP server loop
 * @listener: The TCP listener to accept connections from
 *
 * Starts accepting connections and handling data using internal event loop and
 * callback mechanisms. Runs synchronously.
 */
enum tcp_listener_result tcp_serve(struct tcp_listener listener);

#endif

