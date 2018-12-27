#pragma once

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "pthread.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

typedef struct {
    struct sockaddr_in addr;
    int client;
    int isConn;
}clientInfo;

class Client {
public:
    Client(string host, int port);
    ~Client();

    int run();


private:
    virtual void create();
    virtual void close_socket();
    void echo();
    bool send_request(string);
    bool get_response();
    static void *instant_response(void *);
    bool getchat();
    void handlechat();
    sockaddr_in getserver();
    
    string host_;
    int port_;
    int server_;
    int buflen_;
    char* buf_;
    int disconnect;
    int transport;
    int enableread;
};
