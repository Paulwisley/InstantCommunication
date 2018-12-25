#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <string>
#include "iostream"
#include "pthread.h"
#include <vector>
using namespace std;

class Server {
public:
    Server(int port);
    ~Server();
    void run();
    
private:
    
    /*
    typedef struct {
        void *server;
        vector<int> clients;
    }args;
     */
    
    void create();
    void close_socket();
    void serve();
    void handle(vector<int>&);
    string get_request();
    bool send_response(vector<int>&, string);
    vector<int> getclients();
    void setclients(vector<int>);
    static void *messhandle(void *);
    
    string request;
    bool success;
    int port_;
    int server_;
    int buflen_;
    char* buf_;
    vector<int>m_clients;
    vector<sockaddr_in> clientaddrs;
};
