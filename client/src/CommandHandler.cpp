#include "CommandHandler.h"
#include <regex>
#include <sstream>
#include <algorithm>
vector<string> RedisCommandHandler::splitArgs(const string &input){
    vector<string> tokens;
    regex rgx(R"((\"[^\"]+\"|\S+))");
    auto words_begin= sregex_iterator(input.begin(), input.end(), rgx);
    auto words_end= sregex_iterator();
    for(auto it= words_begin; it!=words_end; ++it){
        string token= it->str();
        if(token.size()>=2 && token.front()=='\"' && token.back()=='\"'){
            token= token.substr(1, token.size()-2);
        }
        tokens.push_back(token);
    }
    return tokens;

}

string RedisCommandHandler::buildRespCommand(const vector<string> &args){
    ostringstream oss;
    oss << "*" <<args.size() << "\r\n";
    // if(args.empty()) continue;
    for(const auto &arg: args){
        
        oss << "$" << arg.size() << "\r\n" << arg << "\r\n";
    }
    return oss.str();
}