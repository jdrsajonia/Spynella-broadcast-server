# Spynella Broadcast Server

A console-based chat application using the broadcast server model to handle multiple concurrent clients. Built in C++ with socket programming and multi-threading support.

## Features

- **Broadcast Messaging**: Messages from any client are automatically broadcasted to all other connected clients
- **Multi-client Support**: Server can handle multiple simultaneous connections using threads
- **Interactive CLI**: Real-time feedback with command support
- **Color-coded Output**: Each client is assigned a unique color
- **Timestamps**: Messages include timestamps for tracking communication flow
- **Connection Management**: Graceful handling of client disconnections

## Requirements

- **C++ Compiler**: GCC or Clang with C++11 or later
- **OS**: Linux/Unix-based systems (uses POSIX APIs)

## Compilation

```bash
# Compile the server
g++ -std=c++11 -pthread server_chat.cpp -o server

# Compile the client
g++ -std=c++11 -pthread client_chat.cpp -o client
```

## Usage

### Start the Server
```bash
./server
```

### Connect a Client
```bash
./client                    # localhost
./client 192.168.1.100     # remote server
```

Default: **localhost:1234**

### Client Commands
- Type text and press **Enter** to send
- Press **Backspace** to edit
- Start with `/` for server commands
- Press **Ctrl+C** to disconnect

## Architecture

- **Server**: Listening socket, thread-per-client, broadcast logic, command parsing
- **Client**: Connection thread, raw input mode, ANSI terminal control

## License

Educational and commercial use.

**Created by [jdrsajonia](https://github.com/jdrsajonia)**
