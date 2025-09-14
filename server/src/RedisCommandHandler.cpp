#include "include/RedisCommandHandler.h"
#include "include/RedisDatabase.h"
#include <sstream>
#include <bits/stdc++.h>
#include <iostream>
#include <algorithm>


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



static std::string handleLlen(const std::vector<std::string>& tokens, RedisDatabase& db){

    if(tokens.size()<2){
       return "-ERROR rename requires key";
    }
    ssize_t len= db.llen(tokens[1]);
    return ":"+ std::to_string(len)+"\r\n";
}


static std::string handleLPush(const std::vector<std::string>& tokens, RedisDatabase& db){
    if(tokens.size()< 3){
        return "-ERROR LPUSH requires a key and a value";
    }
    db.lpush(tokens[1], tokens[2]);
    ssize_t len= db.llen(tokens[1]);
    return ":" + std::to_string(len)+"\r\n";
}


static std::string handleRPush(const std::vector<std::string>& tokens, RedisDatabase& db){
    if(tokens.size()< 3){
        return "-ERROR RPUSH requires a key and a value";
    }
    db.rpush(tokens[1], tokens[2]);
    ssize_t len= db.llen(tokens[1]);
    return ":" + std::to_string(len)+"\r\n";
}
static std::string handleLPop(const std::vector<std::string>& tokens, RedisDatabase& db){
    if(tokens.size()<2){
        return "-ERROR LPOP requires a key";
    }
    std::string val;
    if(db.lpop(tokens[1], val)){
        return "$" + std::to_string(val.size())+"\r\n"+ val+"\r\n";
    }
    return "-1\r\n";
}
static std::string handleRPop(const std::vector<std::string>& tokens, RedisDatabase& db){
    if(tokens.size()<2){
        return "-ERROR RPOP requires a key";
    }
    std::string val;
    if(db.rpop(tokens[1], val)){
        return "$" + std::to_string(val.size())+"\r\n"+ val+"\r\n";
    }
    return "-1\r\n";
}

static std::string handleLRem(const std::vector<std::string>& tokens, RedisDatabase& db){
    if(tokens.size()<4){
        return "-ERROR LREM requires key and count ";
    }
    try{
        int count= std::stoi(tokens[2]);
        int removed= db.lrem(tokens[1], count, tokens[3]);
        return ":"+std::to_string(removed)+"\r\n";
    }catch(const std::exception&){
        return "-ERROR invalid count\r\n";
    }
}


static std::string handleLIndex(const std::vector<std::string>& tokens, RedisDatabase& db){
     if(tokens.size()<4){
        return "-ERROR LINDEX requires key and index. ";
    }
    try{
        int index= std::stoi(tokens[2]);
        string value;
        if(db.lindex(tokens[1], index, value)){
            return "$"+std::to_string(value.size())+"\r\n"+value +"\r\n";
        }else{
            return "$-1\r\n";
        }
    }catch(const std::exception&){
        return "-ERROR invalid index\r\n";
    }
}

static std::string handleLSet(const std::vector<std::string>& tokens, RedisDatabase& db){
    if(tokens.size()<4){
        return "-ERROR LSET requires key, index and value.\r\n";
    }
    try{
        int index= std::stoi(tokens[2]);
        if(db.lset(tokens[1], index, tokens[3])){
            return "+OK\r\n";
        }else{
            return "-ERROR index out of range\r\n";
        }
    }catch(const std::exception&){
        return "-ERROR invalid index\r\n";
    }
}


static std::string handleHset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4) 
        return "-Error: HSET requires key, field and value\r\n";
    db.hset(tokens[1], tokens[2], tokens[3]);
    return ":1\r\n";
}



static std::string handleHget(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3) 
        return "-Error: HSET requires key and field\r\n";
    std::string value;
    if (db.hget(tokens[1], tokens[2], value))
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    return "$-1\r\n";
}


static std::string handleHexists(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3) 
        return "-Error: HEXISTS requires key and field\r\n";
    bool exists = db.hexists(tokens[1], tokens[2]);
    return ":" + std::to_string(exists ? 1 : 0) + "\r\n";
}


static std::string handleHdel(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3) 
        return "-Error: HDEL requires key and field\r\n";
    bool res = db.hdel(tokens[1], tokens[2]);
    return ":" + std::to_string(res ? 1 : 0) + "\r\n";
}

static std::string handleHgetall (const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HGETALL requires key\r\n";
    auto hash = db.hgetall(tokens[1]);
    std::ostringstream oss;
    oss << "*" << hash.size() * 2 << "\r\n";
    for (const auto& pair: hash) {
        oss << "$" << pair.first.size() << "\r\n" << pair.first << "\r\n";
        oss << "$" << pair.second.size() << "\r\n" << pair.second << "\r\n";
    }
    return oss.str();
}


static std::string handleHkeys(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HKEYS requires key\r\n";
    auto keys = db.hkeys(tokens[1]);
    std::ostringstream oss;
    oss << "*" << keys.size() << "\r\n";
    for (const auto& key: keys) {
        oss << "$" << key.size() << "\r\n" << key << "\r\n";
    }
    return oss.str();
}


static std::string handleHvals(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HVALS requires key\r\n";
    auto values = db.hvals(tokens[1]);
    std::ostringstream oss;
    oss << "*" << values.size() << "\r\n";
    for (const auto& val: values) {
        oss << "$" << val.size() << "\r\n" << val << "\r\n";
    }
    return oss.str();
}


static std::string handleHlen(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HLEN requires key\r\n";
    ssize_t len = db.hlen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}


static std::string handleHmset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4 || (tokens.size() % 2) == 1) 
        return "-Error: HMSET requires key followed by field value pairs\r\n";
    std::vector<std::pair<std::string, std::string>> fieldValues;
    for (size_t i = 2; i < tokens.size(); i += 2) {
        fieldValues.emplace_back(tokens[i], tokens[i+1]);
    }
    db.hmset(tokens[1], fieldValues);
    return "+OK\r\n";
}

string RedisCommandHandler::processCommand(const string& commandLine){
    auto tokens= parseRespCommand(commandLine);
    if(tokens.empty()) return "-Error: Empty command\r\n";
    //  for(auto &t: tokens){
    //     cout<<"Token: "<<t<<endl
    // }
    string cmd= tokens[0];
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    ostringstream response;
    RedisDatabase& db= RedisDatabase::getInstance();

    if(cmd=="PING"){
        response<<"+PONG\r\n";
    }else if(cmd=="ECHO"){
        if(tokens.size()<2){
            response << "-ERROR less arguments for ECHO\r\n";
        }else{
            response << "+" << tokens[1] << "\r\n";
        }
    }else if(cmd=="FLUSHALL"){
        db.flushAll();
        response<<"+OK\r\n";
    
    //key value operations

    }else if(cmd=="SET"){
        if(tokens.size()<3){
            response << "-ERROR less arguments for SET\r\n";    
        }else{
            db.set(tokens[1], tokens[2]);
            response<<"+OK\r\n";
        }
    }else if(cmd=="GET"){
        if(tokens.size()<2){
            response << "-ERROR less arguments for GET\r\n";
        }else{
            std::string value;
            if(db.get(tokens[1], value)){
                response<<"$"<<value.size()<<"\r\n"<<value<<"\r\n";
            }else{
                response << "$-1\r\n"; //nil response
            }
        }
    }else if(cmd=="KEYS"){
        std::vector<std::string> allKeys= db.keys();
        response<<"*"<<allKeys.size()<<"\r\n";
        for(auto &key: allKeys){
            response<<"$"<<key.size()<<"\r\n"<<key<<"\r\n";
        }
    }else if(cmd=="TYPE"){
        if(tokens.size()<2){
            response << "-ERROR less arguments for TYPE\r\n";
        }else{
            std::string type= db.type(tokens[1]);
            response<<"+"<<type<<"\r\n";
        }
    }else if(cmd=="DEL" || cmd=="UNLINK"){
        if(tokens.size()<2){
            response << "-ERROR less arguments for" <<  cmd << "\r\n";
        }else{
            bool res= db.del(tokens[1]);
            response<<":"<<(res? "1":"0")<<"\r\n";
        }
    }else if(cmd=="EXPIRE"){
        if(tokens.size()<3){
            response << "-ERROR less arguments for EXPIRE\r\n";
        }else{
            if(db.expire(tokens[1], tokens[2])){
                response<<"+OK\r\n";
            }
           
        }
    }else if(cmd=="RENAME"){
        if(tokens.size()<3){
            response << "-ERROR less arguments for RENAME\r\n";
        }else{
            if(db.rename(tokens[1], tokens[2])){
                response<<"+OK\r\n";
            }
        }
    }
   
    //list operations
    else if(cmd=="LLEN"){
        return handleLlen(tokens, db);
    }else if(cmd=="LPUSH"){
        return handleLPush(tokens, db);
    }else if(cmd=="RPUSH"){
        return handleRPush(tokens,db);
    }else if(cmd=="LPOP"){
        return handleLPop(tokens,db);
    }else if(cmd=="RPOP"){
        return handleRPop(tokens,db);
    }else if(cmd=="LREM"){
        return handleLRem(tokens, db);
    }else if(cmd=="LINDEX"){
        return handleLIndex(tokens,db);
    }else if(cmd=="LSET"){
        return handleLSet(tokens,db);
    }else if(cmd=="HSET"){
        return handleHset(tokens, db);
    }else if(cmd=="HGET"){
        return handleHget(tokens, db);
    }else if(cmd=="HEXISTS"){
        return handleHexists(tokens, db);
    }else if(cmd=="HDEL"){
        return handleHdel(tokens, db);
    }else if(cmd=="HGETALL"){
        return handleHgetall(tokens, db);
    }else if(cmd=="HKEYS"){
        return handleHkeys(tokens, db);
    }else if(cmd=="HVALS"){
        return handleHvals(tokens, db);
    }else if(cmd=="HLEN"){
        return handleHlen(tokens, db);
    }else if(cmd=="HMSET"){
        return handleHmset(tokens, db);
    }
     //hash operations
    else{
        response<<"-Error: Unknown command\r\n";
    }

    return response.str();
}