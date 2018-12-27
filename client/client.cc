#include "client.h"

Client::Client(string host, int port) {
    // setup variables
    host_ = host;
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    disconnect = 0;
    transport = port;
    enableread = 1;
}

Client::~Client() {
}

int Client::run() {
    // connect to the server and run echo program
    create();
    cout<<"1- 获取时间  2- 获取名字  3- 查看已连接端口  4- 转发消息 5- 断开连接 6- 退出系统" <<endl;
    int select;
    while(scanf("%d",&select)!=EOF)
    {
        switch (select) {
            case 1:
            {
                cout<<"请输入命令Time获取时间";
                echo();
                break;
            }
            case 2:
            {
                cout<<"请输入命令name获取当前服务器端机器名字";
                echo();
                break;
            }
            case 3:{
                cout<<"请输入命令who获取当前已连接端口号";
                echo();
                break;
            }
            case 4:{
                cout<<"请输入命令connect和其他客户端进行连接：";
                echo();
                break;
            }
            case 5:{
                cout<<"请输入命令disconnect断开连接";
                echo();
                break;
            }
            case 6:{
                cout<<"请输入命令quit退出系统";
                echo();
            }
            
            default:
                break;
        }
        if(select == 5 || select == 6)
            break;
    }
    //echo();
    return disconnect;
}

sockaddr_in Client::getserver(){
    struct sockaddr_in server_addr;
    // use DNS to get IP address
    struct hostent *hostEntry;
    hostEntry = gethostbyname(host_.c_str());
    if (!hostEntry) {
        cout << "No such host name: " << host_ << endl;
        exit(-1);
    }
    
    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);
    return server_addr;
}

void Client::create() {
    struct sockaddr_in server_addr;
    server_addr = getserver();
    
    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }
    // connect to server
    if (connect(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }
    cout<<"已连接!请选择以下功能..."<<endl;
    handlechat();
}

void Client::close_socket() {
    close(server_);
}

void Client::echo() {
    string line;
    // loop to handle user interface
    while (getline(cin,line)) {
        // append a newline
        line += "\n";
         //send request
        bool success = send_request(line);
        if (not success)
            //cout<<"e1"<<endl;
            break;
         //break if an error occurred
        //success = get_response();
        if (not success && !disconnect)
            //cout<<"e2"<<endl;
            break;
        if (not success && disconnect)
        {
            cout<<"连接已断开..."<<endl;
            cout<<"输入login重新登录，输入quit退出程序 ：";
            shutdown(server_, 1);
            while(getline(cin,line)){
                if(!line.compare("login"))
                    return;
                else if(!line.compare("quit"))
                    return;
                else
                    cout<<"输入非法，请重新输入："<<endl;
            }
        }
         //break if an error occurred
        
//        if(!line.compare("quit\n")){
//            return;
//        }
//
//        if (!line.compare("disconnect\n"))
//        {
//            cout<<"连接已断开..."<<endl;
//            cout<<"输入login重新登录，输入quit退出程序 ：";
//            shutdown(server_, 1);
//            while(getline(cin,line)){
//                if(!line.compare("login"))
//                    return;
//                else if(!line.compare("quit"))
//                    return;
//                else
//                    cout<<"输入非法，请重新输入："<<endl;
//            }
//        }
//
        if(!line.compare("Time\n")||!line.compare("name\n")||!line.compare("who\n")){
            return;
        }
    }
    close_socket();
}

bool Client::send_request(string request) {
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    //传入服务器端套接字
    while (nleft) {
        if ((nwritten = send(server_, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    
    //enableread = 0;
    return true;
}

bool Client::get_response() {
    // read until we get a newline
    //cout<<"进入response"<<endl;
    //response.find("\n") == string::npos
    while ((recv(server_,buf_,1024,0) >= 0)) {
//        if (nread < 0) {
//            if (errno == EINTR)
//                // the socket call was interrupted -- try again
//                continue;
//            else
//                // an error occurred, so break out
//                return "";
//        } else if (nread == 0) {
//            // the socket is closed
//            return "";
//        }
        // be sure to use append in case we have binary data
        string response = "";
        response.append(buf_);
        cout << response;
        memset(buf_, 0, 1024);
    }
    //cout<<"ds="<<response<<endl;
    // a better client would cut off anything after the newline and
    // save it in a cache
//    if(response.compare("quit\n")==0)
//        return false;
//    if(response.compare("disconnect\n")==0)
//    {
//        disconnect = 1;
//        return false;
//    }
    
    //enableread = 1;
    return true;
}

void Client::handlechat(){
    pthread_t pid;
    pthread_create(&pid, NULL, Client::instant_response, (void *)this);
    //pthread_exit(0);
}

void *Client::instant_response(void *client){
    Client *_client = (Client *)client;
    bool success = _client->get_response();
    if(not success){
        pthread_exit(0);
    }
    return NULL;
}

//bool Client:: getchat(){
//    string response = "";
//    // read until we get a newline
//    //cout<<"进入response"<<endl;
//    //struct sockaddr_in server_addr = getserver();
//    //socklen_t len = sizeof(struct sockaddr_in);
//    while (response.find("\n") == string::npos) {
//        //int s = socket(PF_INET, SOCK_DGRAM, 0);
//        int nread = ::recv(server_,buf_,1024,0);
//        if (nread < 0) {
//        if (errno == EINTR)
//                // the socket call was interrupted -- try again
//                continue;
//            else
//                // an error occurred, so break out
//                return "";
//        } else if (nread == 0) {
//            // the socket is closed
//            return "";
//        }
//        // be sure to use append in case we have binary data
//        response.append(buf_,nread);
//    }
//    //cout<<"ds="<<response<<endl;
//    // a better client would cut off anything after the newline and
//    // save it in a cache
//    cout << "收到一条及时信息："<<response;
//        return true;
//}
