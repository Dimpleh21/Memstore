#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H
#include <string>
using namespace std;
class ResponseParser{
    public:
        static string parseResponse(int sockfd);
    private:
        static string parseSimpleString(int sockfd);
        static string parseSimpleError(int sockfd);
        static string parseInteger(int sockfd);
        static string parseBulkString(int sockfd);
        static string parseArray(int sockfd);
};
#endif