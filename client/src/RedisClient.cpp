#include "RedisClient.h"
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>   // for close()

using namespace std;
RedisClient::RedisClient(const string host, int port):host(host), port(port), sockfd(-1){}

RedisClient::~RedisClient(){
    disconnect();
}

bool RedisClient::connectToServer(){
    struct addrinfo hints, *res= nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family= AF_UNSPEC;
    hints.ai_socktype= SOCK_STREAM;
    string portStr= to_string(port);
    int err = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if(err!=0){
        cerr<<"getaddrinfo: "<<gai_strerror(err)<<endl;
        return false;
    }

    for(auto p= res;p!=nullptr; p=p->ai_next){
        sockfd= socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sockfd==-1) continue;
        if(connect(sockfd, p->ai_addr, p->ai_addrlen)==0){
            break;
        }
        close(sockfd);
        sockfd=-1;
    }
    freeaddrinfo(res);

    if(sockfd==-1){
        cerr<< "could not connect to" << host << ":" << port << "\n";
        return false;
    }
    return true;

}


void RedisClient::disconnect(){
    if(sockfd!=-1){
        close(sockfd);
        sockfd=-1;
    }
}

int RedisClient::getSocketFd() const {
    return sockfd;
}
bool RedisClient::sendCommand(const string &command){
    if(sockfd==-1) return false;
    ssize_t sent= send(sockfd, command.c_str(), command.size(), 0);
    return sent== (ssize_t)command.size();
}