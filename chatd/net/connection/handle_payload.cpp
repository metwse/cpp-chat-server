#include <cstring>
#include <cctype>
#include <cstdlib>

#include <chatd/protocol/protocol.hpp>
#include <chatd/net/connection.hpp>
#include <chatd/net/server.hpp>
#include <mutex>


void Connection::handle_payload(char *buff, size_t len) {
    thread_local bool authenticated { false }, new_account { false };
    thread_local char *username { NULL }, *password { NULL };

    if (buff == NULL) {
        if (!authenticated) {
            if (username)
                free(username);
            if (password)
                free(password);
        }

        return;
    }

    if (authenticated) {
        auto payload = Payload::parse(buff, len);

        if (!payload) {
            send_strliteral("ERR: Invalid command!\n");
            return;
        }

        if (payload->kind() == Payload::Kind::Command) {
            auto cmd = dynamic_cast<cmd::Command *>(payload);
            (*cmd)(this, this->m_pool);
            delete payload;
        } else if (payload->kind() == Payload::Kind::Message) {
            auto msg = dynamic_cast<msg::Message *>(payload);
            msg->user = this->user;

            if (typeid(*payload) == typeid(msg::DirectMessage)) {
                auto dmsg = dynamic_cast<msg::DirectMessage *>(payload);
                m_pool->send_msg(dmsg->to, msg);
            }
            else if (typeid(*payload) == typeid(msg::GroupMessage)) {
                auto gmsg = dynamic_cast<msg::GroupMessage *>(payload);
                auto channel = m_pool->get<Channel>(gmsg->to);

                bool is_in_channel = false;

                if (channel) {
                    std::lock_guard<std::mutex> guard(channel->users_m);
                    for (size_t i = 0; i < channel->users.get_size(); i++) {
                        auto ch_user = *(std::shared_ptr<User> *)
                            channel->users[i];

                        if (!strcmp(user->name, ch_user->name)) {
                            is_in_channel = true;
                            break;
                        }
                    }

                    if (is_in_channel) {
                        for (size_t i = 0; i < channel->users.get_size(); i++) {
                            auto ch_user = *(std::shared_ptr<User> *)
                                channel->users[i];

                            if (strcmp(user->name, ch_user->name))
                                m_pool->send_msg(ch_user->name, msg);
                        }
                    }
                }

                if (!is_in_channel)
                    send_strliteral("ERR: Channel not found!\n");
            } else if (typeid(*payload) == typeid(msg::GlobalMessage)) {
                std::lock_guard<std::mutex> guard(user->channels_m);

                if (!user->channels.get_size())
                    send_strliteral("ERR: You are not subscribed to any "
                                    "channel!\n");

                for (size_t i = 0; i < user->channels.get_size(); i++) {
                    auto channel = *(std::shared_ptr<Channel> *)
                        user->channels[i];

                    for (size_t j = 0; j < channel->users.get_size(); j++) {
                        auto ch_user = *(std::shared_ptr<User> *)
                            channel->users[j];

                        if (strcmp(user->name, ch_user->name))
                            m_pool->send_msg(ch_user->name, msg);
                    }
                }

            }
        }
    } else {
        if (buff[len - 1] == '\r') {
            buff[len - 1] = '\0';
            len--;
        }

        if (username == NULL) {
            ssize_t i = 0;

            while (buff[i] != '\0') {
                if (!(isalnum(buff[i]) || buff[i] == '_')) {
                    send_strliteral("ERR: Username contains illegal character"
                                    "(s).\nERR: Try again: ");
                    free(buff);
                    buff = NULL;
                    break;
                }
                i++;
            }

            if (buff) {
                if (i < 2 || i > 20) {
                    send_strliteral("ERR: Username should between 2 and 20 "
                                    "characters length.\nERR: Try again: ");
                    free(buff);
                } else {
                    auto username_cpy = (char *) malloc(sizeof(char) *
                                                        (len + 1));
                    strcpy(username_cpy, buff);
                    send_strliteral("SRV: Hello @");
                    send(username_cpy, len);
                    send_strliteral("\nSRV: Please enter your password: ");
                    username = buff;
                }
            }
        } else if (password == NULL) {
            if (strlen(buff) != len) {
                send_strliteral("ERR: Password contains null-byte.");

                free(buff);
            } else if (len < 8 || len > 64) {
                send_strliteral("ERR: Password should between 8 and 64 "
                                "characters length.\nERR: Try again: ");

                free(buff);
            } else {
                password = buff;
            }
       }

        if (username && password) {
            if (new_account && (buff[0] == 'y' || buff[0] == 'n')) {
                if (buff[0] == 'y') {
                    auto new_user = std::make_shared<User>(username, password);
                    auto new_user_heap = new std::shared_ptr<User>;
                    *new_user_heap = new_user;
                    if (m_pool->push<User>(new_user_heap)) {
                        send_strliteral("SRV: Hello @");
                        send_strliteral(username);
                        send_strliteral("!\n");
                        authenticated = true;
                        user = *new_user_heap;
                    } else {
                        send_strliteral("EXIT: Cannot create user.\n");
                        delete new_user_heap;
                        gracefully_terminate();
                    }
                } else {
                    send_strliteral("EXIT: Bye!\n");
                    gracefully_terminate();
                }
            } else {
                auto existing_user = m_pool->get<User>(username);
                if (existing_user) {
                    if (!strcmp(existing_user->password, password)) {
                        send_strliteral("SRV: Hello @");
                        send(username, strlen(username));
                        send_strliteral("!\n");
                        authenticated = true;
                        user = existing_user;
                    } else {
                        send_strliteral("ERR: Invalid password! Try again: ");
                    }
                    free(password);
                    password = NULL;
                } else {
                    send_strliteral("SRV: No user has found, would you like to "
                                    "a account? (y/n): ");
                    new_account = true;
                }
            }
        }
    }
}
