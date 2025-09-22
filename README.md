# MemStore - Redis-like In-Memory Data Store

MemStore is a simple, Redis-inspired in-memory key-value database server and client, implemented in C++. It supports basic Redis commands and data structures, including strings, lists, and hashes, with persistence and concurrency.

## Core Concepts Used

1. **TCP/IP and Socket Programming**  
   The server and client communicate over TCP sockets, handling connections and data transfer at the network level.

2. **Concurrency and Multithreading**  
   The server spawns a new thread for each client connection, allowing multiple clients to interact with the database simultaneously.

3. **Mutex and Synchronization**  
   Thread safety is ensured using mutexes to protect shared data structures from concurrent access issues.

4. **Data Structures: Hash Tables, Vectors**  
   The database uses hash tables (for key-value and hash types) and vectors (for lists) to efficiently store and retrieve data.

5. **Parsing and RESP Protocol**  
   Commands and responses are parsed and formatted using the Redis Serialization Protocol (RESP), enabling compatibility with Redis clients.

6. **File I/O Persistence**  
   Data is periodically and on-shutdown saved to disk (`dump.my_rdb`) and loaded on startup, ensuring durability.
7. **Signal Handling**  
   The server gracefully handles signals (like `SIGINT`) to trigger safe shutdown and data persistence.

8. **Command Processing and Response Formatting**  
   Commands are parsed, dispatched, and responses are formatted according to the Redis protocol.

9. **Singleton Pattern**  
   The database is implemented as a singleton to ensure a single shared instance across threads.

10. **Bitwise Operators (`|=`)**  
    Used in various places for flag manipulation and efficient state management.

11. **Standard Libraries**  
    Extensive use of C++ STL (e.g., `std::vector`, `std::unordered_map`, `std::thread`, `std::mutex`, etc.) for robust and efficient implementation.

## Features

- **Key/Value Operations:** `SET`, `GET`, `DEL`, `EXPIRE`, `RENAME`, `KEYS`, `TYPE`, `FLUSHALL`
- **List Operations:** `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LLEN`, `LREM`, `LINDEX`, `LSET`, `LGET`
- **Hash Operations:** `HSET`, `HGET`, `HEXISTS`, `HDEL`, `HGETALL`, `HKEYS`, `HVALS`, `HLEN`, `HMSET`
- **Persistence:** Periodic and on-shutdown dump/load to `dump.my_rdb`
- **Concurrency:** Multi-threaded server handles multiple clients
- **RESP Protocol:** Compatible with Redis CLI for basic commands

## Project Structure

```
client/
    Makefile
    bin/
    build/
    src/
        CLI.cpp/.h
        CommandHandler.cpp/.h
        RedisClient.cpp/.h
        ResponseParser.cpp/.h
        main.cpp
server/
    Makefile
    my_redis_server
    dump.my_rdb
    build/
    src/
        main.cpp
        RedisServer.cpp/.h
        RedisDatabase.cpp/.h
        RedisCommandHandler.cpp/.h
    useCases.md
```

## Building

### Server

```sh
cd server
make
```

### Client

```sh
cd client
make
```

## Running

### Start the Server

```sh
cd server
./my_redis_server 6379
```

### Start the Client

```sh
cd client
./bin/my_redis_cli
```

You can also use `redis-cli -p 6379` for basic commands.

## Example Usage

```
SET mykey "hello"
GET mykey
LPUSH mylist a b c
LGET mylist
HSET user:1 name Alice age 30
HGETALL user:1
EXPIRE mykey 10
```

## Supported Commands

See [server/useCases.md](server/useCases.md) for a full list of supported commands, use cases, and expected replies.

## Implementation Notes

- The server uses a singleton [`RedisDatabase`](server/src/include/RedisDatabase.h) for all data storage.
- Data is persisted to `dump.my_rdb` on shutdown and every 5 minutes.
- The client uses [`RedisClient`](client/src/RedisClient.h) to connect and send RESP-formatted commands.
- The server parses RESP protocol and responds in Redis-compatible format.

