**🎮 Rock Paper Scissors (Live Socket Program)**
---
A client-server Rock Paper Scissors game built in C++ using TCP sockets. The application demonstrates network programming concepts including socket communication, concurrent client handling, custom application-layer protocols, and real-time game state management.

***

**Features**
---
- TCP client-server architecture
- Real-time Rock Paper Scissors gameplay
- Custom command protocol
- Concurrent client support using fork()
- Score tracking throughout the session
- Command validation with status responses
- Graceful connection handling

***

**Technologies**
---
- C++
- Berkeley Sockets API
- TCP/IP Networking
- Linux System Programming
- Process Management (fork)
- Custom Client-Server Protocol

***

**Architecture**
---
                           TCP Connection
+----------------+     <--------------------->     +----------------+
|     Client     |                                |     Server      |
|----------------|                                |-----------------|
| Send Commands  |                                | Listen on Port  |
| Receive Moves  |                                | Accept Clients  |
| Display Score  |                                | Process Commands|
+----------------+                                | Track Scores    |
                                                  +-----------------+

***

**Supported Commands**
---
| Command  | Description                             |
| -------- | --------------------------------------- |
| `HELLO`  | Initiates communication with the server |
| `CMDS`   | Displays all available commands         |
| `PLAY`   | Starts a Rock Paper Scissors game       |
| `MOVE R` | Play Rock                               |
| `MOVE P` | Play Paper                              |
| `MOVE S` | Play Scissors                           |
| `SCORE`  | Displays the current score              |
| `CLOSE`  | Terminates the connection               |

***

**How It Works**
---
1. The server creates a TCP socket and listens for incoming client connections.
2. Each new client connection is handled in a separate child process using fork().
3. Clients communicate with the server through a custom text-based protocol.
4. During gameplay:
  - The client sends a move (MOVE R, MOVE P, or MOVE S).
  - The server randomly generates its own move.
  - The winner is determined.
  - Scores are updated and returned to the client.
5. The session continues until the client closes the connection.

***

**Future Improvements**
--
- Replace fork() with multithreading or asynchronous I/O.
- Support multiple players in the same game session.
- Add player usernames and matchmaking.
- Encrypt communication using TLS.
- Implement persistent leaderboards.
- Create a graphical client interface.

