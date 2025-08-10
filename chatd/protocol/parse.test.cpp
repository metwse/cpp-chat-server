#include <cassert>
#include <cstring>
#include <cstdlib>

#include <chatd/protocol/protocol.hpp>


Payload *parse_nullterminated(const char *str_literal) {
    char *str = (char *) malloc(sizeof(char *) * (strlen(str_literal) + 1));
    memcpy(str, str_literal, strlen(str_literal) + 1);

    return Payload::parse(str, strlen(str_literal));
}

cmd::Command *parse_command(const char *str_literal) {
    Payload *payload = parse_nullterminated(str_literal);
    if (!payload)
        return NULL;

    assert(payload->kind() == Payload::Kind::Command);

    return dynamic_cast<cmd::Command *>(payload);
}

msg::Message *parse_message(const char *str_literal) {
    Payload *payload = parse_nullterminated(str_literal);
    if (!payload)
        return NULL;

    assert(payload->kind() == Payload::Kind::Message);

    return dynamic_cast<msg::Message *>(payload);
}


void test_message();
void test_command();

int main() {
    test_message();
    test_command();
}

void test_message() {
    const char *valid_msgs[] = {
        "global message",
        "#channel message",
        "#channel    message   ",
        "@direct direct",
        "@direct    direct   ",
        "@a e",
        "#a e",
        "a",
        " ",
        "@a  ",
        "#e  ",
    };

    const char *invalid_msgs[] = {
        "",
        "#",
        "@",
        "#nocontent",
        "@nocontent",
        "# nochannel",
        "@ nouser",
        "@ a",
        "# e",
    };

    for (const char *payload : valid_msgs) {
        auto msg = parse_message(payload);
        assert(msg);

        delete msg;
    }

    for (const char *payload : invalid_msgs) {
        auto msg = parse_message(payload);
        assert(!msg);
    }
}


void test_command() {
    const char *valid_cmds[] = {
        "/subscribe arg1 arg2",
        "/subscribe    arg1   arg2 ",
        "/unsubscribe arg1",
        "/unsubscribe arg1   ",
        "/list_users arg1",
        "/list_users    arg1",
        "/listusers    arg1",
        "/delete arg1",
        "/delete    arg1   ",
        "/logout",
        "/logout ",
    };

    const char *invalid_cmds[] = {
        "/subscribe channel passwd extraarg",
        "/subscribe    insufficient ",
        "/unsubscribe ",
        "/ unsubscribe",
        "/list_users",
        "/listusers",
        "/ list_users ",
        "/delete ",
        "/delete channel extraarg",
        "/logout me",
    };

    for (const char *payload : valid_cmds) {
        auto cmd = parse_command(payload);
        assert(cmd);

        size_t argcount = cmd->args.get_size();

        if (argcount >= 1)
            assert(!strcmp((char *) cmd->args[0], "arg1"));
        if (argcount >= 2)
            assert(!strcmp((char *) cmd->args[1], "arg2"));

        delete cmd;
    }

    for (const char *payload : invalid_cmds) {
        auto cmd = parse_command(payload);
        assert(!cmd);
    }
}
