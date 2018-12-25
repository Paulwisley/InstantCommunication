#include "server.h"

Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    request = "";
    success = true;
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
        handle(clients);
        //一旦接收到了就开辟一个线程将处理程序放入线程内
    }
    close_socket();
}

void Server::handle(vector<int> &clients) {
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
    Server *_server = (Server *)server;
    clients = _server->getclients();
    while(1){
    // get a request
    //int _client = *((int*)client);
    string request = _server->get_request();
    // break if client is done or an error occurred
    if (request.empty())
        break;
    // send response
    bool success = _server->send_response(clients,request);
    // break if an error occurred
    if (not success)
        break;
    }
    int client = clients[clients.size() - 1];
    close(client);
    //return NULL;
    return NULL;
}

string Server::get_request() {
    string request = "";
    // read until we get a newline
    int client = m_clients[m_clients.size() - 1];
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
    
    if(request.compare("connect\n") == 0){
        
    }
    
    return request;
}

bool Server::send_response(vector<int>&clients, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    int client = m_clients[m_clients.size() - 1];
    // loop to be sure it is all sent
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
    return true;
}

vector<int> Server::getclients(){
    return m_clients;
}

void Server::setclients(vector<int>clients){
    m_clients = clients;
}



