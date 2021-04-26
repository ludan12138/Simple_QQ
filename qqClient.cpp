#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>
#include<iostream>

using namespace std;

class qqClient{
public:
    int m_sockfd;

    qqClient();

    bool connectToServer(const char *serverip,const int port);
    
    int Send(const void *buf,const int buflen);

    int Recv(void *buf,const int buflen);

    bool login();

    void print_help();

    ~qqClient();
};

int main(int argc,char *argv[])
{
    qqClient client;
    
    char cmd_buf[100];
    if(client.connectToServer(argv[1],atoi(argv[2]))){
        cout<<"connect success\n";
        while(1){
            cout<<">>";
            string cmd;
            cin>>cmd;
            
                //case "help": client.print_help(); break;
                //case "exit": return 0;
                if(cmd=="login") {
                    memset(cmd_buf,0,sizeof(cmd_buf));
                    strcpy(cmd_buf,cmd.c_str());
                    client.Send(cmd_buf,strlen(cmd_buf));
                    client.login();
                    break;
                }
                
        }
    };  
}

qqClient::qqClient(){
    m_sockfd=0;
}

qqClient::~qqClient() {
    if (m_sockfd!=0) close(m_sockfd);
}

bool qqClient::connectToServer(const char *serverip,const int port){
    m_sockfd=socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(serverip);

    if(connect(m_sockfd,(sockaddr*)&servaddr,sizeof(servaddr))!=0){
        close(m_sockfd); m_sockfd=0; return false;
    }
    return true;
}

bool qqClient::login(){
    string username,password;
    char buf[100];
    /*memset(buf,0,sizeof(buf));
    Recv(buf,sizeof(buf));
    cout<<buf;
    cin>>username;
    memset(buf,0,sizeof(buf));
    strcpy(buf,username.c_str());
    Send(buf,strlen(buf));

    memset(buf,0,sizeof(buf));
    Recv(buf,sizeof(buf));
    cout<<buf;
    cin>>password;
    memset(buf,0,sizeof(buf));
    strcpy(buf,password.c_str());
    Send(buf,strlen(buf));*/

    while(1){
        memset(buf,0,sizeof(buf));
        Recv(buf,sizeof(buf));
        cout<<buf;
        if(!strcmp(buf,"login success")||!strcmp(buf,"Create success!")){
            break;
        }
        string reply;
        //getline(cin,reply);
        cin>>reply;
        memset(buf,0,sizeof(buf));
        strcpy(buf,reply.c_str());
        Send(buf,strlen(buf));
    }
}

void qqClient::print_help(){
    const static char *help_message = ""
    "Usage:"
    "\n\n login"
    "\n     login to server so that other client can see you"
    "\n\n logout"
    "\n     logout from server"
    "\n\n list"
    "\n     list online client"
    "\n\n send host:port data"
    "\n\n help"
    "\n     print this help message"
    "\n\n quit"
    "\n     logout and quit this program";
    printf("%s\n", help_message);
}

int qqClient::Send(const void *buf,const int buflen){
    return send(m_sockfd,buf,buflen,0);
}

int qqClient::Recv(void *buf,const int buflen){
    return recv(m_sockfd,buf,buflen,0);
}