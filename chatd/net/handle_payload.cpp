#include "chatd/protocol/protocol.hpp"
#include <cstring>
#include <cctype>
#include <cstdlib>

#include <chatd/net/connection.hpp>


void Connection::handle_payload(char *buff, size_t len) {
    thread_local bool authenticated { false }, new_account { false };
    thread_local char *username { NULL }, *password { NULL };

    if (authenticated) {
        auto payload = Payload::parse(buff, len);

        if (!payload)
            return;

        if (payload->kind() == Payload::Kind::Command) {
            // auto _ = dynamic_cast<cmd::Command *>(payload);
        } else if (payload->kind() == Payload::Kind::Message) {
            // auto _ = dynamic_cast<msg::Message *>(payload);
        }

        if (payload)
            delete payload;
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
                    auto new_user = new std::shared_ptr<User>(
                        new User { username, password }
                    );
                    if (m_pool->push_user(new_user)) {
                        send_strliteral("SRV: Hello @");
                        send_strliteral(username);
                        send_strliteral("!\n");
                        authenticated = true;
                        user = std::shared_ptr<User>(*new_user);
                    } else {
                        send_strliteral("EXIT: Cannot create user.\n");
                        gracefully_terminate();
                    }
                } else {
                    send_strliteral("EXIT: Bye!\n");
                    free(username);
                    free(password);
                    gracefully_terminate();
                }
            } else {
                auto existing_user = m_pool->get_user(username);
                if (existing_user) {
                    if (!strcmp(existing_user->password, password)) {
                        send_strliteral("SRV: Hello @");
                        send(username, strlen(username));
                        send_strliteral("!\n");
                        authenticated = true;
                        user = std::shared_ptr<User>(existing_user);
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
