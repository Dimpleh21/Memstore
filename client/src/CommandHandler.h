#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>
using namespace std;

class RedisCommandHandler{
    public:
        static vector<string> splitArgs(const string &input);
        static string buildRespCommand(const vector<string> &args);
};

#endif