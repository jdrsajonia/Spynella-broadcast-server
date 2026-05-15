# Spynella Broadcast Server

A console-based chat application using the broadcast server model to handle multiple concurrent clients. Built in C++ with socket programming and multi-threading support.

## 📋 Overview

Spynella Broadcast Server is a lightweight, command-line chat application that implements a server-client architecture. The server broadcasts messages from clients to all other connected clients in real-time, creating a shared chat experience. Each client maintains an interactive terminal interface with support for raw input mode.

## ✨ Features

- **Broadcast Messaging**: Messages from any client are automatically broadcasted to all other connected clients
- **Multi-client Support**: Server can handle multiple simultaneous connections using threads
- **Interactive CLI**: Raw terminal mode for responsive client input with real-time feedback
- **Command System**: Support for server-side commands (prefix with `/`)
- **Color-coded Output**: Each client is assigned a unique color for visual distinction
- **Timestamps**: Messages include timestamps for tracking communication flow
- **Automatic Client Detection**: Server tracks and announces client joins and exits
- **Connection Management**: Graceful handling of client disconnections and errors

## 🔧 Requirements

- **C++ Compiler**: GCC or Clang with C++11 or later support
- **Operating System**: Linux/Unix-based systems (uses POSIX APIs like `termios`)
- **Standard Libraries**: Standard C++ library (thread, mutex, iostream, etc.)

## 📦 Project Structure

```
├── server_chat.cpp     # Server implementation with broadcast logic
├── client_chat.cpp     # Client implementation with interactive terminal
└── README.md          # This file
```

## 🚀 Compilation

To compile both the server and client, use a C++ compiler:

```bash
# Compile the server
g++ -std=c++11 -pthread server_chat.cpp -o server

# Compile the client
g++ -std=c++11 -pthread client_chat.cpp -o client
```

Or compile both together:

```bash
g++ -std=c++11 -pthread server_chat.cpp client_chat.cpp -o spynella
```

## 📖 Usage

### Starting the Server

```bash
./server
```

The server will start listening on **port 1234** and display a welcome banner. It will accept incoming client connections and manage all communication.

### Connecting a Client

In a separate terminal, connect to the server:

```bash
# Connect to localhost
./client

# Connect to a remote server by IP address
./client 192.168.1.100
```

Default connection details:
- **Default IP**: `127.0.0.1` (localhost)
- **Port**: `1234`

### Client Interaction

Once connected, you can:

1. **Send Messages**: Type any text and press **Enter** to send to all other clients
2. **Use Backspace**: Press **Backspace** or **Delete** to edit your input
3. **Execute Commands**: Start a message with `/` to execute server commands
4. **Exit**: Press **Ctrl+C** to disconnect gracefully

Example session:
```
[localhost:1234]> Hello everyone!
[localhost:1234]> /status
[localhost:1234]> Goodbye!
```

## 🏗️ Architecture

### Server (`server_chat.cpp`)

- **Listening Socket**: Accepts incoming client connections on port 1234
- **Thread-per-Client**: Each connected client is handled by a dedicated thread
- **Broadcast Logic**: Routes messages to all clients except the sender
- **Client Tracking**: Maintains a list of active clients with their metadata (IP, nickname, color)
- **Command Parsing**: Interprets and executes `/` prefixed commands

### Client (`client_chat.cpp`)

- **Connection Thread**: Maintains connection to server and receives messages
- **Raw Input Mode**: Disables canonical input processing for responsive typing
- **Message Buffering**: Collects typed characters before sending on Enter
- **Terminal Control**: Uses ANSI escape codes for cursor positioning and clearing
- **Signal Handling**: Gracefully handles interruption signals (Ctrl+C)

## 🔌 Communication Protocol

### Message Flow

1. **Client → Server**: Text message or command
2. **Server Processing**:
   - If message starts with `/`: Execute as command
   - Otherwise: Broadcast to all other clients with timestamp and sender info
3. **Server → All Clients**: Formatted message with timestamp and sender nickname

### Message Format

```
[HH:MM:SS][Client_Nickname]: Message content
```

## ⚙️ Technical Details

### Synchronization

- **Mutex Locks**: Thread-safe access to the client list and console output
- **Lock Guard**: RAII-based locking for exception safety

### Terminal Control

- **Raw Mode**: Disables line buffering and echo for immediate character input
- **ANSI Codes**: Used for cursor control and clearing lines
- **Termios API**: POSIX terminal I/O control

### Error Handling

- Socket creation and binding validation
- Connection state verification
- Graceful client removal on disconnect
- Distinction between normal and error-based disconnections

## 🛠️ Building and Running a Complete Example

```bash
# Terminal 1 - Start the server
./server

# Terminal 2 - First client
./client

# Terminal 3 - Second client
./client

# Now type messages in either client and see them broadcast to the other
```

## 🐛 Troubleshooting

| Issue | Solution |
|-------|----------|
| "Port already in use" | Wait a few seconds for the port to be released, or change the port number in the code |
| "Connection refused" | Ensure the server is running and listening on port 1234 |
| "Raw mode not working" | Verify you're running on a Unix/Linux system with terminal support |
| Input appears corrupted | The terminal may not support ANSI codes; ensure you're using a modern terminal |

## 📝 Future Enhancements

- User authentication system
- Persistent message logging
- Private/direct messages
- Channel-based chat rooms
- Username customization
- Configuration file support
- WebSocket bridge for web clients

## 📜 License

This project is provided as-is for educational and commercial use.

## 👤 Author

Created by [jdrsajonia](https://github.com/jdrsajonia)

---

**Happy chatting! 💬**
