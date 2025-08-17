#include <cstdlib>
#include <cstring>
#include <mutex>

#include <chatd/protocol/protocol.hpp>
#include <chatd/net/connection.hpp>
#include <chatd/net/server.hpp>


void cmd::Subscribe::operator()(Connection *c, ConnectionPool *pool) {
    auto name = (char *) args[0];
    auto password = (char *) args[1];

    auto user = c->user;
    std::lock_guard<std::mutex> gurad(user->channels_m);

    for (size_t i = 0; i < user->channels.get_size(); i++) {
        if (!strcmp((*(std::shared_ptr<Channel> *) user->channels[i])->name, name)) {
            c->send_strliteral("ERR: You are already in the channel!\n");
            return;
        }
    }

    auto channel = pool->get<Channel>(name);
    auto *joined = new std::shared_ptr<Channel> { nullptr };

    if (channel) {
        if (!strcmp(password, channel->password)) {
            c->send_strliteral("SRV: Joined to the channel.\n");

            *joined = channel;
        } else {
            c->send_strliteral("ERR: Invalid password!\n");
        }
    } else {
        char *cpy_name = new char[strlen(name) + 1];
        char *cpy_password = new char[strlen(password) + 1];
        strcpy(cpy_name, name);
        cpy_name[strlen(name)] = '\0';
        strcpy(cpy_password, password);
        cpy_password[strlen(password)] = '\0';

        auto new_channel = std::make_shared<Channel>(cpy_name, cpy_password);
        std::shared_ptr<Channel> *new_channel_heap = new std::shared_ptr<Channel>;
        *new_channel_heap = std::shared_ptr<Channel>(new_channel);

        if (pool->push<Channel>(new_channel_heap)) {
            c->send_strliteral("SRV: Channel created.\n");

            *joined = new_channel;
        } else {
            c->send_strliteral("ERR: Could not create channel.\n");
            delete new_channel_heap;
        }
    }

    if (*joined) {
        user->channels.push(joined);
        auto user_ref = new std::shared_ptr<User>();
        *user_ref = user;
        std::lock_guard<std::mutex> guard((*joined)->users_m);
        (*joined)->users.push(user_ref);
    } else
        delete joined;
}

void cmd::Unsubscribe::operator()(Connection *c, ConnectionPool *pool) {
    auto name = (char *) args[0];

    auto user = c->user;
    std::lock_guard<std::mutex> gurad(user->channels_m);

    for (size_t i = 0; i < user->channels.get_size(); i++) {
        auto channel = *(std::shared_ptr<Channel> *) user->channels[i];
        std::lock_guard<std::mutex> guard_channel(channel->users_m);

        if (!strcmp(channel->name, name)) {
            user->channels.remove(i);

            for (size_t j = 0; j < channel->users.get_size(); j++) {
                auto ch_user = *(std::shared_ptr<User> *) channel->users[j];

                if (!strcmp(ch_user->name, user->name)) {
                    channel->users.remove(j);
                    break;
                }
            }

            if (channel->users.get_size() == 0)
                pool->remove<Channel>(channel->name);

            c->send_strliteral("SRV: Unsubscribed!\n");

            return;
        }
    }

    c->send_strliteral("ERR: You not in the channel!\n");
}

void cmd::Delete::operator()(Connection *, ConnectionPool *) {}

void cmd::Logout::operator()(Connection *, ConnectionPool *) {}

void cmd::ListUsers::operator()(Connection *, ConnectionPool *) {}
