#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <chatd/collections/vec.hpp>
#include <chatd/net/connection.hpp>

#include <cstddef>
#include <memory>


#define WELCOME_MESSAGE "Welcome to the chat relay!\n" \
                        "Please enter your username: "

/**
 * class User - User data
 */
class User {
public:
    User(const char *username_, const char *password_)
        : username { username_ }, password { password_ }
    {}

    ~User() = default;

    const char *username;
    const char *password;

    Vec channels;
};

/**
 * class Payload - Base class for all protocol payloads
 *
 * Abstract base class that defines the interface for all message payloads in
 * the chatd protocol. Provides polymorphic behavior for different message
 * types.
 */
class Payload {
public:
    virtual ~Payload();

    enum class Kind {
        Message, Command
    };

    /**
     * parse_payload() - Parses cstring payloads
     *
     * Buffer should be null-terminated even thoough buffer length is known.
     */
    static Payload *parse(char *buff, size_t len);

    virtual inline Payload::Kind kind() = 0;
    std::shared_ptr<User> user;

protected:
    char *buff{NULL};
    size_t len;

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
    virtual ~Message() = default;

    virtual inline Payload::Kind kind() { return Kind::Message; };

    /**
     * send() - Serialize message to string representation and send it to a
     *          client
     * @client: Individual client that payload sent to.
     *
     * Converts the message object into a string format suitable for
     * transmission over the network and sends it.
     */
    virtual void send(Connection &) = 0;
};

/**
 * class DirectMessage - Private message between users
 */
class DirectMessage : public Message {
public:
    DirectMessage() = default;
    virtual ~DirectMessage() = default;

    void send(Connection &) override;

    char *content;
    char *to;
};

/**
 * class GroupMessage - Channel message for group communication
 */
class GroupMessage : public Message {
public:
    GroupMessage() = default;
    virtual ~GroupMessage() = default;

    void send(Connection &) override;

    char *content;
    char *to;
};

/**
 * class GlobalMessage - Global message that sent to all channels client
 *                       subscribed to.
 */
class GlobalMessage : public Message {
public:
    GlobalMessage() = default;
    virtual ~GlobalMessage() = default;

    void send(Connection &) override;

    char *content;
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
    virtual ~Command() = default;

    virtual inline Payload::Kind kind() { return Kind::Command; };

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
    Subscribe() = default;
    virtual ~Subscribe() = default;

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
    Unsubscribe() = default;
    virtual ~Unsubscribe() = default;

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
    ListUsers() = default;
    virtual ~ListUsers() = default;

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
    Delete() = default;
    virtual ~Delete() = default;

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
    Logout() = default;
    virtual ~Logout() = default;

    void operator()(Connection &, ConnectionPool &) override;
};

}

#endif
