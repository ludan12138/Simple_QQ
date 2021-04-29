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
    int m_sockfd_c;

    qqClient();

    bool connectToServer(const char *serverip,const int port);
    
    int Send(const void *buf,const int buflen);

    int Recv(void *buf,const int buflen);

    bool login();

    void print_help();

    ~qqClient();
};
struct param{
    int fd_c;
    qqClient *client;
};

void *pth_main(void *arg){
    param tmp=*(param*)arg;
    int clientfd_c=tmp.fd_c;
    char buf[1024];
    while(1){
        //cout<<"i am recving\n";
        memset(buf,0,sizeof(buf));
        recv(clientfd_c,buf,sizeof(buf),0);
        cout<<buf<<endl;
    }

}
int main(int argc,char *argv[])
{
    qqClient client;
    
    char cmd_buf[100];
    if(client.connectToServer(argv[1],atoi(argv[2]))){
        //cout<<"connect success\n";
        pthread_t pthid;
        param param1;
        param1.fd_c=client.m_sockfd_c;
        param1.client=&client;
        if(pthread_create(&pthid,NULL,pth_main,&param1)!=0){
            cout<<"pthreat_create failed\n";
            return -1;
        }
        while(1){
            cout<<"(offline)>>";
            string cmd;
            getline(cin,cmd);
            if(cmd==""){
                continue;
            }
            //case "help": client.print_help(); break;
            //case "exit": return 0;
            if(cmd=="login") {
                memset(cmd_buf,0,sizeof(cmd_buf));
                strcpy(cmd_buf,cmd.c_str());
                client.Send(cmd_buf,strlen(cmd_buf));
                client.login();
                continue;
            }
            if(cmd=="help"){
                client.print_help();
                continue;
            }
            if(cmd=="exit"){
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

    sleep(1);

    m_sockfd_c=socket(AF_INET,SOCK_STREAM,0);

    sockaddr_in servaddr_c;
    memset(&servaddr_c,0,sizeof(servaddr_c));
    servaddr_c.sin_family = AF_INET;
    servaddr_c.sin_port = htons(port);
    servaddr_c.sin_addr.s_addr = inet_addr(serverip);

    if(connect(m_sockfd_c,(sockaddr*)&servaddr_c,sizeof(servaddr_c))!=0){
        close(m_sockfd_c); m_sockfd_c=0; return false;
    }

    return true;
}

bool qqClient::login(){
    string username,password;
    char buf[100];
    while(1){
        memset(buf,0,sizeof(buf));
        Recv(buf,sizeof(buf));
        cout<<buf;
        if(!strcmp(buf,"login success")){
            break;
        }
        if(!strcmp(buf,"Create success!")){
            continue;
        }
        string reply;
        //getline(cin,reply);
        cin>>reply;
        memset(buf,0,sizeof(buf));
        strcpy(buf,reply.c_str());
        Send(buf,strlen(buf));
    }
    cout<<endl;
    cin.get();
    //after login success
    while(1){
        cout<<"(online)>>";
        string cmd;
        getline(cin,cmd);
        memset(buf,0,sizeof(buf));
        strcpy(buf,cmd.c_str());
        Send(buf,strlen(buf));
        if(cmd=="logoff"){
            break;
        }
        if(cmd=="list"){
            while(1){
                memset(buf,0,sizeof(buf));
                Recv(buf,sizeof(buf));
                if(!strcmp(buf,"finish")) break;                
                cout<<buf<<endl;
            }
            continue;
        }
        if(cmd=="chat"){
            cout<<"Who do you to chat with?(type user's name):";
            string sendTo;
            cin>>sendTo;

            memset(buf,0,sizeof(buf));
            strcpy(buf,sendTo.c_str());
            Send(buf,strlen(buf));

            while(1){
                memset(buf,0,sizeof(buf));
                Recv(buf,sizeof(buf));
                cout<<buf;
                if(!strcmp(buf,"Now you can send massage!(type \"quit\" to leave.)\n")){
                    break;
                }
                cin>>sendTo;
                memset(buf,0,sizeof(buf));
                strcpy(buf,sendTo.c_str());
                Send(buf,strlen(buf));
            }
            cout<<"you can type message\n";
            cin.get();
            while(1){
                string message;
                /*memset(buf,0,sizeof(buf));
                Recv(buf,sizeof(buf));
                cout<<buf;*/

                //cin>>message;
                getline(cin,message);
                memset(buf,0,sizeof(buf));
                //cin.getline(buf,sizeof(buf));
                //getline(cin,message);
                //message=buf;
                if(message=="quit"){
                    strcpy(buf,message.c_str());
                    Send(buf,strlen(buf));
                    break;
                }
                strcpy(buf,message.c_str());
                Send(buf,strlen(buf));
            }
            continue;
        }
        if(cmd=="send"){

        }
    }

}

void qqClient::print_help(){
    const static char *help_message = ""
    "Usage:"
    "\n\n help"
    "\n     print this help message"
    "\n\n exit"
    "\n     exit the program"
    "\n\n login"
    "\n     login to server so that other client can see you"
    "\n\n when you login success,use followed commands to interact"
    "\n\n logout"
    "\n     logout from server"
    "\n\n list"
    "\n     list online client"
    "\n\n send"
    "\n     choose a online user to send file to him"
    "\n\n chat"
    "\n     choose a online user to chat with him";
    printf("%s\n", help_message);
}

int qqClient::Send(const void *buf,const int buflen){
    return send(m_sockfd,buf,buflen,0);
}

int qqClient::Recv(void *buf,const int buflen){
    return recv(m_sockfd,buf,buflen,0);
}