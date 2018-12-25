#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "client.h"

using namespace std;

int main(int argc, char **argv)
{
    int option;

    // setup default arguments
    int port = 3000;
    string host = "localhost";

    // process command line options using getopt()
    // see "man 3 getopt"
    /*
    while ((option = getopt(argc,argv,"h:p:")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'h':
                host = optarg;
                break;
            default:
                cout << "client [-h host] [-p port]" << endl;
                exit(EXIT_FAILURE);
        }
    }
     */
    int quit = 1;
    while(quit){
        cout<<"welcome to ICS"<<endl;
        cout<<"1- 连接服务器  2- 退出客户端"<<endl;
        int select;
        cin>>select;
        cin.get();
        switch (select) {
            case 1:
            {
                cout<<"enter the connet host:"<<endl;
                cin>>host;
                cin.get();
                cout<<"enter the connet port:"<<endl;
                cin>>port;
                cin.get();
                Client client = Client(host, port);
                quit = client.run();
                break;
            }
            case 2:
            {
                exit(1);
                break;
            }
            default:
                break;
        }
    }
}

