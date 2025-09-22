# MemStore - Redis-like In-Memory Data Store

MemStore is a simple, Redis-inspired in-memory key-value database server and client, implemented in C++. It supports basic Redis commands and data structures, including strings, lists, and hashes, with persistence and concurrency.

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

