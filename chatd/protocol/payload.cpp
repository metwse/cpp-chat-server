#include <cstdlib>
#include <cstring>

#include <chatd/protocol/protocol.hpp>


// Only ASCII chars between SPACE (32) and ~ (126) are allowed in protocol
// messages.
static inline bool is_valid_char(char c) {
    return (' ' <= c) && (c <= '~');
}


Payload *Payload::parse(char *buff, size_t len) {
    for (size_t i = 0; i < len; i++)
        if (!is_valid_char(buff[i]))
            return NULL;

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
    } else if (buff[0] == '@'){
        size_t e;
        for (e = 1; e < len; e++)
            if (buff[e] == ' ')
                break;

        if (e >= len - 2)
            return NULL;

        auto msg = new msg::DirectMessage();
        msg->buff = buff;
        msg->len = len;

        msg->msg_begin = len + 1;

        return msg;
    } else if (buff[0] == '#'){
        size_t e;
        for (e = 1; e < len; e++)
            if (buff[e] == ' ')
                break;

        if (e >= len - 2)
            return NULL;

        auto msg = new msg::GroupMessage();
        msg->buff = buff;
        msg->len = len;

        msg->msg_begin = len + 1;

        return msg;
    } else {
        auto msg = new msg::GlobalMessage();
        msg->buff = buff;
        msg->len = len;

        return msg;
    }
}

Payload::~Payload() {
    if (buff != NULL)
        free(buff);
}
