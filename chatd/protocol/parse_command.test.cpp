#include <cassert>
#include <cstring>
#include <cstdlib>

#include <chatd/protocol/protocol.hpp>


Payload *parse_nullterminated(char *cstring) {
    return Payload::parse(cstring, strlen(cstring));
}

char *alloc_cstring(const char *str_literal) {
    char *str = (char *) malloc(sizeof(char *) * (strlen(str_literal) + 1));
    memcpy(str, str_literal, strlen(str_literal) + 1);

    return str;
}

cmd::Command *parse_command(const char *str_literal) {
    Payload *payload = parse_nullterminated(alloc_cstring(str_literal));

    if (!payload)
        return NULL;

    assert(payload->kind() == Payload::Kind::Command);

    return dynamic_cast<cmd::Command *>(payload);
}



int main() {
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
