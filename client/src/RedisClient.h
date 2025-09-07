#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H
#include <string>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

class RedisClient{
    public:
        RedisClient(const string host, int port);
        ~RedisClient();
        bool connectToServer();
        void disconnect();
        int getSocketFd() const;
        bool sendCommand(const string &command);
    private:
        string host;
        int port;
        int sockfd; 
        
};
#endif