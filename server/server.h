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
#include <map>
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
    void handle();
    string get_request(int);
    bool send_response(vector<int>&, string);
    vector<int> getclients();
    vector<int> getAllclient();
    map<int,int> getclient2port();
    void setclients(vector<int>);
    static void *messhandle(void *);
    int getchat();
    void setchat(int);
    void p2pchat(int, Server *, int);
    void broadcast(vector<int>);
    
    string request;
    bool success;
    int port_;
    int server_;
    int buflen_;
    char* buf_;
    int chat;
    vector<int>m_clients;
    map<int,int>m_client2ports;//维护所有的端口号
    vector<sockaddr_in> clientaddrs;
};
