#include "CLI.h"

#include <vector>
using namespace std;
CLI::CLI(const string host, int port):redisClient(host, port), host(host), port(port) {};

static string trim(const string &s){
    size_t start= s.find_first_not_of("\t\n\r\v\f ");
    if(start== string::npos) return "";
    size_t end= s.find_last_not_of("\t\n\r\v\f ");
    return s.substr(start, end-start+1);
}
void CLI::run() {
    if (!redisClient.connectToServer()) {
        return;
    }

    std::cout << "Connected to Redis at" << redisClient.getSocketFd() << std::endl;

    while(true){
        cout << host << ":" << port << "> ";
        cout.flush();
        string line;
        if(!getline(cin, line)){
            break;
        }
        line= trim(line);
        if(line.empty()) continue;  
        if(line=="quit" || line=="exit"){
            cout << "GoodBye.\n";
            break;
        }
        if(line=="help"){
            cout << "Displaying help" << endl;
            continue; 
        }

        vector<string> args= RedisCommandHandler::splitArgs(line);
        if(args.empty()) continue;

        // for(const auto &arg:args){
        //     cout << arg << endl;
        // }
        string command= RedisCommandHandler::buildRespCommand(args);
        if(!redisClient.sendCommand(command)){
            cerr << "Failed to send command to server.\n";
            break;
        }

        string response= ResponseParser::parseResponse(redisClient.getSocketFd());
        cout << response << endl;
        

    }
    redisClient.disconnect();


}