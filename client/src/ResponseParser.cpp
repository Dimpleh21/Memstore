#include "ResponseParser.h"
#include <iostream>
#include <sstream>
#include <unistd.h> // for read()
#include <sys/types.h>
#include <sys/socket.h>

static bool readChar(int sockfd, char &c){
    ssize_t n= read(sockfd, &c, 1);
    return n==1;
}

static string readLine(int sockfd){
    string line;
    char c;
    while(readChar(sockfd, c)){
        if(c=='\r'){
            if(readChar(sockfd, c) && c=='\n'){
                break;
            }
        }
        line.push_back(c);
    }
    return line;
}

string ResponseParser::parseResponse(int sockfd){
    char prefix;
    if(!readChar(sockfd, prefix)){
        return "-Error: Failed to read response from server\r\n";
    }
    switch(prefix){
        case '+':
            return parseSimpleString(sockfd);
        case '-': 
            return parseSimpleError(sockfd);
        case ':':
            return parseInteger(sockfd);
        case '$':
            return parseBulkString(sockfd);
        case '*':
            return parseArray(sockfd);

        default:
            return "(Error) Unsupported response type";
    }
}

string ResponseParser::parseSimpleString(int sockfd){
    return readLine(sockfd);
}

string ResponseParser::parseSimpleError(int sockfd){
    return "(Error) " + readLine(sockfd);
}
string ResponseParser::parseInteger(int sockfd){
    return readLine(sockfd);
}

string ResponseParser::parseBulkString(int sockfd){
    string lenstr= readLine(sockfd);
    int length = stoi(lenstr);
    if(length==-1){
        return "(nil)";
    }
    string bulk;
    bulk.resize(length);
    int totalRead=0;
    while(totalRead<length){
        ssize_t n= recv(sockfd, &bulk[totalRead], length-totalRead, 0);
        if(n<=0){
            return "-Error: Failed to read bulk string";
        }
        totalRead+= n;

    }
    char dummy;
    readChar(sockfd, dummy); //read \r
    readChar(sockfd, dummy); //read \n
    return bulk;
}


string ResponseParser::parseArray(int sockfd){
    string countstr= readLine(sockfd);
    int count= stoi(countstr);
    if(count==-1){
        return "(nil)";
    }
    ostringstream oss;
    for(int i=0;i<count;i++){
        oss << parseResponse(sockfd);
        if(i!=count-1) oss << "\n";
    }
    return oss.str();
}
