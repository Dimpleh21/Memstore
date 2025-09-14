#include "include/RedisServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "include/RedisDatabase.h"
int main(int argc, char* argv[]){
    int port = 6379;
    if(argc>=2){
        port= std::stoi(argv[1]);
    }
    if(RedisDatabase::getInstance().load("dump.my_rdb")){
        std::cout<< "database loaded from dump.my_rdb successfully." << std::endl;
    }else{
        std::cout << "No dump found." << std::endl;
    }
    RedisServer server(port);
    std::thread persistenceThread([](){
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(300));
            if(!RedisDatabase::getInstance().dump("dump.my_rdb")){
                std::cerr<<"Error dumping database."<<std::endl;
            }else{
                std::cout<<"Database dumped to dump.my_rdb"<<std::endl;
            }
        }
    });
    persistenceThread.detach();
    server.run();
    return 0;
}