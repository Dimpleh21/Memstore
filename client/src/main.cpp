#include <iostream>
#include <string>
#include "CLI.h"
using namespace std;

int main(int argc, char* argv[]){
    string host= "127.0.0.1";
    int port = 6379;
    //parsing command line arguments
    int i=1;
    while(i <argc){
        string arg= argv[i];
        if(arg=="-h" && i+1<argc){
            host= argv[++i];
        }else if(arg=="-p" && i+1<argc){
            port= stoi(argv[++i]);
        }else{
            break;
        }
        ++i;
    }

    CLI cli(host, port);
    cli.run();
}