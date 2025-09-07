#include "include\RedisServer.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
static RedisServer* globalServer= nullptr;
RedisServer::RedisServer(int port):port(port),serverSocket(-1), running(true){
    globalServer=this;
}


void RedisServer::shutdown(){
    running= false;
    if(serverSocket!=-1){
        close(serverSocket);
        // serverSocket=-1;
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

}