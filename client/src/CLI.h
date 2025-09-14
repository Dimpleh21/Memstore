#ifndef CLI_H
#define CLI_H
#include <string>
#include <vector>
#include "CommandHandler.h"
#include "RedisClient.h"
#include "ResponseParser.h"
using namespace std;
class CLI{
    public:
        CLI(const string host, int port);
        void run(const vector<string>& commandArgs);
        void executeCommand(const vector<string>& args);

    private:
        RedisClient redisClient;
        string host;
        int port;
        

};

#endif