#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <chatd/collections/vec.hpp>
#include <chatd/net/connection.hpp>


/**
 * class Payload - Base class for all protocol payloads
 *
 * Abstract base class that defines the interface for all message payloads in
 * the chatd protocol. Provides polymorphic behavior for different message
 * types.
 */
class Payload {
public:
    virtual ~Payload() = 0;
};

namespace msg {

/**
 * class Message - Base message class for chatd protocol
 *
 * Represents a basic message in the chatd protocol containing sender
 * information and message data. Serves as base class for specialized message
 * types.
 */
class Message : public Payload {
public:
    /**
     * send() - Serialize message to string representation and send it to a
     *          client
     * @client: Individual client that payload sent to.
     *
     * Converts the message object into a string format suitable for
     * transmission over the network and sends it.
     */
    virtual void send(Connection &) = 0;

    /**
     * @from: Username of the message sender
     *
     * Null-terminated string containing the username of the client who sent
     * this message.
     */
    char *from;

    /**
     * @data: Message content
     *
     * Null-terminated string containing the actual message text sent by the
     * user.
     */
    char *data;
};

/**
 * class DirectMessage - Private message between users
 */
class DirectMessage : public Message {
public:
    DirectMessage(char *from, char *to, char *data);

    void send(Connection &) override;

    /**
     * @to: Username of the message recipient
     *
     * Null-terminated string containing the username of the intended recipient
     * for this private message.
     */
    char *to;
};

/**
 * class GroupMessage - Channel message for group communication
 */
class GroupMessage : public Message {
public:
    GroupMessage(char *from, char *to, char *data);

    void send(Connection &) override;

    /**
     * @to: Name of the target channel
     *
     * Null-terminated string containing the name of the channel where this
     * message should be delivered.
     */
    char *to;
};

/**
 * class GlobalMessage - Global message that sent to all channels client
 *                       subscribed to.
 */
class GlobalMessage : public Message {
public:
    GlobalMessage(char *from, char *data);

    void send(Connection &) override;
};

}


namespace cmd {

/**
 * class Command - Base class for all chatd protocol commands
 *
 * Abstract base class that defines the interface for all protocol commands.
 * Commands are executed with access to the originating connection and the
 * connection pool for managing server state.
 */
class Command : public Payload {
public:
    /**
     * operator()() - Execute the command
     * @conn: Connection that issued the command
     * @pool: Connection pool for server-wide operations
     *
     * Pure virtual function that executes the specific command logic.
     */
    virtual void operator()(Connection &, ConnectionPool &) = 0;

    /**
     * @args: Command arguments
     *
     * Vector containing the parsed arguments for this command, extracted from
     * the original command string.
     */
    Vec args;
};

/**
 * class Subscribe - Handle /subscribe command
 *
 * Implements the `/subscribe <channel_name> <channel_password>` command
 * that allows users to subscribe to channels for receiving messages.
 */
class Subscribe : public Command {
public:
    void operator()(Connection &, ConnectionPool &) override;
};

/**
 * class Unsubscribe - Handle /unsubscribe command
 *
 * Implements the `/unsubscribe <channel_name>` command that removes the user's
 * subscription from the specified channel.
 */
class Unsubscribe : public Command {
public:
    void operator()(Connection &, ConnectionPool &) override;
};

/**
 * class ListUsers - Handle /list_users command
 *
 * Implements the `/list_users <channel_name>` command that lists all users
 * currently subscribed to the specified channel. Only available to channel
 * subscribers.
 */
class ListUsers : public Command {
public:
    void operator()(Connection &, ConnectionPool &) override;
};

/**
 * class Delete - Handle /delete command
 *
 * Implements the `/delete <channel_name>` command that deletes the specified
 * channel. Only available to channel subscribers.
 */
class Delete : public Command {
public:
    void operator()(Connection &, ConnectionPool &) override;
};

/**
 * class Logout - Handle /logout command
 *
 * Implements the /logout command that logs out and unauthenticates the current
 * connection.
 */
class Logout : public Command {
public:
    void operator()(Connection &, ConnectionPool &) override;
};

}

#endif
