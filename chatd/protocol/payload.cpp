#include <cstdlib>
#include <cstring>

#include <chatd/protocol/protocol.hpp>


// Only ASCII chars between SPACE (32) and ~ (126) are allowed in protocol
// messages.
static inline bool is_valid_char(char c) {
    return (' ' <= c) && (c <= '~');
}


Payload *Payload::parse(char *buff, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (buff[i] == '\r' && i == len - 1) {
            buff[i] = '\0';
            len--;
        } else if (!is_valid_char(buff[i])) {
            free(buff);
            return NULL;
        }
    }

    if (buff[0] == '/') {
        cmd::Command *cmd = NULL;
        size_t last_token_end = 0;
        for (size_t i = 1; i < len; i++) {
            if (buff[i] == ' ' || i == len - 1) {
                if (buff[i] == ' ')
                    buff[i] = '\0';

                if (cmd == NULL) {
                    if (!strcmp(&buff[1], "subscribe"))
                        cmd = new cmd::Subscribe();
                    else if (!strcmp(&buff[1], "unsubscribe"))
                        cmd = new cmd::Unsubscribe();
                    else if (!strcmp(&buff[1], "delete"))
                        cmd = new cmd::Delete();
                    else if (!strcmp(&buff[1], "listusers") ||
                            !strcmp(&buff[1], "list_users"))
                        cmd = new cmd::ListUsers();
                    else if (!strcmp(&buff[1], "logout"))
                        cmd = new cmd::Logout();
                    else {
                        free(buff);
                        return NULL;
                    }
                } else {
                    if (last_token_end + 1 != i)
                        cmd->args.push(&buff[last_token_end + 1]);
                }

                last_token_end = i;
            }
        }

        cmd->buff = buff;
        cmd->len = len;

        size_t argcount = cmd->args.get_size();

        bool is_args_valid = (
            (typeid(*cmd) == typeid(cmd::Subscribe) && argcount == 2) ||
            (typeid(*cmd) == typeid(cmd::Unsubscribe) && argcount == 1) ||
            (typeid(*cmd) == typeid(cmd::ListUsers) && argcount == 1) ||
            (typeid(*cmd) == typeid(cmd::Delete) && argcount == 1) ||
            (typeid(*cmd) == typeid(cmd::Logout) && argcount == 0)
        );

        if (is_args_valid) {
            return cmd;
        } else {
            delete cmd;
            return NULL;
        }
    } else if (buff[0] == '@' || buff[0] == '#'){
        if (len < 3) {
            free(buff);
            return NULL;
        }

        size_t i;

        for (i = 1; i < len; i++) {
            if (buff[i] == ' ') {
                if (i == 1) {
                    free(buff);
                    return NULL;
                }

                buff[i] = '\0';
                break;
            }
            if (i >= len - 2) {
                free(buff);
                return NULL;
            }
        }

        if (buff[0] == '@') {
            auto msg = new msg::DirectMessage();
            msg->to = buff + 1;
            msg->content = &buff[i + 1];

            msg->buff = buff;
            msg->len = len;

            return msg;
        } else {
            auto msg = new msg::GroupMessage();
            msg->to = buff + 1;
            msg->content = &buff[i + 1];

            msg->buff = buff;
            msg->len = len;

            return msg;
        }
    } else {
        auto msg = new msg::GlobalMessage();

        msg->content = buff;

        msg->buff = buff;
        msg->len = len;

        if (msg->len == 0) {
            delete msg;
            return NULL;
        }

        return msg;
    }
}

Payload::~Payload() {
    if (buff != NULL)
        free(buff);
}
