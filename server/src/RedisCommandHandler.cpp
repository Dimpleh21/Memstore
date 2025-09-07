#include <include/RedisCommandHandler.h>
#include <sstream>
#include <bits/stdc++.h>
using namespace std;

//RESP parser:
// 2*\r\n$4\r\nLLEN\r\n$3\r\nkey\r\n
// 2*--> number of arguments
// $4--> length of first argument
// LLEN--> first argument
// $3--> length of second argument


vector<string> parseRespCommand(const string &input){
    vector<string> tokens;
    if(input.empty()) return tokens;
    if(input[0]!='*'){
       istringstream iss(input);
       string token;
       while(iss>>token){
              tokens.push_back(token);
       }
         return tokens;
    }
    size_t pos=0;
    if(input[pos]!='*') return tokens;
    pos++;
    //crlf= carriage return line feed
    size_t crlf=input.find("\r\n", pos);
    if(crlf==string::npos) return tokens;

    int numElements = stoi(input.substr(pos, crlf-pos));
    pos= crlf+2;
    for(int i=0;i<numElements;i++){
        if(pos>=input.size() || input[pos]!='$') break;
        pos++;
        crlf= input.find("\r\n", pos);
        if(crlf==string::npos) break;
        int len= stoi(input.substr(pos, crlf-pos));
        pos= crlf+2;
        if(pos+len>input.size()) break;
        string token= input.substr(pos, len);
        tokens.push_back(token);    
        pos+= len+2; //move past token and crlf
    }

    return tokens;
}


RedisCommandHandler::RedisCommandHandler(){}

string RedisCommandHandler::processCommand(const string& commandLine){
    auto tokens= parseRespCommand(commandLine);
    if(tokens.empty()) return "-Error: Empty command\r\n";
    string cmd= tokens[0];
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    ostringstream response;

    return response.str();
}