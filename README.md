# cpp-chat-server
A simple TCP-based chat server implemented in C++.

## Building
### Release Build
```sh
make build
```
This compiles the project with optimization flags into `target/chatd`.

### Debug Build
```sh
make build_debug
```
This compiles with debug symbols (`-g3`) and assertions enabled into
`target/chatd.debug`.

## Running
| Command | Description |
|--|--|
| `make run` | Runs the optimized binary. |
| `make debug` | Runs the debug binary inside `gdb`. |
| `make memleak` | Runs the project with `valgrind`. |

## Tests
Unit tests are automatically discovered if files end with `.test.c` or
`.test.cpp.`.

For example, if you have a file like `chatd/collections/vec.test.c`, you can:
- Build it with: `make build_test_colections/vec`
- Run it under `gdb` with: `make test_collections/vec`

> Note: Test binaries are placed under target/tests.


## Dependencies
- GNU Make (v4 or later)
- `g++`, `gcc`
- `bear` (optional, for `compile_commands`)
- `valgrind` (optional, for `memleak`)
- `gdb` (optional, for debugging)

## Contributing
Contributions are welcome! Please follow conventional C/C++ style and document
any new functionality with clear comments or test files.

Please also check our [Code of Conduct](https://github.com/metwse/code-of-conduct)
before contributing.

### Generating Compilation Database
To generate a `compile_commands.json` file for use in IDEs or language servers:
```sh
make compile_commands
```
