#include "server.h"

Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    request = "";
    success = true;
    chat = 0;
}

Server::~Server() {
    delete buf_;
}

void Server::run() {
    // create and run the server
    create();
    serve();
}

void Server::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (::bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void Server::close_socket() {
    close(server_);
}

void Server::serve() {
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    vector<int>clients;
    while((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0)
    {
        clients.push_back(client);
        setclients(clients);
        //cout<<"size = "<<clients.size()<<"  cur = "<<client<<endl;
        handle();
        //一旦接收到了就开辟一个线程将处理程序放入线程内
    }
    close_socket();
}

void Server::handle() {
    //pthread_t pid;
    // loop to handle all requests
    //pthread_create(&pid, NULL, messhandle,(void *)&client);
    pthread_t pid;
    /*
    while(1){
        // get a request
        //int _client = *((int*)client);
     
        string request = Server::get_request(client);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // send response
        bool success = Server::send_response(client,request);
        // break if an error occurred
        if (not success)
            break;
    }
     */
    //args Arg;
    //memset(&Arg, 0, sizeof(Arg));
    //Arg.client = client;
    //Arg.clients = clients;
    //Arg.server = this;
    pthread_create(&pid, NULL, Server::messhandle, (void *)this);
}

void * Server::messhandle(void *server){
    //args *Arg = (args *)arg;
    //int client = Arg->clients;
    vector<int>clients;
    vector<int> allclients;
    Server *_server = (Server *)server;
    clients = _server->getclients();//返回的是当前连接的客户端端口套接字
    allclients = _server->getAllclient();//返回全部已经连接的套接字
    while(1){
    // get a request
    //int _client = *((int*)client);
    int _chat = _server->getchat();
    if(_chat == 1){
        string request = _server->get_request(clients[0]);
        int s = request.find_first_not_of("");
        int e = request.find_last_not_of(" ");
        string newrequest = request.substr(s, e - s + 1);
        int port = atoi(newrequest.c_str());//要连接的客户端端口号
        map<int, int> allclient2port = _server->getclient2port();
        if(!allclient2port.count(port))
        {
            _server->send_response(clients, "请重新输入正确的端口号\n");
            continue;
        }
        int client = allclient2port[port];
        _server->send_response(clients, "通信已连接，请发送消息\n");
        _server->p2pchat(client, _server,clients[0]);
        _server->setchat(0);
        string inform = "连接已断开\n";
        _server->send_response(clients, inform);
        continue;
    }
    string request = _server->get_request(clients[0]);
    // break if client is done or an error occurred
    if (request.empty())
        break;
    // send response
    if(!request.compare("connect\n"))
        continue;
    bool success = _server->send_response(clients,request);
    // break if an error occurred
    if (not success)
        break;
    }
    int client = clients[0];
    close(client);
    //return NULL;
    return NULL;
}

string Server::get_request(int client) {
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        request.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    if(request.compare("Time\n") == 0){
        time_t t = time(0);
        char ch[64];
        strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S\n", localtime(&t)); //年-月-日 时-分-秒
        string newrequrest = "";
        newrequrest.append(ch);
        return newrequrest;
    }
    
    if(request.compare("name\n") == 0){
        string newrequest = "";
        char hostname[128];
        if(gethostname(hostname, 128))
            perror("gethostname");
        newrequest = string(hostname);
        newrequest.append("\n");
        return newrequest;
    }
    
    if(request.compare("who\n") == 0){
        //查询所有已经连接的客户端，将套接字和端口直接建立映射
        string newrequest = "";
        int num = m_clients.size();
        for(int i = 0; i < num; i++){
            if(i == num - 1){
                struct sockaddr_in addr;
                socklen_t len = sizeof(addr);
                if(getpeername(m_clients[i], (struct sockaddr*) &addr, &len) < 0)
                    perror("getsockname");
                newrequest.append(inet_ntoa((in_addr)addr.sin_addr));
                int port = ntohs(addr.sin_port);
                if(!m_client2ports.count(port)){
                    m_client2ports[port] = m_clients[i];
                    m_port2clients[m_clients[i]] = port;
                }
                string strport = to_string(port);
                newrequest.append(":"+strport+"(self)"+"\n");
                break;
            }
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            if(getpeername(m_clients[i], (struct sockaddr*) &addr, &len) < 0)
                perror("getsockname");
            newrequest.append(inet_ntoa((in_addr)addr.sin_addr));
            int port = ntohs(addr.sin_port);
            if(!m_client2ports.count(port))
                m_client2ports[port] = m_clients[i];
            string strport = to_string(port);
            newrequest.append(":"+strport+"\n");
        }
        /*
        string ip;
        ip.append(inet_ntoa((in_addr)addr.sin_addr));
        int port = ntohs(addr.sin_port);
        string strport = to_string(port);
        newrequest = ip;
        newrequest.append(":"+strport+"\n");
         */
        return newrequest;
    }
    
    if(!request.compare("connect\n")||!request.compare("reconnect\n")){
        string newrequest = "";
        newrequest.append("请输入想要连接的端口号：\n");
        chat = 1;
        return newrequest;
    }
    
    return request;
}

bool Server::send_response(vector<int>&clients, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    for(int i = 0; i < clients.size(); i++){
        int client = clients[i];
        while (nleft) {
            if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
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
    }
    return true;
}

vector<int> Server::getclients(){
    vector<int>curClient;
    curClient.push_back(m_clients[m_clients.size() - 1]);
    return curClient;
}

vector<int> Server::getAllclient(){
    return m_clients;
}

map<int,int> Server::getclient2port(){
    return m_client2ports;
}

void Server::setclients(vector<int>clients){
    m_clients = clients;
}

int Server::getchat(){
    return chat;
}

void Server::setchat(int _chat){
    chat = _chat;
}

//P2P
void Server::p2pchat(int client, Server *server,int fromclient){
    time_t startT = time(nullptr);
    while (1) {
        string request = server->get_request(fromclient);
        if(!request.compare("discom\n"))
            break;
        string selfport = to_string(m_port2clients[getclients()[0]]);
        request.append(" -- 来自端口："+ selfport+"\n");
        vector<int> clients;
        clients.push_back(client);
        if(request.empty())
            break;
        bool success = send_response(clients, request);
        if(not success)
            break;
        time_t endT = time(nullptr);
        if((endT - startT) > 300)
            break;
    }
}

//广播
void Server::broadcast(vector<int> _clients){
    while(1){
        
    }
}
