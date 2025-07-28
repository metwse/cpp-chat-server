#ifndef ASYNCIO_H
#define ASYNCIO_H

#include <chatd/net/tcp/stream.h>
#include <chatd/net/tcp/listener.h>


struct async_io {
	int sockfd;
};

#endif
