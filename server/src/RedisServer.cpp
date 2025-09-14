#include "include/RedisServer.h"
#include "include/RedisCommandHandler.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <cstring>
#include <signal.h>
#include "include/RedisDatabase.h"
using namespace std;
static RedisServer* globalServer= nullptr;

void signalHandler(int signum){
    if(globalServer){
        cout << "Signal " << signum << " received, shutting down server..." << endl;
        globalServer->shutdown();
    }
    exit(signum);
}

void RedisServer::setupSignalHandler(){
    signal(SIGINT, signalHandler);
}
RedisServer::RedisServer(int port):port(port),server_socket(-1), running(true){
    globalServer=this;
    setupSignalHandler();
}


void RedisServer::shutdown(){
    running= false;
    if(server_socket!=-1){
        
    if(RedisDatabase::getInstance().dump("dump.my_rdb")){
        std::cout<<"Database dumped to dump.my_rdb"<<std::endl;
    }
    else{
        std::cerr<<"Error dumping database."<<std::endl;
    }
    close(server_socket);
    }
    std::cout<<"Server shutdown initiated."<<std::endl;
}

void RedisServer::run(){
    server_socket= socket(AF_INET,SOCK_STREAM,0);
    if(server_socket==-1){
        std::cerr<<"Failed to create socket."<<std::endl;
        return;
    }
    int opt=1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in serverAddr{};
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(port);
    serverAddr.sin_addr.s_addr=INADDR_ANY;
    if(bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))<0){
        std::cerr<<"Bind failed."<<std::endl;
        return;
    }
    if(listen(server_socket, 10)<0){
        std::cerr<<"Listen failed."<<std::endl;
        return;
    }
    std::cout<<"Redis-like server running on port "<<port<<std::endl;

    vector<std::thread> threads;
    RedisCommandHandler cmdHandler;

    while(running){
        int client_socket = accept(server_socket, nullptr, nullptr);
        if(client_socket<0){
            if(running){
                std::cerr<<"Error accepting client socket."<<std::endl;
            }
            break;
        }

        threads.emplace_back([client_socket, &cmdHandler](){
            char buffer[1024];
            while(true){
                size_t bytes= recv(client_socket, buffer, sizeof(buffer)-1, 0);
                if(bytes<=0) break;
                string request(buffer, bytes);
                string response= cmdHandler.processCommand(request);
                send(client_socket, response.c_str(), response.size(), 0);
            }
            close(client_socket);
        });
    }

    for(auto& thread : threads){
        if(thread.joinable()){
            thread.join();
        }
    }

    //before shutting down, persist data

}